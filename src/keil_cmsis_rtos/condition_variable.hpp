/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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

#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"


WEOS_BEGIN_NAMESPACE

namespace detail
{
//! A helper class for temporarily releasing a lock.
//! The lock_releaser is a helper class to release a lock until the object
//! goes out of scope. The constructor calls unlock() and the destructor
//! calls lock(). It is somehow the dual to the lock_guard which calls lock()
//! in the constructor and unlock() in the destructor.
template <typename LockT>
class lock_releaser
{
public:
    typedef LockT lock_type;

    explicit lock_releaser(lock_type& lock)
        : m_lock(lock)
    {
        m_lock.unlock();
    }

    ~lock_releaser()
    {
        m_lock.lock();
    }

private:
    lock_type& m_lock;
};

} // namespace detail



namespace cv_status
{
    enum cv_status
    {
        no_timeout,
        timeout
    };
} // namespace cv_status

//! A condition variable.
class condition_variable
{
public:
    condition_variable()
        : m_waitingThreads(0)
    {
    }

    //! Destroys the condition variable.
    //!
    //! \note The condition variable must not be destroyed if a thread is
    //! waiting on it.
    ~condition_variable()
    {
        WEOS_ASSERT(m_waitingThreads == 0);
    }

    //! Notifies a thread waiting on this condition variable.
    //! Notifies one thread which is waiting on this condition variable.
    void notify_one() WEOS_NOEXCEPT
    {
        lock_guard<mutex> locker(m_mutex);

        if (m_waitingThreads != 0)
        {
            WaitingThread* next = m_waitingThreads->next;
            m_waitingThreads->dequeued = true;
            m_waitingThreads->signal.post();
            m_waitingThreads = next;
        }
    }

    //! Notifies all threads waiting on this condition variable.
    //! Notifies all threads which are waiting on this condition variable.
    void notify_all() WEOS_NOEXCEPT
    {
        lock_guard<mutex> locker(m_mutex);

        while (m_waitingThreads)
        {
            WaitingThread* next = m_waitingThreads->next;
            m_waitingThreads->dequeued = true;
            m_waitingThreads->signal.post();
            m_waitingThreads = next;
        }
    }

    //! Waits on this condition variable.
    //! The given \p lock is released and the current thread is added to a
    //! list of threads waiting for a notification. The calling thread is
    //! blocked until a notification is sent via notify() or notify_all()
    //! or a spurious wakeup occurs. The \p lock is reacquired when the
    //! function exits (either due to a notification or due to an exception).
    void wait(unique_lock<mutex>& lock)
    {
        // First enqueue ourselfs in the list of waiters.
        WaitingThread w;
        enqueue(w);

        // We can only release the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser<unique_lock<mutex> > releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait();
        }
    }

    //! Waits on this condition variable with a timeout.
    //! Releases the given \p lock and adds the calling thread to a list
    //! of threads waiting for a notification. The thread is blocked until
    //! a notification is sent, a spurious wakeup occurs or the timeout
    //! period \p d expires. When the function returns, the \p lock is
    //! reacquired no matter what has caused the wakeup.
    template <typename RepT, typename PeriodT>
    cv_status::cv_status wait_for(unique_lock<mutex>& lock,
                                  const chrono::duration<RepT, PeriodT>& d)
    {
        // First enqueue ourselfs in the list of waiters.
        WaitingThread w;
        enqueue(w);

        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser<unique_lock<mutex> > releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            bool gotSignal = w.signal.try_wait_for(d);
            if (!gotSignal)
            {
                lock_guard<mutex> locker(m_mutex);
                if (!w.dequeued)
                    dequeue(w);
                return cv_status::timeout;
            }
        }
        return cv_status::no_timeout;
    }

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
    void enqueue(WaitingThread& w)
    {
        lock_guard<mutex> locker(m_mutex);

        if (!m_waitingThreads)
            m_waitingThreads = &w;
        else
        {
            //! \todo enqueue using priorities
            WaitingThread* iter = m_waitingThreads;
            while (iter->next)
                iter = iter->next;
            iter->next = &w;
        }
    }

    //! Removes the waiter \p w from the queue.
    void dequeue(WaitingThread& w)
    {
        WEOS_ASSERT(m_waitingThreads);

        if (m_waitingThreads == &w)
            m_waitingThreads = w.next;
        else
        {
            WaitingThread* iter = m_waitingThreads;
            while (iter->next != &w)
            {
                iter = iter->next;
                WEOS_ASSERT(iter);
            }
            iter->next = w.next;
        }
    }

    //! A mutex to protect the list of waiters from concurrent modifications.
    mutex m_mutex;
    //! A pointer to the first waiter.
    WaitingThread* m_waitingThreads;


    condition_variable(const condition_variable&);
    condition_variable& operator= (const condition_variable&);
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP
