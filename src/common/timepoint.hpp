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

#ifndef WEOS_COMMON_TIMEPOINT_HPP
#define WEOS_COMMON_TIMEPOINT_HPP

#include "../config.hpp"
#include "duration.hpp"

#include <boost/config.hpp>

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
    //! Creates a time point whose difference to the epoch time is equal
    //! to the given duration \p d.
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

template <typename ClockT, typename Duration1T, typename Duration2T>
inline
BOOST_CONSTEXPR
typename boost::common_type<Duration1T, Duration2T>::type
    operator- (const time_point<ClockT, Duration1T>& x,
               const time_point<ClockT, Duration2T>& y)
{

    return x.time_since_epoch() - y.time_since_epoch();
}

} // namespace chrono
} // namespace weos

#endif // WEOS_COMMON_TIMEPOINT_HPP
