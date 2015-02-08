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

#include "mutex.hpp"

WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     mutex
// ----=====================================================================----

mutex::mutex()
    : m_locked(false)
{
    os_mut_init(&m_mutex);
}

mutex::~mutex()
{
    WEOS_ASSERT(!m_locked);
}

void mutex::lock()
{
    OS_RESULT result = os_mut_wait(&m_mutex, 0xFFFF);
    if (result == OS_R_TMO)
        WEOS_THROW_SYSTEM_ERROR(result, "mutex::lock failed");

    if (!m_locked)
    {
        m_locked = true;
    }
    else
    {
        // The mutex has been locked twice. Undo one lock and throw an
        // exception.
        result = os_mut_release(&m_mutex);
        if (result != OS_R_OK)
            WEOS_THROW_SYSTEM_ERROR(result, "mutex::lock failed");

        WEOS_THROW_SYSTEM_ERROR(errc::resource_deadlock_would_occur,
                                "deadlock in mutex::lock");
    }
}

bool mutex::try_lock()
{
    OS_RESULT result = os_mut_wait(&m_mutex, 0);
    if (result == OS_R_TMO)
    {
        // Owned by another thread.
        return false;
    }
    if (result != OS_R_OK && result != OS_R_MUT)
        WEOS_THROW_SYSTEM_ERROR(result, "mutex::try_lock failed");

    if (!m_locked)
    {
        m_locked = true;
        return true;
    }
    else
    {
        // The mutex has already been locked by the current thread. Unlock
        // it once and pretend that try_lock() failed.
        result = os_mut_release(&m_mutex);
        if (result != OS_R_OK)
            WEOS_THROW_SYSTEM_ERROR(result, "mutex::try_lock failed");
        return false;
    }
}

void mutex::unlock()
{
    WEOS_ASSERT(m_locked);
    m_locked = false;
    OS_RESULT result = os_mut_release(&m_mutex);
    if (result != OS_R_OK)
        WEOS_THROW_SYSTEM_ERROR(result, "mutex::unlock failed");
}

// ----=====================================================================----
//     recursive_mutex
// ----=====================================================================----

recursive_mutex::recursive_mutex()
{
    os_mut_init(&m_mutex);
}

recursive_mutex::~recursive_mutex()
{
}

void recursive_mutex::lock()
{
    OS_RESULT result = os_mut_wait(&m_mutex, 0xFFFF);
    if (result == OS_R_TMO)
        WEOS_THROW_SYSTEM_ERROR(result, "recursive_mutex::lock failed");
}

bool recursive_mutex::try_lock()
{
    OS_RESULT result = os_mut_wait(&m_mutex, 0);
    if (result == OS_R_TMO)
    {
        // The mutex is owned by another thread.
        return false;
    }
    if (result != OS_R_OK && result != OS_R_MUT)
        WEOS_THROW_SYSTEM_ERROR(result,
                                "recursive_mutex::try_lock failed");

    return true;
}

void recursive_mutex::unlock()
{
    OS_RESULT result = os_mut_release(&m_mutex);
    if (result != OS_R_OK)
        WEOS_THROW_SYSTEM_ERROR(result, "recursive_mutex::unlock failed");
}

WEOS_END_NAMESPACE
