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

#include "semaphore.hpp"


WEOS_BEGIN_NAMESPACE

semaphore::semaphore(value_type value)
    : m_id(0)
{
    // Keil's RTOS wants a zero'ed control block type for initialization.
    m_controlBlock._[0] = 0;
    osSemaphoreDef_t semaphoreDef = { m_controlBlock._ };
    m_id = osSemaphoreCreate(&semaphoreDef, value);
    if (m_id == 0)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                "semaphore::semaphore failed");
}

semaphore::~semaphore()
{
    if (m_id)
        osSemaphoreDelete(m_id);
}

void semaphore::post()
{
    osStatus status = osSemaphoreRelease(m_id);
    if (status != osOK)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(status),
                                "semaphore::post failed");
}

void semaphore::wait()
{
    std::int32_t result = osSemaphoreWait(m_id, osWaitForever);
    if (result <= 0)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                "semaphore::wait failed");
}

bool semaphore::try_wait()
{
    std::int32_t result = osSemaphoreWait(m_id, 0);
    if (result < 0)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                "semaphore::try_wait failed");

    return result != 0;
}

semaphore::value_type semaphore::value() const
{
    //! \todo Use an SVC here.
    return m_controlBlock.numTokens();
}

WEOS_END_NAMESPACE
