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

#include "semaphore.hpp"

#include "hal.hpp"


WEOS_BEGIN_NAMESPACE

semaphore::semaphore(value_type value)
{
    os_sem_init(&m_semaphore, value);
}

void semaphore::post()
{
    if (hal::isInIsr())
    {
        isr_sem_send(&m_semaphore);
    }
    else
    {
        OS_RESULT result = os_sem_send(&m_semaphore);
        if (result != OS_R_OK) todo: is this the correct return code?
            WEOS_THROW_SYSTEM_ERROR(result, "semaphore::post failed");
    }
}

void semaphore::wait()
{
    OS_RESULT result = os_sem_wait(&m_semaphore, 0xFFFF);
    if (result == OS_R_TMO)
        WEOS_THROW_SYSTEM_ERROR(result, "semaphore::wait failed");
}

bool semaphore::try_wait()
{
    OS_RESULT result = os_sem_wait(&m_semaphore, 0);
    return result != OS_R_TMO;
}

// The header (first 32 bits) of the semaphore control block. The full
// definition can be found in ${Keil-RL-RTX}/SRC/rt_TypeDef.h.
struct SemaphoreControlBlockHeader
{
    todo: check this again because it is not well aligned
    volatile std::uint8_t controlBlockType;
    volatile std::uint16_t numTokens;
    volatile std::uint8_t unused;
};

semaphore::value_type semaphore::value()
{
    return reinterpret_cast<const SemaphoreControlBlockHeader*>(
                &m_semaphore)->numTokens;
}

WEOS_END_NAMESPACE
