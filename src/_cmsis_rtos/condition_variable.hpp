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

#ifndef WEOS_CMSIS_RTOS_CONDITIONVARIABLE_HPP
#define WEOS_CMSIS_RTOS_CONDITIONVARIABLE_HPP

#include "core.hpp"

#include "_tq.hpp"
#include "../chrono.hpp"
#include "../mutex.hpp"
#include "../semaphore.hpp"


WEOS_BEGIN_NAMESPACE

WEOS_SCOPED_ENUM_BEGIN(cv_status)
{
    no_timeout,
    timeout
};
WEOS_SCOPED_ENUM_END(cv_status)

//! A condition variable.
class condition_variable
{
    //! A helper class for temporarily releasing a lock.
    //! The lock_releaser is a helper class to release a lock until the object
    //! goes out of scope. The constructor calls unlock() and the destructor
    //! calls lock(). It is somehow the dual to the lock_guard<> which calls
    //! lock() in the constructor and unlock() in the destructor.
    class lock_releaser
    {
    public:
        explicit lock_releaser(unique_lock<mutex>& lock) noexcept
            : m_lock(lock)
        {
            m_lock.unlock();
        }

        ~lock_releaser() noexcept(false)
        {
            m_lock.lock();
        }

    private:
        unique_lock<mutex>& m_lock;
    };

public:
    typedef condition_variable* native_handle_type;

    //! Creates a condition variable.
    condition_variable();

    //! Destroys the condition variable.
    //!
    //! \note The condition variable must not be destroyed if a thread is
    //! waiting on it.
    ~condition_variable();

    //! Notifies a thread waiting on this condition variable.
    //!
    //! Notifies one thread which is waiting on this condition variable.
    //!
    //! \note This method may be called in an interrupt context.
    void notify_one() noexcept;

    //! Notifies all threads waiting on this condition variable.
    //!
    //! Notifies all threads which are waiting on this condition variable.
    //!
    //! \note This method may be called in an interrupt context.
    void notify_all() noexcept;

    //! Waits on this condition variable.
    //!
    //! The given \p lock is released and the current thread is added to a
    //! list of threads waiting for a notification. The calling thread is
    //! blocked until a notification is sent via notify() or notify_all()
    //! or a spurious wakeup occurs. The \p lock is re-acquired when the
    //! function exits (either due to a notification or due to an exception).
    void wait(unique_lock<mutex>& lock);

    //! Waits on this condition variable.
    //!
    //! This is a convenience function which is equivalent to
    //! \code
    //! while (!pred())
    //! {
    //!     wait(lock);
    //! }
    //! \endcode
    template <typename TPredicate>
    inline
    void wait(unique_lock<mutex>& lock, TPredicate pred)
    {
        while (!pred())
        {
            wait(lock);
        }
    }

    //! Waits on this condition variable with a timeout.
    //!
    //! Releases the given \p lock and adds the calling thread to a list
    //! of threads waiting for a notification. The thread is blocked until
    //! a notification is sent, a spurious wakeup occurs or the timeout
    //! period \p d expires. When the function returns, the \p lock is
    //! re-acquired no matter what has caused the wakeup.
    template <typename TRep, typename TPeriod>
    inline
    cv_status wait_for(unique_lock<mutex>& lock,
                       const chrono::duration<TRep, TPeriod>& d)
    {
        // First enqueue ourselves in the list of waiters.
        weos_detail::_tq::_t t(m_tq);
        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        lock_releaser releaser(lock);
        // Wait until we receive a signal, then re-lock the lock.
        if (t.wait_for(d) || t.unlink())
            return cv_status::no_timeout;
        else
            return cv_status::timeout;
    }

    template <typename TRep, typename TPeriod, typename TPredicate>
    inline
    bool wait_for(unique_lock<mutex>& lock,
                  const chrono::duration<TRep, TPeriod>& d,
                  TPredicate pred)
    {
        while (!pred())
        {
            // TODO: This does not work in case of spurious wakeups. We
            // wait too long then.
            if (wait_for(lock, d) == cv_status::timeout)
              return pred();
        }
        return true;
    }

    //! Waits on this condition variable with a timeout.
    //!
    //! Releases the given \p lock and adds the calling thread to a list
    //! of threads waiting for a notification. The thread is blocked until
    //! a notification is sent, a spurious wakeup occurs or the timeout
    //! point \p time is reached. When the function returns, the \p lock is
    //! re-acquired no matter what has caused the wakeup.
    template <typename TClock, typename TDuration>
    cv_status wait_until(unique_lock<mutex>& lock,
                         const chrono::time_point<TClock, TDuration>& time)
    {
        // First enqueue ourselves in the list of waiters.
        weos_detail::_tq::_t t(m_tq);
        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        lock_releaser releaser(lock);
        // Wait until we receive a signal, then re-lock the lock.
        if (t.wait_until(time) || t.unlink())
            return cv_status::no_timeout;
        else
            return cv_status::timeout;
    }

    template <typename TClock, typename TDuration, typename TPredicate>
    inline
    bool wait_until(unique_lock<mutex>& lock,
                    const chrono::time_point<TClock, TDuration>& time,
                    TPredicate pred)
    {
        while (!pred())
        {
            if (wait_until(lock, time) == cv_status::timeout)
                return pred();
        }
        return true;
    }

    //! Returns the native handle.
    native_handle_type native_handle()
    {
        return this;
    }

    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;

private:
    weos_detail::_tq m_tq;
};

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_CONDITIONVARIABLE_HPP
