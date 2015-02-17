/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2015, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef WEOS_COMMON_FUTURE_HPP
#define WEOS_COMMON_FUTURE_HPP

#include "../atomic.hpp"
#include "../chrono.hpp"
#include "../exception.hpp"
#include "../memory.hpp"
#include "../semaphore.hpp"
#include "../system_error.hpp"
#include "../thread.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

template <typename TResult>
class future;

// ----=====================================================================----
//
// ----=====================================================================----

WEOS_SCOPED_ENUM_BEGIN(future_status)
{
    ready,
    timeout,
    deferred
};
WEOS_SCOPED_ENUM_END(future_status)

WEOS_SCOPED_ENUM_BEGIN(future_errc)
{
    broken_promise = 1,
    future_already_retrieved,
    promise_already_satisfied,
    no_state
};
WEOS_SCOPED_ENUM_END(future_errc)

template <>
struct is_error_code_enum<future_errc> : public true_type {};

#ifdef WEOS_NO_SCOPED_ENUM
template <>
struct is_error_code_enum<future_errc::type> : public true_type {};
#endif

const error_category& future_category() noexcept;

inline
error_code make_error_code(future_errc errno) noexcept
{
    return error_code(static_cast<int>(errno), future_category());
}

class future_error // TODO: Base class
{
public:
    future_error(error_code ec)
        : m_errorCode(ec)
    {
    }

    const error_code& code() const noexcept
    {
        return m_errorCode;
    }

private:
    error_code m_errorCode;
};

// ----=====================================================================----
//     Shared state
// ----=====================================================================----

namespace weos_detail
{

class OneshotConditionVariable : private semaphore
{
public:
    OneshotConditionVariable()
    {
    }

    void notify()
    {
        semaphore::post();
    }

    void wait()
    {
        semaphore::wait();
        semaphore::post();
    }

    template <typename TRep, typename TPeriod>
    bool wait_for(const chrono::duration<TRep, TPeriod>& d)
    {
        if (semaphore::try_wait_for(d))
        {
            semaphore::post();
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename TClock, typename TDuration>
    bool wait_until(const chrono::time_point<TClock, TDuration>& tp)
    {
        if (semaphore::try_wait_until(tp))
        {
            semaphore::post();
            return true;
        }
        else
        {
            return false;
        }
    }
};

// ----=====================================================================----
//     SharedStateBase
// ----=====================================================================----

class SharedStateBase
{
public:
    enum
    {
        FutureAttached   = 0x01,
        BeingSatisfied   = 0x02,
        ValueConstructed = 0x04,
        Ready            = 0x08,
    };

    SharedStateBase() noexcept
        : m_referenceCount(1),
          m_flags(0)
    {
    }

    virtual ~SharedStateBase()
    {
    }

    int referenceCount() const noexcept
    {
        return m_referenceCount;
    }

    void incReferenceCount() noexcept
    {
        ++m_referenceCount;
    }

    void decReferenceCount() noexcept
    {
        if (--m_referenceCount == 0)
        {
            destroy();
        }
    }

    void attachFuture();

    bool isReady() const noexcept
    {
        return (m_flags & Ready) != 0;
    }

    void wait();

    void startSettingValue();
    void setException(exception_ptr exc);
    void setValue();

    void copyValue();

protected:
    atomic_int m_referenceCount;
    atomic_uint m_flags;
    exception_ptr m_exception;
    OneshotConditionVariable m_cv;

    virtual void destroy() noexcept
    {
        delete this;
    }
};

struct SharedStateBaseDeleter
{
    void operator()(SharedStateBase* state) noexcept
    {
        state->decReferenceCount();
    }
};

// ----=====================================================================----
//     SharedState
// ----=====================================================================----

template <typename TResult>
class SharedState : public SharedStateBase
{
    typedef typename aligned_storage<sizeof(TResult),
                                     alignment_of<TResult>::value>::type storage_t;

public:
    template <typename T>
    void setValue(T&& value)
    {
        this->startSettingValue();
        ::new (&m_value) TResult(WEOS_NAMESPACE::forward<T>(value));
        this->m_flags |= SharedStateBase::ValueConstructed | SharedStateBase::Ready;
        this->m_cv.notify();
    }

    TResult moveValue()
    {
        this->wait();
        if (this->m_exception != nullptr)
            rethrow_exception(this->m_exception);
        return WEOS_NAMESPACE::move(*reinterpret_cast<TResult*>(&m_value));
    }

protected:
    storage_t m_value;

    virtual void destroy() noexcept override
    {
        if (this->m_flags & SharedStateBase::ValueConstructed)
            reinterpret_cast<TResult*>(&m_value)->~TResult();
        delete this;
    }
};

// ----=====================================================================----
//     AsyncSharedState
// ----=====================================================================----

// TODO: Move DecayedFunction and decay_copy into thread.hpp because we would
// need them there too.
template <typename TFunction, typename... TArgs>
class DecayedFunction
{
public:
    typedef typename ::WEOS_NAMESPACE::weos_detail::invoke_result_type<TFunction, TArgs...>::type result_type;

    explicit DecayedFunction(TFunction&& f, TArgs&&... args)
        : m_boundFunction(WEOS_NAMESPACE::move(f),
                          WEOS_NAMESPACE::move(args)...)
    {
    }

    DecayedFunction(DecayedFunction&& other)
        : m_boundFunction(WEOS_NAMESPACE::move(other.m_boundFunction))
    {
    }

    result_type operator()()
    {
        typedef typename weos_detail::make_tuple_indices<
                1 + sizeof...(TArgs), 1>::type indices_type;
        return invoke(indices_type());
    }

private:
    template <std::size_t... TIndices>
    result_type invoke(weos_detail::TupleIndices<TIndices...>)
    {
        return WEOS_NAMESPACE::invoke(
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<0>(m_boundFunction)),
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<TIndices>(m_boundFunction))...);
    }

    tuple<TFunction, TArgs...> m_boundFunction;
};

// 30.2.6
template <typename T>
typename decay<T>::type decay_copy(T&& v)
{
    return WEOS_NAMESPACE::forward<T>(v);
}

template <typename TResult, typename TCallable>
class AsyncSharedState : public SharedState<TResult>
{
public:
    explicit AsyncSharedState(TCallable&& callable)
        : m_callable(WEOS_NAMESPACE::forward<TCallable>(callable))
    {
    }

    virtual void invoke()
    {
        try
        {
            this->setValue(m_callable());
        }
        catch (...)
        {
            this->setException(current_exception());
        }
    }

protected:
    virtual void destroy() noexcept override
    {
        this->wait();
        SharedState<TResult>::destroy();
    }

private:
    TCallable m_callable;
};

template <typename TResult, typename TCallable>
future<TResult> makeAsyncSharedState(const thread::attributes& attrs,
                                     TCallable&& f)
{
    using shared_state_type = AsyncSharedState<TResult, TCallable>;

    unique_ptr<shared_state_type, SharedStateBaseDeleter>
            state(new shared_state_type(WEOS_NAMESPACE::forward<TCallable>(f)));
    thread(attrs, &shared_state_type::invoke, state.get()).detach();

    return future<TResult>(state.get());
}

} // weos_detail

// ----=====================================================================----
//     future<T>
// ----=====================================================================----

template <typename TResult>
class future
{
    typedef weos_detail::SharedState<TResult> shared_state_type;

public:
    //! Constructs a future without shared state.
    future() noexcept
        : m_state(nullptr)
    {
    }

    //! Move-constructs a future.
    //!
    //! Constructs a future by moving from the \p other future.
    future(future&& other) noexcept
        : m_state(other.m_state)
    {
        other.m_state = nullptr;
    }

    //! Destroys the future.
    ~future()
    {
        if (m_state)
            m_state->decReferenceCount();
    }

    //! Move-assigns the \p other future to this one.
    future& operator=(future&& other) noexcept
    {
        if (m_state)
            m_state->decReferenceCount();
        m_state = other.m_state;
        other.m_state = nullptr;

        return *this;
    }

    future(const future&) = delete;
    future& operator=(const future&) = delete;

    //shared_future<TResult> share();

    //! Returns the result.
    //!
    //! Blocks the caller until the result is available and returns it.
    //! If an exception has been stored in the shared state, it will be
    //! thrown.
    TResult get()
    {
        unique_ptr<weos_detail::SharedStateBase,
                   weos_detail::SharedStateBaseDeleter> state(m_state);
        m_state = nullptr;
        return static_cast<shared_state_type*>(state.get())->moveValue();
    }

    //! Swaps two futures.
    //!
    //! Swaps this future with the \p other future.
    void swap(future& other) noexcept
    {
        std::swap(m_state, other.m_state);
    }

    //! Checks if the future has an attached shared state.
    //!
    //! Returns \p true, if the future has an attached shared state.
    bool valid() const noexcept
    {
        return m_state != nullptr;
    }

    //! Waits until the result is available.
    //!
    //! Blocks the caller until the result is available.
    void wait() const
    {
        m_state->wait();
    }

    // TODO:
    template <typename TRep, typename TPeriod>
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d) const;

    // TODO:
    template <typename TClock, typename TDuration>
    future_status wait_until(const chrono::time_point<TClock, TDuration>& tp) const;

private:
    // The shared state.
    shared_state_type* m_state;

    // This constructor is used in a promise to create a future with the
    // same shared state.
    explicit future(shared_state_type* state)
        : m_state(state)
    {
        m_state->attachFuture();
        m_state->incReferenceCount();
    }


    template <typename T>
    friend class promise;

    template <typename T, typename U>
    friend future<T> weos_detail::makeAsyncSharedState(
            const thread::attributes&, U&&);
};

// ----=====================================================================----
//     future<void>
// ----=====================================================================----

template <>
class future<void>
{
public:
    //! Default constructs a future.
    //!
    //! Default constructs a future without an associated shared state.
    future() noexcept
        : m_state(nullptr)
    {
    }

    //! Move-constructs a future.
    //!
    //! Constructs a future by moving from the \p other future.
    future(future&& other) noexcept
        : m_state(other.m_state)
    {
        other.m_state = nullptr;
    }

    //! Destroys the future.
    //!
    //! Destroys the future and releases its reference to the shared state.
    //! This function will block, if the future is a result to an
    //! async() call and the result is not available, yet.
    ~future();

    //! Move-assigns the \p other future to this one.
    future& operator=(future&& other) noexcept
    {
        if (m_state)
            m_state->decReferenceCount();
        m_state = other.m_state;
        other.m_state = nullptr;

        return *this;
    }

    future(const future&) = delete;
    future& operator=(const future&) = delete;

    //! Returns the result.
    //!
    //! Blocks the caller until the result is available and returns it.
    //! If an exception has been stored in the shared state, it will be
    //! thrown.
    void get();

    // TODO:
    //shared_future<void> share();

    //! Swaps two futures.
    //!
    //! Swaps this future with the \p other future.
    void swap(future& other) noexcept
    {
        std::swap(m_state, other.m_state);
    }

    //! Checks if the future has an attached shared state.
    //!
    //! Returns \p true, if the future has an attached shared state.
    bool valid() const noexcept
    {
        return m_state != nullptr;
    }

    //! Waits until the result is available.
    //!
    //! Blocks the caller until the result is available.
    void wait() const
    {
        m_state->wait();
    }

    // TODO:
    template <typename TRep, typename TPeriod>
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d) const;

    // TODO:
    template <typename TClock, typename TDuration>
    future_status wait_until(const chrono::time_point<TClock, TDuration>& tp) const;

private:
    // The shared state.
    weos_detail::SharedStateBase* m_state;

    // This constructor is used in a promise to create a future with the
    // same shared state.
    explicit future(weos_detail::SharedStateBase* state);

    template <typename T>
    friend class promise;
};

//! Swaps two futures \p a and \p b.
template <typename T>
inline
void swap(future<T>& a, future<T>& b) noexcept
{
    a.swap(b);
}

// ----=====================================================================----
//     promise<T>
// ----=====================================================================----

template <typename T>
class promise;

// ----=====================================================================----
//     promise<void>
// ----=====================================================================----

template <>
class promise<void>
{
public:
    //! Default-constructs a promise.
    //!
    //! The shared state will be empty.
    promise();

    // TODO:
    // template <typename TAllocator>
    // promise(allocator_arg_t, const TAllocator& allocator);

    //! Constructs a promise by moving from the \p other promise.
    promise(promise&& other) noexcept
        : m_state(other.m_state)
    {
        other.m_state = nullptr;
    }

    //! Destroys the promise.
    ~promise();

    //! Move-assigns the \p other promise to this one.
    promise& operator=(promise&& other) noexcept
    {
        promise(WEOS_NAMESPACE::move(other)).swap(*this);
        return *this;
    }

    promise(const promise& other) = delete;
    promise& operator=(const promise& other) = delete;

    //! Returns a future associated with this promise.
    future<void> get_future();

    void set_value();
    void set_exception(exception_ptr exc);

    // TODO:
    // void set_value_at_thread_exit();
    // void set_exception_at_thread_exit(exception_ptr exc);

    //! Swaps this promise with the \p other promise.
    void swap(promise& other) noexcept
    {
        std::swap(m_state, other.m_state);
    }

private:
    weos_detail::SharedStateBase* m_state;
};

//! Swaps two promises \p a and \p b.
template <typename T>
inline
void swap(promise<T>& a, promise<T>& b) noexcept
{
    a.swap(b);
}

// ----=====================================================================----
//     async()
// ----=====================================================================----

template <typename TFunction, typename... TArgs>
inline
future<typename weos_detail::invoke_result_type<
           typename decay<TFunction>::type,
           typename decay<TArgs>::type...>::type>
async(const thread::attributes& attrs, TFunction&& f, TArgs&&... args)
{
    using namespace weos_detail;

    using result_type = typename invoke_result_type<
                            typename decay<TFunction>::type,
                            typename decay<TArgs>::type...>::type;

    using function_type = DecayedFunction<typename decay<TFunction>::type,
                                          typename decay<TArgs>::type...>;

    return makeAsyncSharedState<result_type, function_type>(
                attrs,
                function_type(decay_copy(WEOS_NAMESPACE::forward<TFunction>(f)),
                              decay_copy(WEOS_NAMESPACE::forward<TArgs>(args))...));
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_FUTURE_HPP
