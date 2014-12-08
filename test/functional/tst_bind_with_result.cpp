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

volatile int f1_a = 0;
void f1(int a)
{
    f1_a = a;
}

volatile char f2_a = 0.0;
volatile std::uint64_t f2_b = 0;
void f2(char a, std::uint64_t b)
{
    f2_a = a;
    f2_b = b;
}

volatile unsigned f3_a = 0;
volatile char f3_b = 0;
volatile float f3_c = 0.0f;
void f3(unsigned a, char b, float c)
{
    f3_a = a;
    f3_b = b;
    f3_c = c;
}

volatile int* f4_a = 0;
volatile double* f4_b = 0;
volatile int f4_c = 0;
volatile float f4_d = 0.0f;
void f4(int* a, double* b, int c, float d)
{
    f4_a = a;
    f4_b = b;
    f4_c = c;
    f4_d = d;
}

} // anonymous namespace

TEST(bind_with_result, function_pointer_0_args)
{
    for (int counter = 0; counter < 100; ++counter)
    {
        ASSERT_FALSE(f0_flag);
        weos::bind<void>(&f0) ();
        ASSERT_TRUE(f0_flag);
        weos::bind<void>(&f0) ();
        ASSERT_FALSE(f0_flag);
    }
}

TEST(bind_with_result, function_pointer_1_arg)
{
    ASSERT_EQ(0, f1_a);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&f1, counter) ();
        ASSERT_EQ(counter, f1_a);
        weos::bind<void>(&f1, weos::placeholders::_1) (counter + 1);
        ASSERT_EQ(counter + 1, f1_a);
        weos::bind<void>(&f1, weos::placeholders::_2) (0, counter + 2);
        ASSERT_EQ(counter + 2, f1_a);
    }
}

TEST(bind_with_result, function_pointer_2_args)
{
    static const char characters[6] = {'M', 'N', 'O', 'P', 'Q', 'R'};
    ASSERT_EQ(0, f2_a);
    ASSERT_EQ(0, f2_b);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&f2,
                         characters[counter % 6],
                         (std::uint64_t(1) << 60) + counter) ();
        ASSERT_EQ('M' + (counter % 6), f2_a);
        ASSERT_EQ(std::uint64_t(0x1000000000000000) + counter, f2_b);

        weos::bind<void>(&f2,
                         weos::placeholders::_1,
                         (std::uint64_t(1) << 59) + counter) (
                    characters[(counter + 1) % 6]);
        ASSERT_EQ('M' + ((counter + 1) % 6), f2_a);
        ASSERT_EQ(std::uint64_t(0x0800000000000000) + counter, f2_b);

        weos::bind<void>(&f2,
                         weos::placeholders::_2,
                         weos::placeholders::_1) (
                    (std::uint64_t(1) << 58) + counter,
                    characters[(counter + 2) % 6]);
        ASSERT_EQ('M' + ((counter + 2) % 6), f2_a);
        ASSERT_EQ(std::uint64_t(0x0400000000000000) + counter, f2_b);
    }
}

TEST(bind_with_result, function_pointer_3_args)
{
    static const char characters[7] = {'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    ASSERT_EQ(0, f3_a);
    ASSERT_EQ(0, f3_b);
    ASSERT_EQ(0.0f, f3_c);
    for (unsigned counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&f3, counter,
                         characters[counter % 7], 2.7182f * counter) ();
        ASSERT_EQ(counter, f3_a);
        ASSERT_EQ('B' + (counter % 7), f3_b);
        ASSERT_EQ(2.7182f * counter, f3_c);

        weos::bind<void>(&f3, weos::placeholders::_3, weos::placeholders::_1,
                         weos::placeholders::_2) (
                    characters[counter % 2], 1.5f * counter, counter + 1);
        ASSERT_EQ(counter + 1, f3_a);
        ASSERT_EQ(counter % 2 ? 'C' : 'B', f3_b);
        ASSERT_EQ(1.5f * counter, f3_c);

        weos::bind<void>(&f3, counter + 2, weos::placeholders::_1,
                         weos::placeholders::_2) (
                    characters[counter % 3], 0.5f * counter, "dummy");
        ASSERT_EQ(counter + 2, f3_a);
        ASSERT_EQ('B' + counter % 3, f3_b);
        ASSERT_EQ(0.5f * counter, f3_c);
    }
}

TEST(bind_with_result, function_pointer_4_args)
{
    int x[3];
    double y[5];

    ASSERT_TRUE(f4_a == 0);
    ASSERT_TRUE(f4_b == 0);
    ASSERT_EQ(0, f4_c);
    ASSERT_EQ(0.0f, f4_d);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&f4, &x[counter % 3], &y[counter % 5],
                0xBEEFBEEF + counter, -1.0f * counter * counter) ();
        ASSERT_TRUE(f4_a == &x[counter % 3]);
        ASSERT_TRUE(f4_b == &y[counter % 5]);
        ASSERT_EQ(int(0xBEEFBEEF) + counter, f4_c);
        ASSERT_EQ(-1.0f * counter * counter, f4_d);

        weos::bind<void>(&f4, &x[0], weos::placeholders::_2,
                         0xDEADBEEF + counter, 2.0f * counter + counter) (
                    "dummy", &y[counter % 3]);
        ASSERT_TRUE(f4_a == &x[0]);
        ASSERT_TRUE(f4_b == &y[counter % 3]);
        ASSERT_EQ(int(0xDEADBEEF) + counter, f4_c);
        ASSERT_EQ(2.0f * counter + counter, f4_d);

        weos::bind<void>(&f4,
                         weos::placeholders::_2, weos::placeholders::_4,
                         weos::placeholders::_1, weos::placeholders::_3) (
                    0xBEEFDEAD + counter, &x[counter % 2],
                2.0f * counter * counter, &y[counter % 4]);
        ASSERT_TRUE(f4_a == &x[counter % 2]);
        ASSERT_TRUE(f4_b == &y[counter % 4]);
        ASSERT_EQ(int(0xBEEFDEAD) + counter, f4_c);
        ASSERT_EQ(2.0f * counter * counter, f4_d);
    }
}

namespace
{

int difference(int a, int b)
{
    return a - b;
}

char pythagoras(char a, char b)
{
    return a*a + b*b;
}

struct ReturnTest
{
    ReturnTest(char value)
        : m_value(value)
    {
    }

    char m_value;
};

} // anonymous namespace

TEST(bind_with_result, return_from_function_pointer1)
{
    {
        int diff = weos::bind<int>(&difference, 1, 2) ();
        ASSERT_EQ(difference(1, 2), diff);
    }

    {
        int diff = weos::bind<int>(&difference, weos::placeholders::_2,
                                   weos::placeholders::_1) (1, 2);
        ASSERT_EQ(difference(2, 1), diff);
    }

    for (int counter = 0; counter < 100; ++counter)
    {
        int diff = weos::bind<int>(&difference, weos::placeholders::_1, 1) (
                       counter);
        ASSERT_EQ(counter - 1, diff);
    }

    for (int counter = 0; counter < 100; ++counter)
    {
        int diff = weos::bind<int>(&difference, 0, weos::placeholders::_4) (
                       0, 1, 2, counter);
        ASSERT_EQ(-counter, diff);
    }
}

TEST(bind_with_result, return_from_function_pointer2)
{
    {
        char res = weos::bind<char>(&pythagoras, 3, 4) ();
        ASSERT_EQ(25, res);
    }

    {
        char res = weos::bind<char>(&pythagoras, weos::placeholders::_1, 4) (
                       3);
        ASSERT_EQ(25, res);
    }

    {
        char res = weos::bind<char>(&pythagoras, 3, weos::placeholders::_2) (
                       0, 3);
        ASSERT_EQ(18, res);
    }

    {
        int res = weos::bind<int>(&pythagoras, 3, 4) ();
        ASSERT_EQ(25, res);
    }

    {
        int res = weos::bind<int>(&pythagoras, weos::placeholders::_1,
                                  weos::placeholders::_3) (3, 4, 5);
        ASSERT_EQ(34, res);
    }

    {
        ReturnTest res = weos::bind<ReturnTest>(&pythagoras, 3, 4) ();
        ASSERT_EQ(25, res.m_value);
    }
}

// ----=====================================================================----
//     Member function pointers
// ----=====================================================================----

namespace
{

struct MemberFunction0
{
    MemberFunction0()
        : m_flag(false)
    {
    }

    void toggle()
    {
        m_flag = !m_flag;
    }

    void toggleConst() const
    {
        m_flag = !m_flag;
    }

    mutable bool m_flag;
};

struct MemberFunction1
{
    MemberFunction1()
        : m_a(0)
    {
    }

    void set(float* a)
    {
        m_a = a;
    }

    void setConst(float* a) const
    {
        m_a = a;
    }

    mutable float* m_a;
};

struct MemberFunction2
{
    MemberFunction2()
        : m_a(0.0),
          m_b(false)
    {
    }

    void set(float a, bool b)
    {
        m_a = a;
        m_b = b;
    }

    void setConst(float a, bool b) const
    {
        m_a = a;
        m_b = b;
    }

    mutable float m_a;
    mutable bool m_b;
};

struct MemberFunction3
{
    MemberFunction3()
        : m_a(0),
          m_b(0),
          m_c(0)
    {
    }

    void set(short a, long b, void* c)
    {
        m_a = a;
        m_b = b;
        m_c = c;
    }

    void setConst(short a, long b, void* c) const
    {
        m_a = a;
        m_b = b;
        m_c = c;
    }

    mutable short m_a;
    mutable long m_b;
    mutable void* m_c;
};

} // anonymous namespace

TEST(bind_with_result, member_function_0_args)
{
    MemberFunction0 m;
    ASSERT_FALSE(m.m_flag);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction0::toggle, &m) ();
        ASSERT_TRUE(m.m_flag);
        weos::bind<void>(&MemberFunction0::toggle, weos::placeholders::_1) (&m);
        ASSERT_FALSE(m.m_flag);
    }
}

TEST(bind_with_result, const_member_function_0_args)
{
    const MemberFunction0 m;
    ASSERT_FALSE(m.m_flag);
    for (int counter = 0; counter < 1; ++counter)
    {
        weos::bind<void>(&MemberFunction0::toggleConst, &m) ();
        ASSERT_TRUE(m.m_flag);
        weos::bind<void>(&MemberFunction0::toggleConst,
                         weos::placeholders::_1) (&m);
        ASSERT_FALSE(m.m_flag);
    }
}

TEST(bind_with_result, member_function_1_arg)
{
    MemberFunction1 m;
    float values[10];
    ASSERT_TRUE(m.m_a == 0);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction1::set, &m, &values[counter % 10]) ();
        ASSERT_TRUE(m.m_a == &values[counter % 10]);

        weos::bind<void>(&MemberFunction1::set,
                         weos::placeholders::_1, &values[counter % 9]) (&m);
        ASSERT_TRUE(m.m_a == &values[counter % 9]);

        weos::bind<void>(&MemberFunction1::set,
                         weos::placeholders::_2, weos::placeholders::_1) (
                    &values[counter % 2], &m);
        ASSERT_TRUE(m.m_a == &values[counter % 2]);
    }
}

TEST(bind_with_result, const_member_function_1_arg)
{
    const MemberFunction1 m;
    float values[10];
    ASSERT_TRUE(m.m_a == 0);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction1::setConst, &m,
                         &values[counter % 10]) ();
        ASSERT_TRUE(m.m_a == &values[counter % 10]);

        weos::bind<void>(&MemberFunction1::setConst, &m,
                         weos::placeholders::_2) (
                    "dummy", &values[counter % 3]);
        ASSERT_TRUE(m.m_a == &values[counter % 3]);
    }
}

TEST(bind_with_result, member_function_2_args)
{
    MemberFunction2 m;
    ASSERT_EQ(0.0f, m.m_a);
    ASSERT_FALSE(m.m_b);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction2::set, &m,
                         float(counter) / 100, bool(counter % 2)) ();
        ASSERT_EQ(float(counter) / 100, m.m_a);
        ASSERT_TRUE(m.m_b == (counter % 2) ? true : false);

        weos::bind<void>(&MemberFunction2::set, &m,
                         weos::placeholders::_1, bool((counter + 1) % 2)) (
                    counter * 2);
        ASSERT_EQ(counter * 2, m.m_a);
        ASSERT_TRUE(m.m_b == (counter % 2) ? false : true);
    }
}

TEST(bind_with_result, const_member_function_2_args)
{
    const MemberFunction2 m;
    ASSERT_EQ(0.0f, m.m_a);
    ASSERT_FALSE(m.m_b);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction2::setConst, &m,
                         float(counter) / 100, bool(counter % 2)) ();
        ASSERT_EQ(float(counter) / 100, m.m_a);
        ASSERT_TRUE(m.m_b == (counter % 2) ? true : false);

        weos::bind<void>(&MemberFunction2::setConst, &m,
                         float(counter) / 10, weos::placeholders::_1) (
                    !bool(counter % 2));
        ASSERT_EQ(float(counter) / 10, m.m_a);
        ASSERT_TRUE(m.m_b == (counter % 2) ? false : true);
    }
}

TEST(bind_with_result, member_function_3_args)
{
    MemberFunction3 m;
    ASSERT_EQ(0, m.m_a);
    ASSERT_EQ(0, m.m_b);
    ASSERT_EQ(0, m.m_c);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction3::set, &m,
                         counter, -counter,
                         counter % 2 ? static_cast<void*>(&m.m_a)
                                     : static_cast<void*>(&m.m_b)) ();
        ASSERT_EQ(counter, m.m_a);
        ASSERT_EQ(-counter, m.m_b);
        ASSERT_TRUE(m.m_c == (counter % 2 ? static_cast<void*>(&m.m_a)
                                          : static_cast<void*>(&m.m_b)));

        weos::bind<void>(&MemberFunction3::set, weos::placeholders::_1,
                         weos::placeholders::_2, weos::placeholders::_3,
                         weos::placeholders::_4) (
                    &m, counter + 1, -counter - 1, &m.m_c);
        ASSERT_EQ(counter + 1, m.m_a);
        ASSERT_EQ(-counter - 1, m.m_b);
        ASSERT_TRUE(m.m_c == & m.m_c);
    }
}

TEST(bind_with_result, const_member_function_3_args)
{
    const MemberFunction3 m;
    ASSERT_EQ(0, m.m_a);
    ASSERT_EQ(0, m.m_b);
    ASSERT_EQ(0, m.m_c);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::bind<void>(&MemberFunction3::setConst, &m,
                         counter, -counter,
                         counter % 2 ? static_cast<void*>(&m.m_a)
                                     : static_cast<void*>(&m.m_b)) ();
        ASSERT_EQ(counter, m.m_a);
        ASSERT_EQ(-counter, m.m_b);
        ASSERT_TRUE(m.m_c == (counter % 2 ? static_cast<void*>(&m.m_a)
                                          : static_cast<void*>(&m.m_b)));

        weos::bind<void>(&MemberFunction3::setConst,
                         weos::placeholders::_4, weos::placeholders::_3,
                         weos::placeholders::_2, weos::placeholders::_1) (
                    &m.m_c, -counter - 1, counter + 1, &m);
        ASSERT_EQ(counter + 1, m.m_a);
        ASSERT_EQ(-counter - 1, m.m_b);
        ASSERT_TRUE(m.m_c == & m.m_c);
    }
}

namespace
{

class TemplateMember
{
    template <bool TValue>
    struct dispatch_tag
    {
    };

public:
    TemplateMember()
        : m_value(0)
    {
    }

    template <int TValue>
    void set()
    {
        m_value = TValue;
    }

    template <int TValue>
    void setCapped()
    {
        setCapped_impl<TValue>(dispatch_tag<(TValue <= 3)>());
    }

    volatile int m_value;

private:
    template <int TValue>
    void setCapped_impl(dispatch_tag<true>)
    {
        m_value = TValue;
    }

    template <int TValue>
    void setCapped_impl(dispatch_tag<false>)
    {
        m_value = 3;
    }
};

} // anonymous namespace

TEST(bind_with_result, template_member_function)
{
    TemplateMember m;
    ASSERT_EQ(0, m.m_value);

    {
        weos::bind<void>(&TemplateMember::set<1>, &m) ();
    }
    ASSERT_EQ(1, m.m_value);

    {
        weos::bind<void>(&TemplateMember::set<2>, &m) ();
    }
    ASSERT_EQ(2, m.m_value);

    {
        weos::bind<void>(&TemplateMember::set<5>, weos::placeholders::_1) (&m);
    }
    ASSERT_EQ(5, m.m_value);

    {
        weos::bind<void>(&TemplateMember::set<0>, weos::placeholders::_2) (&m, &m);
    }
    ASSERT_EQ(0, m.m_value);

    {
        weos::bind<void>(&TemplateMember::setCapped<1>, &m) ();
    }
    ASSERT_EQ(1, m.m_value);

    {
        weos::bind<void>(&TemplateMember::setCapped<2>, weos::placeholders::_1) (&m);
    }
    ASSERT_EQ(2, m.m_value);

    {
        weos::bind<void>(&TemplateMember::setCapped<5>, weos::placeholders::_3) (
                    "dummy", 0, &m);
    }
    ASSERT_EQ(3, m.m_value);

    {
        weos::bind<void>(&TemplateMember::setCapped<0>, &m) ();
    }
    ASSERT_EQ(0, m.m_value);

    {
        weos::bind<void>(&TemplateMember::setCapped<100>, &m) ();
    }
    ASSERT_EQ(3, m.m_value);
}

namespace
{

struct MemberReturn
{
    MemberReturn(int a, int b)
        : m_a(a),
          m_b(b)
    {
    }

    int sum(int x)
    {
        return m_a + m_b + x;
    }

    int sumConst(int x) const
    {
        return m_a + m_b + x;
    }

    int m_a;
    int m_b;
};

} // anonymous namespace

TEST(bind_with_result, return_from_member_function_pointer)
{
    {
        MemberReturn m(3, 4);
        int sum1 = weos::bind<int>(&MemberReturn::sum, &m, 0) ();
        ASSERT_EQ(7, sum1);

        int ofs = 20;
        int sum2 = weos::bind<int>(&MemberReturn::sum,
                                   weos::placeholders::_1, ofs) (&m);
        ASSERT_EQ(27, sum2);

        ofs = 30;
        int sum3 = weos::bind<int>(&MemberReturn::sum,
                                   &m, weos::placeholders::_1) (ofs);
        ASSERT_EQ(37, sum3);
    }

    {
        MemberReturn m(3, 4);
        int sum1 = weos::bind<int>(&MemberReturn::sumConst, &m, 0) ();
        ASSERT_EQ(7, sum1);

        int ofs = 20;
        int sum2 = weos::bind<int>(&MemberReturn::sumConst,
                                   weos::placeholders::_1, ofs) (&m);
        ASSERT_EQ(27, sum2);

        ofs = 30;
        int sum3 = weos::bind<int>(&MemberReturn::sumConst,
                                   &m, weos::placeholders::_1) (ofs);
        ASSERT_EQ(37, sum3);
    }

    {
        const MemberReturn m(3, 4);
        int sum1 = weos::bind<int>(&MemberReturn::sumConst, &m, 0) ();
        ASSERT_EQ(7, sum1);

        int ofs = 20;
        int sum2 = weos::bind<int>(&MemberReturn::sumConst,
                                   weos::placeholders::_1, ofs) (&m);
        ASSERT_EQ(27, sum2);

        ofs = 30;
        int sum3 = weos::bind<int>(&MemberReturn::sumConst,
                                   &m, weos::placeholders::_1) (ofs);
        ASSERT_EQ(37, sum3);
    }
}

// ----=====================================================================----
//     Member data pointers
// ----=====================================================================----

struct MemberData
{
    MemberData()
        : data(0)
    {
    }

    int data;
};

TEST(bind_with_result, member_data_pointer)
{
    int x;
    MemberData m;
    ASSERT_TRUE(m.data == 0);

    x = weos::bind<int>(&MemberData::data, &m) ();
    ASSERT_EQ(0, x);

    weos::bind<int&>(&MemberData::data, &m) () = 22;
    ASSERT_EQ(22, m.data);

    x = weos::bind<int>(&MemberData::data, &m) ();
    ASSERT_EQ(22, x);
}
