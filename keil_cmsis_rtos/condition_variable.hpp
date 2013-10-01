/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
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

#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"

#include <boost/utility.hpp>

namespace weos
{

namespace detail
{
//! A helper class for temporarily releasing a lock.
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

enum cv_status
{
    no_timeout,
    timeout
};

//! A condition variable.
class condition_variable : boost::noncopyable
{
public:
    condition_variable()
        : m_waiters(0)
    {
    }

    ~condition_variable()
    {
        assert(m_waiters == 0);
    }

    void notify_one() BOOST_NOEXCEPT
    {
        lock_guard<mutex> locker(m_mutex);

        Waiter* head = m_waiters;
        if (head != 0)
        {
            m_waiters = head->next;
            head->dequeued = true;
            head->signal.post();
        }
    }

    void notify_all() BOOST_NOEXCEPT
    {
        lock_guard<mutex> locker(m_mutex);

        for (Waiter* head = m_waiters; head != 0; head = head->next)
        {
            head->dequeued = true;
            head->signal.post();
        }
        m_waiters = 0;
    }

    void wait(unique_lock<mutex>& lock)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        enqueueWaiter(w);

        // We can only release the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser<unique_lock<mutex> > releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait();
        }
    }

    template <typename RepT, typename PeriodT>
    cv_status wait_for(unique_lock<mutex>& lock,
                       const chrono::duration<RepT, PeriodT>& d)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        enqueueWaiter(w);

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
                    dequeueWaiter(w);
                return timeout;
            }
        }
        return no_timeout;
    }

private:
    struct Waiter
    {
        Waiter()
            : next(0),
              dequeued(false)
        {
        }

        Waiter* next;
        semaphore signal;
        bool dequeued;
    };

    //! Adds the waiter \p w to the queue.
    void enqueueWaiter(Waiter& w)
    {
        lock_guard<mutex> locker(m_mutex);

        //! \todo enqueue using priorities
        if (m_waiters)
        {
            Waiter* iter = m_waiters;
            while (iter->next)
                iter = iter->next;
            iter->next = &w;
        }
        else
        {
            m_waiters = &w;
        }
    }

    //! Removes the waiter \p w from the queue.
    void dequeueWaiter(Waiter& w)
    {
        if (m_waiters == &w)
            m_waiters = w.next;
        else
        {
            Waiter* iter = m_waiters;
            if (iter->next == &w)
                iter->next = w.next;
        }
    }

    mutex m_mutex;
    Waiter* m_waiters;
};

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP
