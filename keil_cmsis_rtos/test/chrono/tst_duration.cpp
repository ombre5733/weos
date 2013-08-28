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

#include "../../duration.hpp"

#include "gtest/gtest.h"

template <typename T>
class PredefinedDurations : public ::testing::Test
{
};

typedef ::testing::Types<weos::chrono::microseconds,
                         weos::chrono::milliseconds,
                         weos::chrono::seconds,
                         weos::chrono::minutes> duration_types;
TYPED_TEST_CASE(PredefinedDurations, duration_types);

TYPED_TEST(PredefinedDurations, DefaultConstructor)
{
    TypeParam d;
    ASSERT_EQ(0, d.count());
}

TYPED_TEST(PredefinedDurations, ConstructorWithArgument)
{
    TypeParam d(42);
    ASSERT_EQ(42, d.count());
}

TYPED_TEST(PredefinedDurations, CopyConstructor)
{
    TypeParam d1(42);
    TypeParam d2(d1);
    ASSERT_EQ(42, d2.count());
}

TYPED_TEST(PredefinedDurations, Zero)
{
    TypeParam d = TypeParam::zero();
    ASSERT_EQ(0, d.count());
}

TYPED_TEST(PredefinedDurations, Min)
{
    TypeParam d = TypeParam::min();
    ASSERT_EQ(-2147483648, d.count());
}

TYPED_TEST(PredefinedDurations, Max)
{
    TypeParam d = TypeParam::max();
    ASSERT_EQ(2147483647, d.count());
}

TYPED_TEST(PredefinedDurations, Arithmetics)
{
    TypeParam d1(21);
    TypeParam d2(-42);
    ASSERT_EQ( 21, (+d1).count());
    ASSERT_EQ(-21, (-d1).count());
    ASSERT_EQ(-42, (+d2).count());
    ASSERT_EQ( 42, (-d2).count());

    ASSERT_EQ( 21, (d1++).count());
    ASSERT_EQ( 22, d1.count());
    ASSERT_EQ( 23, (++d1).count());
    ASSERT_EQ( 23, d1.count());

    ASSERT_EQ( 23, (d1--).count());
    ASSERT_EQ( 22, d1.count());
    ASSERT_EQ( 21, (--d1).count());
    ASSERT_EQ( 21, d1.count());

    ASSERT_EQ(-42, (d2++).count());
    ASSERT_EQ(-41, d2.count());
    ASSERT_EQ(-40, (++d2).count());
    ASSERT_EQ(-40, d2.count());

    ASSERT_EQ(-40, (d2--).count());
    ASSERT_EQ(-41, d2.count());
    ASSERT_EQ(-42, (--d2).count());
    ASSERT_EQ(-42, d2.count());

    d1 += d2;
    ASSERT_EQ(-21, d1.count());
    d1 -= d2;
    ASSERT_EQ( 21, d1.count());
}

TEST(duration_cast, s_to_ms)
{
    weos::chrono::seconds s(23);
    weos::chrono::milliseconds ms = weos::chrono::duration_cast<weos::chrono::milliseconds>(s);
    ASSERT_EQ(23000, ms.count());
}

TEST(duration_cast, s_to_us)
{
    weos::chrono::seconds s(23);
    weos::chrono::microseconds us = weos::chrono::duration_cast<weos::chrono::microseconds>(s);
    ASSERT_EQ(23000000, us.count());
}

TEST(duration_cast, min_to_s)
{
    weos::chrono::minutes min(23);
    weos::chrono::seconds s = weos::chrono::duration_cast<weos::chrono::seconds>(min);
    ASSERT_EQ(23*60, s.count());
}

TEST(duration_cast, min_to_ms)
{
    weos::chrono::minutes min(23);
    weos::chrono::milliseconds ms = weos::chrono::duration_cast<weos::chrono::milliseconds>(min);
    ASSERT_EQ(23*60000, ms.count());
}

TEST(duration_cast, min_to_us)
{
    weos::chrono::minutes min(23);
    weos::chrono::microseconds us = weos::chrono::duration_cast<weos::chrono::microseconds>(min);
    ASSERT_EQ(23*60000000, us.count());
}

#if 0
double slept_s = 0;
double slept_ms = 0;
double slept_us = 0;

template <typename RepT>
void sleep(weos::chrono::duration<RepT, boost::ratio<1> >& d)
{
    slept_s = d.count();
}

template <typename RepT>
void sleep(weos::chrono::duration<RepT, boost::milli>& d)
{
    slept_ms = d.count();
}

template <typename RepT>
void sleep(weos::chrono::duration<RepT, boost::micro>& d)
{
    slept_us = d.count();
}

TEST(duration, OverloadResolution)
{
    sleep(weos::chrono::seconds(11));
    ASSERT_EQ(11, slept_s);
    slept_s = 0;
    sleep(weos::chrono::duration<uint8_t, boost::ratio<1> >(23));
    ASSERT_EQ(23, slept_s);
    slept_s = 0;
    sleep(weos::chrono::duration<double, boost::ratio<1> >(3.14));
    ASSERT_EQ(3.14, slept_s);
    slept_s = 0;

    sleep(weos::chrono::milliseconds(11));
    ASSERT_EQ(11, slept_ms);
    slept_ms = 0;
    sleep(weos::chrono::duration<uint8_t, boost::milli>(23));
    ASSERT_EQ(23, slept_ms);
    slept_ms = 0;
    sleep(weos::chrono::duration<double, boost::milli>(3.14));
    ASSERT_EQ(3.14, slept_ms);
    slept_ms = 0;

    sleep(weos::chrono::microseconds(11));
    ASSERT_EQ(11, slept_us);
    slept_us = 0;
    sleep(weos::chrono::duration<uint8_t, boost::micro>(23));
    ASSERT_EQ(23, slept_us);
    slept_us = 0;
    sleep(weos::chrono::duration<double, boost::micro>(3.14));
    ASSERT_EQ(3.14, slept_us);
    slept_us = 0;
}
#endif
