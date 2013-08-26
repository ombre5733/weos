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

#ifndef WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP
#define WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP

#include "cmsis_os.h"

#include "duration.hpp"

namespace weos
{
namespace chrono
{

// ----=====================================================================----
//     time_point
// ----=====================================================================----

//! A time point.
template <typename ClockT, typename DurationT = typename ClockT::duration>
class time_point
{
public:
    typedef ClockT clock;
    typedef DurationT duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;

    BOOST_CONSTEXPR time_point()
    {
    }

    //! Creates a time point from a duration.
    BOOST_CONSTEXPR explicit time_point(const duration& d)
        : m_duration(d)
    {
    }

    //! Returns the time point relative to the clock's epoch.
    //! Returns the time point as a duration since the clock's epoch.
    duration time_since_epoch() const
    {
        return m_duration;
    }

    // Arithmetic operators.

    //! Adds a duration.
    //! Adds the duration \p d to this time point and returns the time point.
    time_point& operator+= (const duration& d)
    {
        m_duration += d;
        return *this;
    }

    //! Subtracts a duration.
    //! Subtracts the duration \p d from this time point and returns the time
    //! point.
    time_point& operator-= (const duration& d)
    {
        m_duration -= d;
        return *this;
    }

    // Special values.

    static BOOST_CONSTEXPR time_point max()
    {
        return time_point(duration::max());
    }

    static BOOST_CONSTEXPR time_point min()
    {
        return time_point(duration::min());
    }

private:
    duration m_duration;
};

// ----=====================================================================----
//     system_clock
// ----=====================================================================----

//! The system clock.
//! The system clock's period is equal to the time between two OS ticks. This
//! period has to be set with the OS_TICK macro (in us).
class system_clock
{
public:
    typedef int32_t rep;
    typedef boost::ratio<OS_TICK, 1000000> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<system_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = true;

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
//! set with the OS_CLOCK macro (in Hz).
class high_resolution_clock
{
public:
    typedef int32_t rep;
    typedef boost::ratio<1, OS_CLOCK> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(osKernelSysTick()));
    }
};

namespace detail
{

template <typename RepT, typename PeriodT, typename FunctorT>
struct cmsis_wait
{
    static bool wait(const chrono::duration<RepT, PeriodT>& d,
                     const FunctorT& fun)
    {
        if (d.count() <= 0)
            return fun(0);

        // Keil's CMSIS RTX limits the delay to 0xFFFE ticks. If we want to
        // block longer, we have to issue multiple calls.

        typedef chrono::milliseconds::rep rep;
        // A delay time such that numTicks := d.count() * 1000 / OS_TICK is
        // for sure smaller than or equal to 0xFFFE.
        const rep maxDelay = 65 * static_cast<rep>(OS_TICK);

        rep count = d.count();
        while (count > maxDelay)
        {
            bool success = fun(maxDelay);
            if (success)
                return true;
            count -= maxDelay;
        }
        return fun(count);
    }
};

} // namespace detail

} // namespace chrono
} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_CHRONO_HPP
