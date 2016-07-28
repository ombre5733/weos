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

#include "_mutex.hpp"

namespace std
{

// ----=====================================================================----
//     mutex
// ----=====================================================================----

mutex::~mutex()
{
    WEOS_ASSERT(!m_locked);
    osMutexDelete(native_handle());
}

void mutex::lock()
{
    osStatus result = osMutexWait(native_handle(), osWaitForever);
    if (result != osOK)
        WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::cmsis_error_t(result),
                                "mutex::lock failed");

    if (!m_locked)
    {
        m_locked = true;
    }
    else
    {
        // The mutex has been locked twice. Undo one lock and throw an
        // exception.
        result = osMutexRelease(native_handle());
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::cmsis_error_t(result),
                                    "mutex::lock failed");
        WEOS_THROW_SYSTEM_ERROR(std::errc::resource_deadlock_would_occur,
                                "deadlock in mutex::lock");
    }
}

bool mutex::try_lock() noexcept
{
    osStatus result = osMutexWait(native_handle(), 0);

    // If osMutexWait(mutex, millisec) fails to acquire a mutex within the
    // timeout, the error code is
    //   osErrorTimeoutResource if millisec != 0
    //   osErrorResource if millisec == 0
    if (result == osErrorResource)
    {
        // The mutex is owned by another thread.
        return false;
    }
    if (result != osOK)
        return false;

    if (!m_locked)
    {
        m_locked = true;
        return true;
    }
    else
    {
        // The mutex has already been locked by the current thread. Unlock
        // it once and pretend that try_lock() failed.
        result = osMutexRelease(native_handle());
        (void)result;
        WEOS_ASSERT(result == osOK);
        return false;
    }
}

void mutex::unlock() noexcept
{
    WEOS_ASSERT(m_locked);
    m_locked = false;
    osStatus result = osMutexRelease(native_handle());
    (void)result;
    WEOS_ASSERT(result == osOK);
}

// ----=====================================================================----
//     timed_mutex
// ----=====================================================================----

bool timed_mutex::try_lock_for(chrono::milliseconds ms)
{
    using namespace chrono;

    if (ms < ms.zero())
        ms = ms.zero();

    do
    {
        static_assert(osCMSIS_RTX <= ((4<<16) | 80),
                      "Check the maximum timeout.");
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;

        osStatus result = osMutexWait(native_handle(), truncated.count());
        if (result == osOK)
            return true;

        if (   result != osErrorResource
            && result != osErrorTimeoutResource)
        {
            WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::osErrorOS,
                                    "semaphore::try_wait_for failed");
        }

    } while (ms > ms.zero());

    return false;
}

// ----=====================================================================----
//     recursive_mutex
// ----=====================================================================----

recursive_mutex::~recursive_mutex()
{
    osMutexDelete(native_handle());
}

void recursive_mutex::lock()
{
    osStatus result = osMutexWait(native_handle(), osWaitForever);
    if (result != osOK)
        WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::cmsis_error_t(result),
                                "recursive_mutex::lock failed");
}

bool recursive_mutex::try_lock() noexcept
{
    return osMutexWait(native_handle(), 0) == osOK;
}

void recursive_mutex::unlock() noexcept
{
    osStatus result = osMutexRelease(native_handle());
    (void)result;
    WEOS_ASSERT(result == osOK);
}

// ----=====================================================================----
//     recursive_timed_mutex
// ----=====================================================================----

bool recursive_timed_mutex::try_lock_for(chrono::milliseconds ms)
{
    using namespace chrono;

    if (ms < ms.zero())
        ms = ms.zero();

    do
    {
        static_assert(osCMSIS_RTX <= ((4<<16) | 80),
                      "Check the maximum timeout.");
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;

        osStatus result = osMutexWait(native_handle(), truncated.count());
        if (result == osOK)
            return true;

        if (   result != osErrorResource
            && result != osErrorTimeoutResource)
        {
            WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::osErrorOS,
                                    "semaphore::try_wait_for failed");
        }

    } while (ms > ms.zero());

    return false;
}

} // namespace std
