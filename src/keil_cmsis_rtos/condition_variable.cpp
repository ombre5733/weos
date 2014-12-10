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

#include "condition_variable.hpp"


WEOS_BEGIN_NAMESPACE

condition_variable::condition_variable()
    : m_waitingThreads(0)
{
}

condition_variable::~condition_variable()
{
    WEOS_ASSERT(m_waitingThreads == 0);
}

void condition_variable::notify_one() WEOS_NOEXCEPT
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

void condition_variable::notify_all() WEOS_NOEXCEPT
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

void condition_variable::wait(unique_lock<mutex>& lock)
{
    // First enqueue ourselves in the list of waiters.
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

WEOS_END_NAMESPACE
