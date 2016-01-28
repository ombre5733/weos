/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#include <atomic.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

TEST(atomic_flag, default_construction)
{
    weos::atomic_flag flag;
    (void)flag;
}

TEST(atomic_flag, initialization)
{
    weos::atomic_flag flag = ATOMIC_FLAG_INIT;
    (void)flag;
}

TEST(atomic_flag, test_and_set)
{
    weos::atomic_flag flag = ATOMIC_FLAG_INIT;
    ASSERT_FALSE(flag.test_and_set());
    ASSERT_TRUE(flag.test_and_set());
    flag.clear();
    ASSERT_FALSE(flag.test_and_set());
    ASSERT_TRUE(flag.test_and_set());
}



TEST(atomic_int, default_construction)
{
    weos::atomic_int x;
    (void)x;
}

TEST(atomic_int, construct_with_value)
{
    int value;
    weos::atomic_int x(68);

    value = x;
    ASSERT_EQ(68, value);
}

TEST(atomic_int, load_and_store)
{
    int value;
    weos::atomic_int x(68);

    value = x.load();
    ASSERT_EQ(68, value);
    x.store(21);
    value = x.load();
    ASSERT_EQ(21, value);
    value = x;
    ASSERT_EQ(21, value);
}

TEST(atomic_int, exchange)
{
    int value;
    weos::atomic_int x(68);

    value = x.load();
    ASSERT_EQ(68, value);
    value = x.exchange(21);
    ASSERT_EQ(68, value);
    value = x;
    ASSERT_EQ(21, value);
}

TEST(atomic_int, fetch_X)
{
    int value;
    weos::atomic_int x(68);

    value = x.fetch_add(3);
    ASSERT_EQ(68, value);
    value = x.fetch_sub(5);
    ASSERT_EQ(71, value);
    value = x.fetch_and(64);
    ASSERT_EQ(66, value);
    value = x.fetch_or(17);
    ASSERT_EQ(64, value);
    value = x.fetch_xor(66);
    ASSERT_EQ(81, value);
    value = x;
    ASSERT_EQ(19, value);
}

TEST(atomic_int, compare_exchange_strong)
{
    int expected = 66;
    bool exchanged;
    weos::atomic_int x(68);

    exchanged = x.compare_exchange_strong(expected, 77);
    ASSERT_FALSE(exchanged);
    ASSERT_EQ(68, expected);
    ASSERT_EQ(68, int(x));

    exchanged = x.compare_exchange_strong(expected, 77);
    ASSERT_TRUE(exchanged);
    ASSERT_EQ(68, expected);
    ASSERT_EQ(77, int(x));
}

TEST(atomic_int, operators)
{
    weos::atomic_int x(68);

    ASSERT_EQ(68, int(x));
    x = 77;
    ASSERT_EQ(77, int(x));
}
