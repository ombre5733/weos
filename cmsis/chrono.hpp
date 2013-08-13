/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#ifndef CMSIS_CHRONO_HPP
#define CMSIS_CHRONO_HPP

#include <boost/ratio.hpp>

#include <cstdint>
#include <limits>

namespace osl
{
namespace chrono
{

using boost::ratio;
using boost::milli;

// ----=====================================================================----
//     duration_values
// ----=====================================================================----

template <class Rep>
struct duration_values
{
    static BOOST_CONSTEXPR Rep zero()
    {
        return Rep(0);
    }

    static BOOST_CONSTEXPR Rep min()
    {
        return std::numeric_limits<Rep>::lowest();
    }

    static BOOST_CONSTEXPR int32_t max()
    {
        return std::numeric_limits<Rep>::max();
    }
};

// ----=====================================================================----
//     treat_as_floating_point
// ----=====================================================================----

//template <class Rep>
//struct treat_as_floating_point : std::is_floating_point<Rep> {};

// ----=====================================================================----
//     duration
// ----=====================================================================----

template <typename Rep, typename Period>
class duration
{
public:
    typedef Rep rep;
    typedef Period period;

    BOOST_CONSTEXPR duration() /*= default*/
    {
        m_count = duration_values<rep>::zero();
    }

    duration(const duration& other) /*= default*/
        : m_count(other.m_count)
    {
    }

    template <typename Rep2>
    BOOST_CONSTEXPR explicit duration(const Rep2& count)
    {
        //! \todo Conversion between Rep and Rep2 is missing
        m_count = count;
    }

    BOOST_CONSTEXPR rep count() const
    {
        return m_count;
    }

    static BOOST_CONSTEXPR duration zero()
    {
        return duration(duration_values<rep>::zero());
    }

    static BOOST_CONSTEXPR duration min()
    {
        return duration(duration_values<rep>::min());
    }

    static BOOST_CONSTEXPR duration max()
    {
        return duration(duration_values<rep>::max());
    }

private:
    rep m_count;
};

typedef duration<int32_t, milli> milliseconds;

// ----=====================================================================----
//     time_point
// ----=====================================================================----

template <typename Clock, typename Duration = typename Clock::duration>
class time_point
{
public:
    typedef Clock clock;
    typedef Duration duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;

    BOOST_CONSTEXPR time_point()
    {
    }

    BOOST_CONSTEXPR explicit time_point(const duration& duration)
        : m_duration(duration)
    {
    }

    duration time_since_epoch() const
    {
        return m_duration;
    }

private:
    duration m_duration;
};

// ----=====================================================================----
//     high_resolution_clock
// ----=====================================================================----

class high_resolution_clock
{
    typedef int32_t rep;
    typedef ratio<1, 100000/* SHOULD BE: osKernelSysTickFrequency*/> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = false;

    /*
    static time_point now()
    {
        return osKernelSysTick();
    }
    */
};

} // namespace chrono
} // namespace osl

#endif // CMSIS_CHRONO_HPP
