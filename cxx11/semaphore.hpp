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

#ifndef WEOS_CXX11_SEMAPHORE_HPP
#define WEOS_CXX11_SEMAPHORE_HPP

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace weos
{


class semaphore
{
public:
    semaphore()
        : m_value(0)
    {
    }

    void post()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_value;
        m_conditionVariable.notify_one();
    }

    bool try_wait()
    {
        return try_wait_for(std::chrono::seconds(0));
    }

    template <typename RepT, typename PeriodT>
    bool try_wait_for(const std::chrono::duration<RepT, PeriodT>& d)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_conditionVariable.wait_for(lock, d, [this](){return m_value != 0;}))
        {
            --m_value;
            return true;
        }
        return false;
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_value == 0)
            m_conditionVariable.wait(lock);
        --m_value;
    }

    std::uint32_t value() const
    {
        return m_value;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::uint32_t m_value;
};

} // namespace weos

#endif // WEOS_CXX11_SEMAPHORE_HPP
