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

#ifndef WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP
#define WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP

#include "core.hpp"

#include "../ratio.hpp"
#include "../type_traits.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

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


namespace detail
{

// Keil's CMSIS RTX limits the delay to 0xFFFE ticks (really ticks, not
// milliseconds, unless the SysTick period is 1 ms, in which case the number of
// ticks is equal to the number of milliseconds).
// If we want to block longer, we have to call the wait function multiple
// times. This helper class contains the necessary conversion.
template <typename TDuration>
struct internal_time_cast
{
    static_assert(!treat_as_floating_point<TDuration>::value,
                  "Floating point durations are not supported, yet.");

    // Create the ratio for converting from a duration d to another duration
    // t which is in milliseconds (t = d / 1e-3).
    typedef typename WEOS_NAMESPACE::ratio_divide<typename TDuration::period,
                                                  WEOS_NAMESPACE::milli>::type
                ratio;
    typedef typename WEOS_NAMESPACE::common_type<typename TDuration::rep,
                                                 std::int32_t>::type
                type;

    // Compute the maximum number of milliseconds such that the number of
    // ticks is <= 0xFFFE.
    static const type maxMillisecs =
            static_cast<type>(0xFFFE * 1000)
            / static_cast<type>(WEOS_SYSTICK_FREQUENCY);

    static type convert_and_clip(const TDuration& d)
    {
        if (d.count() <= 0)
            return type(0);

        // Convert the duration d to millisecs (the conversion ceils the ratio).
        type delay = (static_cast<type>(d.count())
                      * static_cast<type>(ratio::num)
                      + static_cast<type>(ratio::den - 1))
                     / static_cast<type>(ratio::den);

        return (delay < maxMillisecs) ? delay : maxMillisecs;
    }

    static type convert_and_clip_below(const TDuration& d)
    {
        if (d.count() <= 0)
            return type(0);

        // Convert the duration d to millisecs (the conversion ceils the ratio).
        type delay = (static_cast<type>(d.count())
                      * static_cast<type>(ratio::num)
                      + static_cast<type>(ratio::den - 1))
                     / static_cast<type>(ratio::den);
        return delay;
    }
};

} // namespace detail

} // namespace chrono

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP
