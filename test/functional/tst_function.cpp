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

#include <functional.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

// ----=====================================================================----
//     Function pointers
// ----=====================================================================----

namespace
{
volatile bool f0_flag = false;
void f0()
{
    f0_flag = !f0_flag;
}

int sum4(int a0, int a1, int a2, int a3)
{
    return a0 + a1 + a2 + a3;
}

} // anonymous namespace

TEST(function, function_pointer_0_args)
{
    for (int counter = 0; counter < 100; ++counter)
    {
        ASSERT_FALSE(f0_flag);
        weos::function<void()> f = weos::bind<void>(&f0);
        f();
        ASSERT_TRUE(f0_flag);
        weos::function<void()> g;
        g = f;
        g();
        ASSERT_FALSE(f0_flag);
    }
}

TEST(function, sum)
{
    {
        int sum;
        weos::function<int()> f = weos::bind<int>(&sum4, 1, 2, 3, 4);
        sum = f();
        ASSERT_EQ(10, sum);
        weos::function<int()> g(f);
        sum = g();
        ASSERT_EQ(10, sum);
        weos::function<int()> h;
        h = f;
        sum = h();
        ASSERT_EQ(10, sum);
    }
    {
        int sum;
        weos::function<int(int)> f = weos::bind<int>(&sum4, weos::placeholders::_1, 2, 3, 4);
        sum = f(1);
        ASSERT_EQ(10, sum);
        weos::function<int(int)> g(f);
        sum = g(2);
        ASSERT_EQ(11, sum);
        weos::function<int(int)> h;
        h = f;
        sum = h(3);
        ASSERT_EQ(12, sum);
    }
}
