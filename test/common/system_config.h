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

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

//! The frequency of the high-speed external (HSE) clock in Hz.
#define SYSTEM_EXTERNAL_CLOCK_FREQUENCY   8000000

//! The timeout until which the high-speed external (HSE) clock has to be
//! stable. After enabling the HSE clock, a counter is incremented in a
//! tight loop until either the HSERDY flag is set or the counter exceeds this
//! timeout value.
#define SYSTEM_HSE_READY_TIMEOUT   0x0600

//! The system clock frequency in Hz.
#define SYSTEM_CLOCK_FREQUENCY   168000000

//! The number of waitstates for the flash. Allowed values are 0-7.
#define SYSTEM_FLASH_WAITSTATES   5

#if   SYSTEM_CLOCK_FREQUENCY <= 42000000
#  define SYSTEM_SLOW_PERIPHERAL_DIVIDER   1
#elif SYSTEM_CLOCK_FREQUENCY <= 84000000
#  define SYSTEM_SLOW_PERIPHERAL_DIVIDER   2
#else
#  define SYSTEM_SLOW_PERIPHERAL_DIVIDER   4
#endif

#if SYSTEM_CLOCK_FREQUENCY <= 84000000
#  define SYSTEM_FAST_PERIPHERAL_DIVIDER   1
#else
#  define SYSTEM_FAST_PERIPHERAL_DIVIDER   2
#endif

//! The frequency of the (slow) APB1 bus in Hz.
#define SYSTEM_SLOW_PERIPHERAL_CLOCK                                           \
    (SYSTEM_CLOCK_FREQUENCY / SYSTEM_SLOW_PERIPHERAL_DIVIDER)

//! The frequency of the (fast) APB2 bus in Hz.
#define SYSTEM_FAST_PERIPHERAL_CLOCK                                           \
    (SYSTEM_CLOCK_FREQUENCY / SYSTEM_FAST_PERIPHERAL_DIVIDER)

#endif // SYSTEM_CONFIG_H
