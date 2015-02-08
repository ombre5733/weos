/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2015, Manuel Freiberger
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

#ifndef WEOS_COMMON_CHRONO_HPP
#define WEOS_COMMON_CHRONO_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../ratio.hpp"
#include "../type_traits.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace chrono
{

template <typename RepT, typename PeriodT = ratio<1> >
class duration;


namespace detail
{

template <typename FromDurationT, typename ToDurationT>
struct duration_caster;

// -----------------------------------------------------------------------------
// is_duration
// -----------------------------------------------------------------------------

// A trait to determine if a type is a duration. We need this for duration_cast.
template <typename TypeT>
struct is_duration : public false_type {};

template <typename RepT, typename PeriodT>
struct is_duration<duration<RepT, PeriodT> > : public true_type {};

template <typename RepT, typename PeriodT>
struct is_duration<const duration<RepT, PeriodT> > : public true_type {};

template <typename RepT, typename PeriodT>
struct is_duration<volatile duration<RepT, PeriodT> > : public true_type {};

template <typename RepT, typename PeriodT>
struct is_duration<const volatile duration<RepT, PeriodT> > : public true_type {};

// -----------------------------------------------------------------------------
// Compile-time Greatest Common Divisor & Least Common Multiple
// -----------------------------------------------------------------------------

// EUCLID(a, b)
//   if b = 0
//     then return a
//   else return EUCLID(b, a mod b)
template <intmax_t A, intmax_t B>
struct static_gcd_impl
{
    static const intmax_t value = static_gcd_impl<B, A % B>::value;
};

template <intmax_t A>
struct static_gcd_impl<A, 0>
{
    static const intmax_t value = A;
};

template <>
struct static_gcd_impl<0, 0>
{
    static const intmax_t value = 0;
};

template <intmax_t A, intmax_t B>
struct static_gcd : public integral_constant<intmax_t, static_gcd_impl<A, B>::value>
{
};


template <intmax_t A, intmax_t B>
struct static_lcm_impl
{
    static const intmax_t value = A / static_gcd_impl<A, B>::value * B;
};

template <>
struct static_lcm_impl<0, 0>
{
    static const intmax_t value = 0;
};

template <intmax_t A, intmax_t B>
struct static_lcm : public integral_constant<intmax_t, static_lcm_impl<A, B>::value>
{
};

// -----------------------------------------------------------------------------
// ratio_gcd
// -----------------------------------------------------------------------------

template <typename R1, typename R2>
struct ratio_gcd : ratio<static_gcd<R1::num, R2::num>::value,
                         static_lcm<R1::den, R2::den>::value>::type
{
};

// -----------------------------------------------------------------------------
// checked_division
// -----------------------------------------------------------------------------

template <typename R1, typename R2>
class checked_division
{
    // Divide the numerators by there GCD.
    static const intmax_t gcd_num = static_gcd<R1::num, R2::num>::value;
    static const intmax_t num1 = R1::num / gcd_num;
    static const intmax_t num2 = R2::num / gcd_num;
    // Do the same with the denominators.
    static const intmax_t gcd_den = static_gcd<R1::den, R2::den>::value;
    static const intmax_t den1 = R1::den / gcd_den;
    static const intmax_t den2 = R2::den / gcd_den;

    // Assume sizeof(intmax_t) == 2 and CHAR_BIT == 8:
    // We want max = 0x7fff.
    // Set the highest bit and add 1:
    //   1 << (16 - 1) + 1 = 0x4000 + 1 = 0x4001
    // Negation by two's complement is bit-wise negation plus 1:
    //   -0x4001 = ~0x4001 + 1 = 0x7ffe + 1 = 0x7fff
    // This is what we want.
    static const intmax_t max = -((intmax_t(1) << (sizeof(intmax_t) * 8 - 1)) + 1);

public:
    static const bool overflow = (num1 > max / den2) || (num2 > max / den1);

    // (n1 / d1) / (n2 / d2) = (n1 * d2) / (d1 * n2)
    typedef ratio<num1 * den2, den1 * num2> type;
};

} // namespace detail
} // namespace chrono

// ----=====================================================================----
//     Specialization of common_type for chrono::duration<>
// ----=====================================================================----

template <typename Rep1T, typename Period1T, typename Rep2T, typename Period2T>
struct common_type<chrono::duration<Rep1T, Period1T>,
                   chrono::duration<Rep2T, Period2T> >
{
    typedef chrono::duration<typename common_type<Rep1T, Rep2T>::type,
                             typename chrono::detail::ratio_gcd<Period1T, Period2T>::type> type;
};

namespace chrono
{

// ----=====================================================================----
//     treat_as_floating_point
// ----=====================================================================----

template <typename RepT>
struct treat_as_floating_point : public is_floating_point<RepT> {};

// ----=====================================================================----
//     duration_values
// ----=====================================================================----

//! Create special tick values for a duration.
template <typename RepT>
struct duration_values
{
    static constexpr RepT zero()
    {
        return RepT(0);
    }

    static constexpr RepT min()
    {
        return std::numeric_limits<RepT>::lowest();
    }

    static constexpr RepT max()
    {
        return std::numeric_limits<RepT>::max();
    }
};

// ----=====================================================================----
//     duration
// ----=====================================================================----

template <typename ToDurationT, typename RepT, typename PeriodT>
inline constexpr
typename enable_if<detail::is_duration<ToDurationT>::value,
                   ToDurationT>::type
duration_cast(const duration<RepT, PeriodT>& d);

//! A duration of time.
//! A duration measures an amount of time. It is defined by a number of ticks
//! and a period which is the time in seconds between two ticks.
template <typename RepT, typename PeriodT>
class duration
{
    static_assert(!detail::is_duration<RepT>::value,
                  "The tick representation cannot be a duration");
    static_assert(PeriodT::num > 0, "The period must be positive");

public:
    //! The type used for representing the number of ticks.
    typedef RepT rep;
    //! The time between two ticks specified as a ratio of seconds.
    typedef PeriodT period;

    //! Creates a duration of zero periods.
    constexpr duration() /*= default*/
    {
    }

    //! Creates a duration from another one.
    //! Creates a duration which copies the number of periods from the
    //! \p other duration.
    duration(const duration& other) /*= default*/
        : m_count(other.m_count)
    {
    }

    //! Constructs a duration with \p count ticks.
    //!
    //! \note The constructor is disabled, if it is tried to create an
    //! integer tick from a floating-point value. However, it is possible
    //! to create a floating-point tick from an integer value. In other words,
    //! if the conversion might truncate, the constructor is disabled.
    template <typename TRep2,
              typename _ = typename enable_if<
                               is_convertible<TRep2, rep>::value
                               && (treat_as_floating_point<rep>::value ||
                                   !treat_as_floating_point<TRep2>::value)
                           >::type>
    constexpr
    explicit duration(const TRep2& count)
        : m_count(count)
    {
    }

    //! Constructs a duration from the \p other duration.
    template <typename TRep2, typename TPeriod2,
              typename _ = typename enable_if<!detail::checked_division<TPeriod2, period>::overflow
                                              && (treat_as_floating_point<rep>::value
                                                  || (detail::checked_division<TPeriod2, period>::type::den == 1
                                                      && !treat_as_floating_point<TRep2>::value))>::type>
    constexpr
    duration(const duration<TRep2, TPeriod2>& other)
        : m_count(duration_cast<duration>(other).count())
    {
    }

    duration& operator= (const duration& other) /*= default*/
    {
        m_count = other.m_count;
        return *this;
    }

    //! Returns the number of ticks.
    constexpr rep count() const
    {
        return m_count;
    }

    // Arithmetic operators.

    constexpr duration operator+ () const
    {
        return *this;
    }

    constexpr duration operator- () const
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

    //! Multiplies this duration by a scalar \p a.
    duration& operator*=(const rep& a)
    {
        m_count *= a;
        return *this;
    }

    //! Divides this duration by a scalar \p a.
    duration& operator/=(const rep& a)
    {
        m_count /= a;
        return *this;
    }

    //! Performs a modulo division with the scalar \p a.
    duration& operator%=(const rep& a)
    {
        m_count %= a;
        return *this;
    }

    //! Performs a modulo division with the \p other duration.
    duration& operator%=(const duration& other)
    {
        m_count %= other.count();
        return *this;
    }

    // Special values.

    static constexpr duration zero()
    {
        return duration(duration_values<rep>::zero());
    }

    static constexpr duration min()
    {
        return duration(duration_values<rep>::min());
    }

    static constexpr duration max()
    {
        return duration(duration_values<rep>::max());
    }

private:
    //! The number of periods of which this duration consists.
    rep m_count;
};

// ----=====================================================================----
//     SI-constants
// ----=====================================================================----

typedef duration<std::int64_t, nano>           nanoseconds;
typedef duration<std::int64_t, micro>          microseconds;
typedef duration<std::int64_t, milli>          milliseconds;
typedef duration<std::int64_t>                 seconds;
typedef duration<std::int32_t, ratio<60> >     minutes;
typedef duration<std::int32_t, ratio<3600> >   hours;

// ----=====================================================================----
//     duration comparisons
// ----=====================================================================----

namespace detail
{

// Compares two durations. General case.
template <typename TDuration1, typename TDuration2>
struct duration_equal
{
    static inline constexpr
    bool cmp(const TDuration1& d1, const TDuration2& d2)
    {
        typedef typename common_type<TDuration1, TDuration2>::type common_type;
        return common_type(d1).count() == common_type(d2).count();
    }
};

// Compares two durations. No cast needed when both durations are of the
// same type.
template <typename TDuration>
struct duration_equal<TDuration, TDuration>
{
    static inline constexpr
    bool cmp(const TDuration& d1, const TDuration& d2)
    {
        return d1.count() == d2.count();
    }
};

// Compares two durations. General case.
template <typename TDuration1, typename TDuration2>
struct duration_less
{
    static inline constexpr
    bool cmp(const TDuration1& d1, const TDuration2& d2)
    {
        typedef typename common_type<TDuration1, TDuration2>::type common_type;
        return common_type(d1).count() < common_type(d2).count();
    }
};

// Compares two durations. No cast needed when both durations are of the
// same type.
template <typename TDuration>
struct duration_less<TDuration, TDuration>
{
    static inline constexpr
    bool cmp(const TDuration& d1, const TDuration& d2)
    {
        return d1.count() < d2.count();
    }
};

} // namespace detail

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator==(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return detail::duration_equal<duration<TRep1, TPeriod1>,
                                  duration<TRep2, TPeriod2> >::cmp(d1, d2);
}

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator!=(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return !detail::duration_equal<duration<TRep1, TPeriod1>,
                                   duration<TRep2, TPeriod2> >::cmp(d1, d2);
}

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator<(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return detail::duration_less<duration<TRep1, TPeriod1>,
                                 duration<TRep2, TPeriod2> >::cmp(d1, d2);
}

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator>(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return detail::duration_less<duration<TRep1, TPeriod1>,
                                 duration<TRep2, TPeriod2> >::cmp(d2, d1);
}

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator<=(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return !detail::duration_less<duration<TRep1, TPeriod1>,
                                  duration<TRep2, TPeriod2> >::cmp(d2, d1);
}

template <typename TRep1, typename TPeriod1, typename TRep2, typename TPeriod2>
inline constexpr
bool operator>=(const duration<TRep1, TPeriod1>& d1, const duration<TRep2, TPeriod2>& d2)
{
    return !detail::duration_less<duration<TRep1, TPeriod1>,
                                  duration<TRep2, TPeriod2> >::cmp(d1, d2);
}

// ----=====================================================================----
//     duration arithmetic
// ----=====================================================================----

template <typename Rep1T, typename Period1T, typename Rep2T, typename Period2T>
inline constexpr
typename common_type<duration<Rep1T, Period1T>,
                     duration<Rep2T, Period2T> >::type
operator+ (const duration<Rep1T, Period1T>& x,
           const duration<Rep2T, Period2T>& y)
{
    typedef typename common_type<
            duration<Rep1T, Period1T>,
            duration<Rep2T, Period2T> >::type common_type;
    return common_type(common_type(x).count() + common_type(y).count());
}

template <typename Rep1T, typename Period1T, typename Rep2T, typename Period2T>
inline constexpr
typename common_type<duration<Rep1T, Period1T>,
                     duration<Rep2T, Period2T> >::type
operator- (const duration<Rep1T, Period1T>& x,
           const duration<Rep2T, Period2T>& y)
{
    typedef typename common_type<
            duration<Rep1T, Period1T>,
            duration<Rep2T, Period2T> >::type common_type;
    return common_type(common_type(x).count() - common_type(y).count());
}

// TODO: operator*
// TODO: operator/
// TODO: operator%

// ----=====================================================================----
//     duration_cast
// ----=====================================================================----

namespace detail
{

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
    constexpr ToDurationT cast(const FromDurationT& from) const
    {
        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               from.count()));
    }
};

// Special case R = rN / 1, rN != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, true>
{
    constexpr ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename common_type<
                    typename FromDurationT::rep,
                    typename ToDurationT::rep,
                    intmax_t>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)));
    }
};

// Special case R = 1 / rD, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, false>
{
    constexpr ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename common_type<
                    typename FromDurationT::rep,
                    typename ToDurationT::rep,
                    intmax_t>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               / static_cast<common_type>(RatioT::den)));
    }
};

// General case R = rN / rD, rN != 1, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, false>
{
    constexpr ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename common_type<
                    typename FromDurationT::rep,
                    typename ToDurationT::rep,
                    intmax_t>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)
                               / static_cast<common_type>(RatioT::den)));
    }
};

template <typename FromDurationT, typename ToDurationT>
struct duration_caster
{
    typedef typename ratio_divide<
                         typename FromDurationT::period,
                         typename ToDurationT::period>::type ratio;

    typedef duration_cast_helper<FromDurationT, ToDurationT,
                                 ratio,
                                 ratio::num == 1,
                                 ratio::den == 1> helper;

    constexpr ToDurationT cast(const FromDurationT& from) const
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
//! All values are cast to at least intmax_t before performing the computation.
template <typename ToDurationT, typename RepT, typename PeriodT>
inline constexpr
typename enable_if<detail::is_duration<ToDurationT>::value,
                   ToDurationT>::type
duration_cast(const duration<RepT, PeriodT>& d)
{
    return detail::duration_caster<duration<RepT, PeriodT>,
                                   ToDurationT>().cast(d);
}

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

    constexpr time_point()
        : m_duration(duration::zero())
    {
    }

    //! Creates a time point from a duration.
    //! Creates a time point whose difference to the epoch time is equal
    //! to the given duration \p d.
    constexpr explicit time_point(const duration& d)
        : m_duration(d)
    {
    }

    //! Creates a time point from another time point.
    //! Creates a time point from the other time point \p tp.
    template <typename TDuration2,
              typename _ = typename enable_if<
                               is_convertible<TDuration2, duration>::value>::type>
    constexpr
    time_point(const time_point<clock, TDuration2>& tp)
        : m_duration(tp.time_since_epoch())
    {
    }

    //! Returns the time point relative to the clock's epoch.
    //! Returns the time point as a duration since the clock's epoch.
    constexpr duration time_since_epoch() const
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

    static constexpr time_point max()
    {
        return time_point(duration::max());
    }

    static constexpr time_point min()
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
inline constexpr
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
inline constexpr
bool
operator==(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() == y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline constexpr
bool
operator!=(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() != y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline constexpr
bool
operator<(const time_point<TClock, TDuration1>& x,
          const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() < y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline constexpr
bool
operator>(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() > y.time_since_epoch();
}


template <typename TClock, typename TDuration1, typename TDuration2>
inline constexpr
bool
operator<=(const time_point<TClock, TDuration1>& x,
           const time_point<TClock, TDuration2>& y)
{
    return x.time_since_epoch() <= y.time_since_epoch();
}

template <typename TClock, typename TDuration1, typename TDuration2>
inline constexpr
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
inline constexpr
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
inline constexpr
time_point<TClock,
           typename common_type<duration<TRep1, TPeriod1>, TDuration2>::type>
operator+(const duration<TRep1, TPeriod1>& d,
          const time_point<TClock, TDuration2>& tp)
{
    return tp + d;
}

template <typename TClock, typename TDuration1, typename TRep2, typename TPeriod2>
inline constexpr
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
inline constexpr
typename common_type<Duration1T, Duration2T>::type
operator- (const time_point<ClockT, Duration1T>& x,
           const time_point<ClockT, Duration2T>& y)
{
    return x.time_since_epoch() - y.time_since_epoch();
}

} // namespace chrono

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_CHRONO_HPP
