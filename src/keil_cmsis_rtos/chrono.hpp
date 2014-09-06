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

#include "../common/duration.hpp"
#include "../common/ratio.hpp"
#include "../common/timepoint.hpp"
#include "../type_traits.hpp"

#include <cstdint>

// Declaration from ../3rdparty/keil_cmsis_rtos/SRC/rt_Time.h.
extern "C" std::uint32_t rt_time_get(void);


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
    typedef std::int32_t rep;
    typedef ratio<1, WEOS_SYSTICK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<system_clock> time_point;

    static WEOS_CONSTEXPR_OR_CONST bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(rt_time_get()));
    }
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
    typedef std::int32_t rep;
    typedef ratio<1, WEOS_SYSTEM_CLOCK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static WEOS_CONSTEXPR_OR_CONST bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(osKernelSysTick()));
    }
};

namespace detail
{

// Keil's CMSIS RTX limits the delay to 0xFFFE ticks (really ticks, not
// milliseconds, unless the SysTick period is 1 ms, in which case the number of
// ticks is equal to the number of milliseconds).
// If we want to block longer, we have to call the wait function multiple
// times. This helper contains the necessary boilerplate code.
template <typename RepT, typename PeriodT, typename FunctorT>
struct cmsis_wait
{
    // Create the ratio for converting from a duration d to another duration
    // t which is in milliseconds (t = d / 1e-3).
    typedef typename ratio_divide<PeriodT, milli>::type ratio;
    typedef typename common_type<RepT, cast_least_int_type>::type
        common_type;

    // Compute the maximum number of milliseconds which correspond to a SysTick
    // value of less than 0xFFFF.
    static const common_type maxMillisecs =
            static_cast<common_type>(0xFFFE * 1000)
            / static_cast<common_type>(WEOS_SYSTICK_FREQUENCY);

    static bool wait(const chrono::duration<RepT, PeriodT>& d, FunctorT& fun)
    {
        if (d.count() <= 0)
            return fun(0);

        // Convert the duration d to millisecs (the conversion ceils the ratio).
        common_type delay
                = (static_cast<common_type>(d.count())
                   * static_cast<common_type>(ratio::num)
                   + static_cast<common_type>(ratio::den - 1))
                  / static_cast<common_type>(ratio::den);
        // Note: A tick of 1 will wake the thread at the beginning of the next
        // period. However, some time has already passed in the current period,
        // so the actual delay is less than a tick.
        // Thus, we increase the delay by 1 ms to ensure that our delay time
        // is a lower bound.
        delay += common_type(1);
        while (delay > maxMillisecs)
        {
            bool success = fun(maxMillisecs);
            if (success)
                return true;
            delay -= maxMillisecs;
        }
        return fun(delay);
    }
};

} // namespace detail

} // namespace chrono

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP
