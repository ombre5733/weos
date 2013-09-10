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

#ifndef WEOS_KEIL_CMSIS_RTOS_DURATION_HPP
#define WEOS_KEIL_CMSIS_RTOS_DURATION_HPP

#include "../config.hpp"

#include <boost/config.hpp>
#include <boost/ratio.hpp>
#include <boost/type_traits/common_type.hpp>

#include <cstdint>
#include <limits>

namespace weos
{
namespace chrono
{

// ----=====================================================================----
//     treat_as_floating_point
// ----=====================================================================----

//template <class RepT>
//struct treat_as_floating_point : std::is_floating_point<RepT> {};

// ----=====================================================================----
//     duration_values
// ----=====================================================================----

//! Create special tick values for a duration.
template <typename RepT>
struct duration_values
{
    static BOOST_CONSTEXPR RepT zero()
    {
        return RepT(0);
    }

    static BOOST_CONSTEXPR RepT min()
    {
        return std::numeric_limits<RepT>::lowest();
    }

    static BOOST_CONSTEXPR RepT max()
    {
        return std::numeric_limits<RepT>::max();
    }
};

// ----=====================================================================----
//     duration
// ----=====================================================================----

//! A duration of time.
//! A duration measures an amount of time. It is defined by a number of ticks
//! and a period which is the time in seconds between two ticks.
template <typename RepT, typename PeriodT = boost::ratio<1> >
class duration
{
public:
    //! The type used for representing the number of ticks.
    typedef RepT rep;
    //! The time is seconds between two ticks specified as
    //! <tt>boost::ratio<></tt>.
    typedef PeriodT period;

    //! Creates a duration of zero periods.
    BOOST_CONSTEXPR duration() /*= default*/
        : m_count(duration_values<rep>::zero())
    {
    }

    duration(const duration& other) /*= default*/
        : m_count(other.m_count)
    {
    }

    template <typename Rep2>
    /*BOOST_CONSTEXPR*/ explicit duration(const Rep2& count,
                                      //! \todo Conversion between RepT and Rep2 is missing
                                      typename boost::enable_if_c<false>* = 0)
    {
        m_count = count;
    }

    duration& operator= (const duration& other) /*= default*/
    {
        if (this != &other)
            m_count = other.m_count;
        return *this;
    }

    //! Returns the number of ticks.
    BOOST_CONSTEXPR rep count() const
    {
        return m_count;
    }

    // Arithmetic operators.

    BOOST_CONSTEXPR duration operator+ () const
    {
        return duration(m_count);
    }

    BOOST_CONSTEXPR duration operator- () const
    {
        return duration(-m_count);
    }

    duration& operator++ ()
    {
        ++m_count;
        return *this;
    }

    //! \todo Other operators are missing

    duration operator++ (int)
    {
        return duration(m_count++);
    }

    duration& operator-- ()
    {
        --m_count;
        return *this;
    }

    duration operator-- (int)
    {
        return duration(m_count--);
    }

    //! Adds another duration.
    //! Adds the \p other duration to this duration and returns this duration.
    duration& operator+= (const duration& other)
    {
        m_count += other.m_count;
        return *this;
    }

    //! Subtracts another duration.
    //! Subtracts the \p other duration from this duration and returns this
    //! duration.
    duration& operator-= (const duration& other)
    {
        m_count -= other.m_count;
        return *this;
    }

    // Special values.

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

typedef duration<std::int32_t, boost::micro>          microseconds;
typedef duration<std::int32_t, boost::milli>          milliseconds;
typedef duration<std::int32_t>                        seconds;
typedef duration<std::int32_t, boost::ratio<60> >     minutes;
typedef duration<std::int32_t, boost::ratio<3600> >   hours;

// ----=====================================================================----
//     duration_cast
// ----=====================================================================----

namespace detail
{

// The smallest integer type which is used in duration_cast.
typedef std::int32_t cast_least_int_type;

// A trait to determine if a type is a duration. We need this for duration_cast.
template <typename TypeT>
struct is_duration : boost::false_type
{
};

template <typename RepT, typename PeriodT>
struct is_duration<duration<RepT, PeriodT> > : boost::true_type
{
};

// This struct is a helper for casting durations. Generally, we seek to convert
// from (f * fN / fD) to (t * tN / tD), where f and t are the ticks of a
// duration and (fN / fD) and (tN / tD) are the associated periods.
// The general solution is t = f * (fN / fD) / (tN / tD).
// We can compute the ration R := rN / rD = (fN / fD) / (tN / tD) at compile
// time (after all the duration's period is a compile-time constant). However,
// the standard requires to avoid useless computations e.g. multiplications or
// divisions by 1. Thus, this template is specialized for the following cases:
// 1) R = 1: The two periods are equal and we only need to cast the ticks.
// 2) R = rN / 1 with rN != 1: The from-ticks have to be multiplied with rN.
// 3) R = 1 / rD with rD != 1: The from-ticks have to be divided by rD.
//
// Note: This implementation differs from the standard as it does not perform
// the computations in the widest type available but in the fastest. To be
// more precise, the computation is done in the common type between the
// input, output and the fastest integer type. The outcome is then converted
// to the result type.
template <typename FromDurationT, typename ToDurationT, typename RatioT,
          bool RatioNumeratorEqualsOne, bool RatioDenominatorEqualsOne>
struct duration_cast_helper;

// Special case R = 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, true>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               from.count()));
    }
};

// Special case R = rN / 1, rN != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, true>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                cast_least_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)));
    }
};

// Special case R = 1 / rD, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, false>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                cast_least_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               / static_cast<common_type>(RatioT::den)));
    }
};

// General case R = rN / rD, rN != 1, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, false>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                cast_least_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)
                               / static_cast<common_type>(RatioT::den)));
    }
};

template <typename FromDurationT, typename ToDurationT>
struct duration_caster
{
    typedef typename boost::ratio_divide<
                         typename FromDurationT::period,
                         typename ToDurationT::period>::type ratio;

    typedef duration_cast_helper<FromDurationT, ToDurationT,
                                 ratio,
                                 ratio::num == 1,
                                 ratio::den == 1> helper;

    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        return helper().cast(from);
    }
};

} // namespace detail

//! A utility function to cast durations.
//! Cast from a <tt>duration<RepT, PeriodT></tt> given in \p d to another
//! duration templated by \p ToDuration. The call
//! <tt>duration_cast<T>(d)</tt> is equivalent to
//! <tt>d.count() * d::period / T::period</tt>. If the destination period is
//! coarser than the source period, a truncation occurs if the destination
//! representation is not a floating point type.
//! All values are cast to at least weos::detail::cast_least_int_type before
//! performing the computation.
template <typename ToDurationT, typename RepT, typename PeriodT>
BOOST_CONSTEXPR
typename boost::enable_if<detail::is_duration<ToDurationT>, ToDurationT>::type
duration_cast(const duration<RepT, PeriodT>& d)
{
    return detail::duration_caster<duration<RepT, PeriodT>,
                                   ToDurationT>().cast(d);
}

} // namespace chrono
} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_DURATION_HPP
