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
#include "../semaphore.hpp"
#include "../system_error.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

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

namespace weos_future_detail
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

class SharedState
{
public:
    enum
    {
        BeingSatisfied = 0x01,
        Ready          = 0x02,
        FutureAttached = 0x04,
    };

    SharedState()
        : m_referenceCount(1),
          m_flags(0)
    {
    }

    virtual ~SharedState()
    {
    }

    int referenceCount() const
    {
        return m_referenceCount;
    }

    void incReferenceCount()
    {
        ++m_referenceCount;
    }

    void decReferenceCount()
    {
        if (--m_referenceCount == 0)
        {
            destroy();
        }
    }

    void attachFuture();

    bool isReady() const
    {
        return (m_flags & Ready) != 0;
    }

    void wait();

    void setException(exception_ptr exc);
    void setValue();
    void value();

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

} // weos_future_detail

// ----=====================================================================----
//     future<T>
// ----=====================================================================----

template <typename T>
class future;

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

    void get();

    // TODO:
    //shared_future<void> share();

    void swap(future& other) noexcept
    {
        std::swap(m_state, other.m_state);
    }

    bool valid() const noexcept
    {
        return m_state != nullptr;
    }

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
    weos_future_detail::SharedState* m_state;

    // This constructor is used in a promise to create a future with the
    // same shared state.
    explicit future(weos_future_detail::SharedState* state);

    template <typename TPromiseValue>
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
    weos_future_detail::SharedState* m_state;
};

//! Swaps two promises \p a and \p b.
template <typename T>
inline
void swap(promise<T>& a, promise<T>& b) noexcept
{
    a.swap(b);
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_FUTURE_HPP
