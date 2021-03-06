/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#ifndef WEOS_CMSIS_RTOS_FUTURE_HPP
#define WEOS_CMSIS_RTOS_FUTURE_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "_core.hpp"

#include "_thread_detail.hpp"
#include "_thread.hpp"
#include "../atomic.hpp"
#include "../chrono.hpp"
#include "../exception.hpp"
#include "../memory.hpp"
#include "../semaphore.hpp"
#include "../system_error.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"

#include <stdexcept>
#include <new>


namespace std
{

template <typename TResult>
class future;

// ----=====================================================================----
//     Types
// ----=====================================================================----

enum class launch
{
    async = 1,
    deferred = 2,
    any = async | deferred
};

enum class future_status
{
    ready,
    timeout,
    deferred
};

enum class future_errc
{
    broken_promise = 1,
    future_already_retrieved,
    promise_already_satisfied,
    no_state
};

template <>
struct is_error_code_enum<future_errc> : public true_type {};

const error_category& future_category() noexcept;

inline
error_code make_error_code(future_errc errno) noexcept
{
    return error_code(static_cast<int>(errno), future_category());
}

class future_error : public std::logic_error
{
public:
    future_error(error_code ec)
        : logic_error("future error"),
          m_errorCode(ec)
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

class OneshotConditionVariable : private WEOS_NAMESPACE::semaphore
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
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d)
    {
        if (semaphore::try_wait_for(d))
        {
            semaphore::post();
            return future_status::ready;
        }
        else
        {
            return future_status::timeout;
        }
    }

    template <typename TClock, typename TDuration>
    future_status wait_until(const chrono::time_point<TClock, TDuration>& tp)
    {
        if (semaphore::try_wait_until(tp))
        {
            semaphore::post();
            return future_status::ready;
        }
        else
        {
            return future_status::timeout;
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

    explicit
    SharedStateBase(void* ownedStack) noexcept
        : m_referenceCount(1),
          m_flags(0),
          m_ownedStack(ownedStack)
    {
    }

    virtual
    ~SharedStateBase()
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

    template <typename TRep, typename TPeriod>
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d)
    {
        if (!(m_flags & Ready))
            return m_cv.wait_for(d);

        return future_status::ready;
    }

    void startSettingValue();
    void setException(exception_ptr exc);
    void setValue();

    void copyValue();

protected:
    atomic_int m_referenceCount;
    atomic_uint m_flags;
    exception_ptr m_exception;
    OneshotConditionVariable m_cv;
    void* m_allocationBase;
    void* m_ownedStack;

    virtual
    void destroy() noexcept;
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
    explicit
    SharedState(void* ownedStack) noexcept
        : SharedStateBase(ownedStack)
    {
    }

    template <typename T>
    void setValue(T&& value)
    {
        this->startSettingValue();
        ::new (&m_value) TResult(std::forward<T>(value));
        this->m_flags |= SharedStateBase::ValueConstructed | SharedStateBase::Ready;
        this->m_cv.notify();
    }

    TResult moveValue()
    {
        this->wait();
        if (this->m_exception != nullptr)
            rethrow_exception(this->m_exception);
        return std::move(*reinterpret_cast<TResult*>(&m_value));
    }

protected:
    storage_t m_value;

    virtual
    void destroy() noexcept override
    {
        if (this->m_flags & SharedStateBase::ValueConstructed)
            reinterpret_cast<TResult*>(&m_value)->~TResult();
        SharedStateBase::destroy();
    }
};

// ----=====================================================================----
//     AsyncSharedState
// ----=====================================================================----

template <typename TResult, typename TCallable>
class AsyncSharedState : public SharedState<TResult>
{
public:
    explicit
    AsyncSharedState(void* ownedStack, TCallable&& callable)
        : SharedState<TResult>(ownedStack),
          m_callable(std::forward<TCallable>(callable))
    {
    }

    virtual
    void invoke() noexcept
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
    virtual
    void destroy() noexcept override
    {
        this->wait();
        SharedState<TResult>::destroy();
    }

private:
    TCallable m_callable;
};

template <typename TCallable>
class AsyncSharedState<void, TCallable> : public SharedStateBase
{
public:
    explicit
    AsyncSharedState(void* ownedStack, TCallable&& callable)
        : SharedStateBase(ownedStack),
          m_callable(std::forward<TCallable>(callable))
    {
    }

    virtual
    void invoke() noexcept
    {
        try
        {
            m_callable();
            this->setValue();
        }
        catch (...)
        {
            this->setException(current_exception());
        }
    }

protected:
    virtual
    void destroy() noexcept override
    {
        this->wait();
        SharedStateBase::destroy();
    }

private:
    TCallable m_callable;
};

template <typename TResult, typename TCallable>
future<TResult> makeAsyncSharedState(
        WEOS_NAMESPACE::weos_detail::ThreadProperties& props, TCallable&& f)
{
    using shared_state_type = AsyncSharedState<TResult, TCallable>;
    static constexpr size_t alignment = alignment_of<shared_state_type>::value;
    static constexpr size_t size = sizeof(shared_state_type);

    auto deleter = props.allocate();

    if (!props.align(alignment, size))
    {
        WEOS_THROW_SYSTEM_ERROR(
                    std::errc::not_enough_memory,
                    "makeAsyncSharedState: stack size too small");
    }

    // Put the shared state on the stack.
    unique_ptr<shared_state_type, SharedStateBaseDeleter> state(
                ::new (props.m_stackBegin) shared_state_type(
                    deleter.owned_stack(),
                    std::forward<TCallable>(f)));
    deleter.release(); // managed by 'state' from now on

    props.offset_by(size);

    WEOS_NAMESPACE::thread(props, &shared_state_type::invoke, state.get()).detach();

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

    template <typename TRep, typename TPeriod>
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d) const
    {
        return m_state->wait_for(d);
    }

    // TODO:
    template <typename TClock, typename TDuration>
    future_status wait_until(const chrono::time_point<TClock, TDuration>& tp) const;

private:
    // The shared state.
    shared_state_type* m_state;

    // This constructor is used in a promise to create a future with the
    // same shared state.
    explicit
    future(shared_state_type* state)
        : m_state(state)
    {
        m_state->attachFuture();
        m_state->incReferenceCount();
    }


    template <typename T>
    friend class promise;

    template <typename T, typename U>
    friend future<T> weos_detail::makeAsyncSharedState(
            WEOS_NAMESPACE::weos_detail::ThreadProperties&, U&&);
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

    template <typename TRep, typename TPeriod>
    future_status wait_for(const chrono::duration<TRep, TPeriod>& d) const
    {
        return m_state->wait_for(d);
    }

    // TODO:
    template <typename TClock, typename TDuration>
    future_status wait_until(const chrono::time_point<TClock, TDuration>& tp) const;

private:
    // The shared state.
    weos_detail::SharedStateBase* m_state;

    // This constructor is used in a promise to create a future with the
    // same shared state.
    explicit
    future(weos_detail::SharedStateBase* state);


    template <typename T>
    friend class promise;

    template <typename T, typename U>
    friend future<T> weos_detail::makeAsyncSharedState(
            WEOS_NAMESPACE::weos_detail::ThreadProperties&, U&&);
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
class promise
{
public:
    //! Default-constructs a promise.
    //!
    //! The shared state will be empty.
    promise()
    {
        using namespace weos_detail;

        struct Deleter
        {
            void operator()(void* p) noexcept
            {
                ::operator delete(p);
            }
        };

        unique_ptr<void, Deleter> p(::operator new(sizeof(SharedState<T>)));
        m_state = ::new (p.get()) SharedState<T>(p.get());
        p.release();
    }

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
    ~promise()
    {
        if (m_state)
        {
            // If no value or exception has been set in the shared state and
            // at least one future is still attached to it, we have to
            // signal a broken promise.
            if (!m_state->isReady() && m_state->referenceCount() > 1)
            {
                m_state->setException(make_exception_ptr(
                                           future_error(make_error_code(future_errc::broken_promise))));
            }

            m_state->decReferenceCount();
        }
    }

    //! Move-assigns the \p other promise to this one.
    promise& operator=(promise&& other) noexcept
    {
        promise(std::move(other)).swap(*this);
        return *this;
    }

    promise(const promise& other) = delete;
    promise& operator=(const promise& other) = delete;

    //! Returns a future associated with this promise.
    future<T> get_future()
    {
        if (m_state == nullptr)
            throw future_error(make_error_code(future_errc::no_state));
        return future<T>(m_state);
    }

    void set_value(const T& value)
    {
        if (m_state == nullptr)
            throw future_error(make_error_code(future_errc::no_state));
        m_state->setValue(value);
    }

    void set_value(T&& value)
    {
        if (m_state == nullptr)
            throw future_error(make_error_code(future_errc::no_state));
        m_state->setValue(std::move(value));
    }

    void set_exception(exception_ptr exc)
    {
        if (m_state == nullptr)
            throw future_error(make_error_code(future_errc::no_state));
        m_state->setException(exc);
    }

    // TODO:
    // void set_value_at_thread_exit();
    // void set_exception_at_thread_exit(exception_ptr exc);

    //! Swaps this promise with the \p other promise.
    void swap(promise& other) noexcept
    {
        std::swap(m_state, other.m_state);
    }

private:
    weos_detail::SharedState<T>* m_state;
};

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
    promise(promise&& other) noexcept;

    //! Destroys the promise.
    ~promise();

    //! Move-assigns the \p other promise to this one.
    promise& operator=(promise&& other) noexcept;

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
    void swap(promise& other) noexcept;

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

} // namespace std


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     future utilities
// ----=====================================================================----

template <typename T>
std::future<T> make_exceptional_future(std::exception_ptr exc)
{
    std::promise<T> promise;
    promise.set_exception(exc);
    return promise.get_future();
}

template <typename T, typename TException>
std::future<T> make_exceptional_future(TException&& exc)
{
    std::promise<T> promise;
    promise.set_exception(std::make_exception_ptr(std::forward<TException>(exc)));
    return promise.get_future();
}

// ----=====================================================================----
//     async()
// ----=====================================================================----

template <typename TFunction, typename... TArgs>
inline
std::future<typename weos_detail::invoke_result_type<
                typename std::decay<TFunction>::type,
                typename std::decay<TArgs>::type...>::type>
async(std::launch launchPolicy, const thread_attributes& attrs,
      TFunction&& f, TArgs&&... args)
{
    using result_type = typename weos_detail::invoke_result_type<
                            typename std::decay<TFunction>::type,
                            typename std::decay<TArgs>::type...>::type;

    using function_type = weos_detail::DecayedFunction<
                              typename std::decay<TFunction>::type,
                              typename std::decay<TArgs>::type...>;

    // TODO: make use of launchPolicy

    weos_detail::ThreadProperties props(attrs);
    return std::weos_detail::makeAsyncSharedState<result_type, function_type>(
                props,
                function_type(weos_detail::decay_copy(std::forward<TFunction>(f)),
                              weos_detail::decay_copy(std::forward<TArgs>(args))...));
}

template <typename TFunction, typename... TArgs>
WEOS_FORCE_INLINE
std::future<typename weos_detail::invoke_result_type<
                typename std::decay<TFunction>::type,
                typename std::decay<TArgs>::type...>::type>
async(const thread_attributes& attrs, TFunction&& f, TArgs&&... args)
{
    return async(std::launch::any, attrs,
                 std::forward<TFunction>(f),
                 std::forward<TArgs>(args)...);
}

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_FUTURE_HPP
