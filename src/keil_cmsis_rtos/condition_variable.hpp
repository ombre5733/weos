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

#ifndef WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP
#define WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP

#include "core.hpp"

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
    void notify_one() noexcept;

    //! Notifies all threads waiting on this condition variable.
    //!
    //! Notifies all threads which are waiting on this condition variable.
    void notify_all() noexcept;

    //! Waits on this condition variable.
    //!
    //! The given \p lock is released and the current thread is added to a
    //! list of threads waiting for a notification. The calling thread is
    //! blocked until a notification is sent via notify() or notify_all()
    //! or a spurious wakeup occurs. The \p lock is reacquired when the
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
    //! reacquired no matter what has caused the wakeup.
    template <typename TRep, typename TPeriod>
    inline
    cv_status wait_for(unique_lock<mutex>& lock,
                       const chrono::duration<TRep, TPeriod>& d)
    {
        using namespace chrono;

        milliseconds converted = duration_cast<milliseconds>(d);
        if (converted < d)
            ++converted;

        return wait_for(lock, converted);
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

    //! \cond
    //! Waits on this condition variable with a timeout.
    //!
    //! This is an overload if the duration is specified in milliseconds.
    cv_status wait_for(unique_lock<mutex>& lock, chrono::milliseconds ms);
    //! \endcond

    // TODO: wait_until()

    //! Returns the native handle.
    native_handle_type native_handle()
    {
        return this;
    }

    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;

private:
    //! An object to wait on a signal.
    //! A WaitingThread can be enqueued in a list of waiters. The condition
    //! variable can either notify the first waiter or all waiters in the list.
    struct WaitingThread
    {
        WaitingThread()
            : next(0),
              dequeued(false)
        {
        }

        // The next waiting thread in the list.
        WaitingThread* next;
        // A semaphore to send a signal to this waiting thread.
        semaphore signal;
        // A flag which is set when the thread has been removed from the list.
        bool dequeued;
    };

    //! Adds the waiter \p w to the queue.
    void enqueue(WaitingThread& w);

    //! Removes the waiter \p w from the queue.
    void maybeDequeue(WaitingThread& w);

    //! A mutex to protect the list of waiters from concurrent modifications.
    mutex m_mutex;
    //! A pointer to the first waiter.
    WaitingThread* m_waitingThreads;
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP
