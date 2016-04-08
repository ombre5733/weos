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

#include "_sleep.hpp"

namespace std
{
namespace this_thread
{

void sleep_for(chrono::milliseconds ms)
{
    using namespace chrono;

    if (ms <= ms.zero())
        return;

    // An osDelay() of 1ms only blocks until the following time slot.
    // As some time already has passed in this slot, the true delay is
    // shorter than what the user specified.

    // We increase the number of milliseconds by one here.
    // TODO: Fix this for clocks other than milliseconds.
    static_assert(   chrono::system_clock::period::num == 1
                  && chrono::system_clock::period::den == 1000,
                  "Only implemented for milliseconds");
    ++ms;

    while (ms > ms.zero())
    {
        static_assert(osCMSIS_RTX <= ((4<<16) | 78),
                      "Check the maximum timeout.");
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;
        osStatus result = osDelay(truncated.count());
        if (result != osOK && result != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::cmsis_error_t(result),
                                    "sleep_for failed");
        }
    }
}

} // namespace this_thread
} // namespace std
