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

#include <tuple.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

using namespace weos;

struct A {};

TEST(tuple, default_construction)
{
    tuple<> t1;
    (void)t1;

    tuple<int> t2;
    (void)t2;

    tuple<int[2], void*> t3;
    (void)t3;

    tuple<A, A, A> t4;
    (void)t4;
}

TEST(tuple, nothrow_default_construction)
{
    static_assert(is_nothrow_default_constructible<tuple<>>::value, "");
    static_assert(is_nothrow_default_constructible<tuple<int>>::value, "");
    static_assert(is_nothrow_default_constructible<tuple<int[2], void*>>::value, "");
    static_assert(is_nothrow_default_constructible<tuple<A, A, A>>::value, "");
}

TEST(tuple, construction)
{
    tuple<int> t1(10);
    (void)t1;

    int i = 10;
    float f = 2106.f;
    tuple<int&, float&> t2(i, f);
    (void)t2;

    tuple<A, A, A> t3(A(), A(), A());
    (void)t3;
}

TEST(tuple, access)
{
    tuple<int> t1(10);
    ASSERT_EQ(10, get<0>(t1));
    get<0>(t1) = 20;
    ASSERT_EQ(20, get<0>(t1));

    int i = 10;
    float f = 2106.f;
    tuple<int&, float&> t2(i, f);
    ASSERT_EQ(10, get<0>(t2));
    ASSERT_EQ(2106.f, get<1>(t2));
    get<0>(t2) = 42;
    get<1>(t2) = -1;
    ASSERT_EQ(42, i);
    ASSERT_EQ(-1.f, f);
}
