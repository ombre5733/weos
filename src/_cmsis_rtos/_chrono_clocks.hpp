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

#ifndef WEOS_CMSIS_RTOS_CHRONO_CLOCKS_HPP
#define WEOS_CMSIS_RTOS_CHRONO_CLOCKS_HPP

#include "_core.hpp"

#include "../ratio.hpp"
#include "../type_traits.hpp"

#include <cstdint>


namespace WEOS_STD_NAMESPACE
{
namespace chrono
{

// ----=====================================================================----
//     system_clock
// ----=====================================================================----

//! The system clock.
//! The system clock's period is equal to the time between two OS ticks. The
//! corresponding frequency has to be set with the WEOS_SYSTICK_FREQUENCY macro.
class system_clock
{
public:
    typedef std::int64_t rep;
    typedef ratio<1, WEOS_SYSTICK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<system_clock> time_point;

    static constexpr bool is_steady = true;

    static time_point now();
};

// ----=====================================================================----
//     high_resolution_clock
// ----=====================================================================----

//! The high-resolution clock.
//! This class provides access to the system's high-resolution clock. The
//! frequency of this clock is equal to the sys-tick timer, which has to be
//! set with the WEOS_SYSTEM_CLOCK_FREQUENCY macro (in Hz).
class high_resolution_clock
{
public:
    typedef std::int64_t rep;
    typedef ratio<1, WEOS_SYSTEM_CLOCK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static constexpr bool is_steady = true;

    static time_point now();
};

// ----=====================================================================----
//     steady_clock
// ----=====================================================================----

typedef system_clock steady_clock;

} // namespace chrono
} // namespace WEOS_STD_NAMESPACE

#endif // WEOS_CMSIS_RTOS_CHRONO_CLOCKS_HPP
