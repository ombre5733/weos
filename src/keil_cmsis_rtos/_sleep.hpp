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

#ifndef WEOS_KEIL_CMSIS_RTOS_SLEEP_HPP
#define WEOS_KEIL_CMSIS_RTOS_SLEEP_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "core.hpp"
#include "../chrono.hpp"

WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TClock, typename TDuration>
void sleep_until(const chrono::time_point<TClock, TDuration>& time)
{
    using namespace chrono;

    while (true)
    {
        auto remainingSpan = time - TClock::now();
        if (remainingSpan <= TDuration::zero())
            return;

        milliseconds converted = duration_cast<milliseconds>(remainingSpan);
        if (converted < milliseconds(1))
            converted = milliseconds(1);
        else if (converted > milliseconds(0xFFFE))
            converted = milliseconds(0xFFFE);

        osStatus result = osDelay(converted.count());
        if (result != osOK && result != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "sleep_until failed");
        }
    }
}

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_SLEEP_HPP
