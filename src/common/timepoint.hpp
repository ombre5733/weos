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

#ifndef WEOS_COMMON_TIMEPOINT_HPP
#define WEOS_COMMON_TIMEPOINT_HPP

#include "../config.hpp"
#include "duration.hpp"


WEOS_BEGIN_NAMESPACE

namespace chrono
{

// ----=====================================================================----
//     time_point
// ----=====================================================================----

//! A time point.
template <typename ClockT, typename DurationT = typename ClockT::duration>
class time_point
{
    static_assert(detail::is_duration<DurationT>::value,
                  "The second template parameter must be a duration");

public:
    typedef ClockT clock;
    typedef DurationT duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;

    WEOS_CONSTEXPR time_point()
        : m_duration(duration::zero())
    {
    }

    //! Creates a time point from a duration.
    //! Creates a time point whose difference to the epoch time is equal
    //! to the given duration \p d.
    WEOS_CONSTEXPR explicit time_point(const duration& d)
        : m_duration(d)
    {
    }

    //! Creates a time point from another time point.
    //! Creates a time point from the other time point \p tp.
    template <typename TDuration2,
              typename _ = typename enable_if<
                               is_convertible<TDuration2, duration>::value>::type>
    WEOS_CONSTEXPR
    time_point(const time_point<clock, TDuration2>& tp)
        : m_duration(tp.time_since_epoch())
    {
    }

    //! Returns the time point relative to the clock's epoch.
    //! Returns the time point as a duration since the clock's epoch.
    WEOS_CONSTEXPR duration time_since_epoch() const
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

    static WEOS_CONSTEXPR time_point max()
    {
        return time_point(duration::max());
    }

    static WEOS_CONSTEXPR time_point min()
    {
        return time_point(duration::min());
    }

private:
    duration m_duration;
};

} // namespace chrono

// ----=====================================================================----
//     Specialization of common_type for chrono::time_point<>
// ----=====================================================================----

template <typename TClock, typename TDuration1, typename TDuration2>
struct common_type<chrono::time_point<TClock, TDuration1>,
                   chrono::time_point<TClock, TDuration2> >
{
    typedef chrono::time_point<TClock,
                               typename common_type<TDuration1, TDuration2>::type>
        type;
};

namespace chrono
{

// ----=====================================================================----
//     time_point_cast
// ----=====================================================================----

template <typename TToDuration, typename TClock, typename TFromDuration>
inline WEOS_CONSTEXPR
time_point<TClock, TToDuration>
time_point_cast(const time_point<TClock, TFromDuration>& tp)
{
    return time_point<TClock, TToDuration>(duration_cast<TToDuration>(
                                               tp.time_since_epoch()));
}

// ----=====================================================================----
//     time_point comparisons
// ----=====================================================================----

template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator==(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() == y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator!=(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() != y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator<(const time_point<TClock, TDuration1>& x,
          const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() < y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator>(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() > y.time_since_epoch();
}


template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator<=(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() <= y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline WEOS_CONSTEXPR
bool
operator>=(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() >= y.time_since_epoch();
}

// ----=====================================================================----
//     time_point arithmetic
// ----=====================================================================----

template <typename TClock, typename TDuration1,
          typename TRep2, typename TPeriod2>
inline WEOS_CONSTEXPR
time_point<TClock,
           typename common_type<TDuration1, duration<TRep2, TPeriod2> >::type>
operator+(const time_point<TClock, TDuration1>& tp,
          const duration<TRep2, TPeriod2>& d)
{
    typedef typename common_type<TDuration1,
                                 duration<TRep2, TPeriod2> >::type
            common_duration;

    return time_point<TClock, common_duration>(tp.time_since_epoch() + d);
}

template <typename TRep1, typename TPeriod1, typename TClock, typename TDuration2>
inline WEOS_CONSTEXPR
time_point<TClock,
           typename common_type<duration<TRep1, TPeriod1>, TDuration2>::type>
operator+(const duration<TRep1, TPeriod1>& d,
          const time_point<TClock, TDuration2>& tp)
{
    return tp + d;
}

template <typename TClock, typename TDuration1, typename TRep2, typename TPeriod2>
inline WEOS_CONSTEXPR
time_point<TClock,
           typename common_type<TDuration1, duration<TRep2, TPeriod2> >::type>
operator-(const time_point<TClock, TDuration1>& tp,
          const duration<TRep2, TPeriod2>& d)
{
    typedef typename common_type<TDuration1,
                                 duration<TRep2, TPeriod2> >::type
            common_duration;

    return time_point<TClock, common_duration>(tp.time_since_epoch() - d);
}

template <typename ClockT, typename Duration1T, typename Duration2T>
inline WEOS_CONSTEXPR
typename common_type<Duration1T, Duration2T>::type
operator- (const time_point<ClockT, Duration1T>& x,
           const time_point<ClockT, Duration2T>& y)
{
    return x.time_since_epoch() - y.time_since_epoch();
}

} // namespace chrono

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_TIMEPOINT_HPP
