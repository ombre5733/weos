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

#include "../../chrono.hpp"

#include "gtest/gtest.h"

struct DummyClock
{
};
typedef weos::chrono::time_point<DummyClock, weos::chrono::seconds>
    second_time_point;

TEST(second_time_point, DefaultConstructor)
{
    second_time_point t;
    ASSERT_EQ(0, t.time_since_epoch().count());
}

TEST(second_time_point, Constructor)
{
    second_time_point t1(weos::chrono::seconds(21));
    ASSERT_EQ(21, t1.time_since_epoch().count());

    second_time_point t2(weos::chrono::seconds(-42));
    ASSERT_EQ(-42, t2.time_since_epoch().count());
}

TEST(second_time_point, Arithmetics)
{
    second_time_point t1(weos::chrono::seconds(21));
    /* second_time_point t2(weos::chrono::seconds(-42)); */

    t1 += weos::chrono::seconds(42);
    ASSERT_EQ(63, t1.time_since_epoch().count());
    t1 -= weos::chrono::seconds(100);
    ASSERT_EQ(-37, t1.time_since_epoch().count());
}
