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
        lock_guard<recursive_timed_mutex> locker(m_mutex);

        Waiter* head = m_waiters;
        if (head != 0)
        {
            m_waiters = head->next;
            head->dequeued = true;
            head->signal.release();
        }
    }

    void notify_all() BOOST_NOEXCEPT
    {
        lock_guard<recursive_timed_mutex> locker(m_mutex);

        for (Waiter* head = m_waiters; head != 0; head = head->next)
        {
            head->dequeued = true;
            head->signal.release();
        }
    }

    void wait(unique_lock<mutex>& lock)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        {
            lock_guard<recursive_timed_mutex> locker(m_mutex);

            //! \todo enqueue using priorities and change to a FIFO
            w.next = m_waiters;
            m_waiters = &w;
        }

        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait();
        }
    }

    template <typename RepT, typename PeriodT>
    void wait_for(unique_lock<recursive_timed_mutex>& lock,
                  const chrono::duration<RepT, PeriodT>& timeout)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        {
            lock_guard<recursive_timed_mutex> locker(m_mutex);

            //! \todo enqueue using priorities and change to a FIFO
            w.next = m_waiters;
            m_waiters = &w;
        }

        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait_for(timeout);
            if (timedout)
            {
                lock_guard<recursive_timed_mutex> locker(m_mutex);
                if (!w.dequeued)
                {
                    remove_w_from_queue();
                }
            }
        }
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

    recursive_timed_mutex m_mutex;
    Waiter* m_waiters;
};

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_CONDITIONVARIABLE_HPP
