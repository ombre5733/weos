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

#include "_semaphore.hpp"


WEOS_BEGIN_NAMESPACE

semaphore::~semaphore()
{
}

void semaphore::post()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mutex.lock();
    ++m_value;
    m_mutex.unlock();
    m_conditionVariable.notify_one();
}

void semaphore::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(lock, [this] { return m_value != 0; });
    --m_value;
}

bool semaphore::try_wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_value > 0)
    {
        --m_value;
        return true;
    }
    else
    {
        return false;
    }
}

semaphore::value_type semaphore::value() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_value;
}

WEOS_END_NAMESPACE
