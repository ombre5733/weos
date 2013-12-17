/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
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

#ifndef WEOS_KEIL_RL_RTX_CHRONO_HPP
#define WEOS_KEIL_RL_RTX_CHRONO_HPP

#include "../config.hpp"
#include "hal.hpp"
#include "../common/duration.hpp"
#include "../common/timepoint.hpp"

#include <cstdint>

namespace weos
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
    typedef std::int32_t rep;
    typedef boost::ratio<1, WEOS_SYSTICK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<system_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(os_time_get()));
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
    typedef boost::ratio<1, WEOS_SYSTEM_CLOCK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(hal::getSysTickValue()));
    }
};

namespace detail
{

// Keil's RL RTX specifies delay times in ticks but we want to have a
// duration in the API. Furthermore, the delay is limited to 0xFFFE ticks.
// If we want to block longer, we have to call the wait function multiple
// times. This helper contains the necessary boilerplate code.
template <typename RepT, typename PeriodT, typename FunctorT>
struct rl_rtx_wait
{
    // Create the ratio for converting from a duration d to a tick-count c via
    // the systick frequency f (c = d * f).
    typedef typename boost::ratio_multiply<
                         PeriodT,
                         boost::ratio<WEOS_SYSTICK_FREQUENCY> >::type ratio;
    typedef typename boost::common_type<RepT, cast_least_int_type>::type
        common_type;

    static bool wait(const chrono::duration<RepT, PeriodT>& d,
                     const FunctorT& fun)
    {
        if (d.count() <= 0)
            return fun(0);

        // Convert the duration d to counts (the conversion ceils the ratio).
        common_type counts
                = (static_cast<common_type>(d.count())
                   * static_cast<common_type>(ratio::num)
                   + static_cast<common_type>(ratio::den - 1))
                  / static_cast<common_type>(ratio::den);
        // Note: A tick of 1 will wake the thread at the beginning of the next
        // period. However, some time has already passed in the current period,
        // so the actual delay is less than a tick. Thus we increase the
        // count by 1 to have a strict lower bound.
        counts += common_type(1);
        while (counts > 0xFFFE)
        {
            bool success = fun(0xFFFE);
            if (success)
                return true;
            counts -= 0xFFFE;
        }
        return fun(counts);
    }
};

} // namespace detail

} // namespace chrono
} // namespace weos

#endif // WEOS_KEIL_RL_RTX_CHRONO_HPP
