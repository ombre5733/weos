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

#include <thread.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

TEST(thread, DefaultConstructor)
{
    weos::thread t;
    ASSERT_FALSE(t.joinable());
}

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

} // anonymouse namespace

TEST(thread, f0)
{
    for (int counter = 0; counter < 100; ++counter)
    {
        ASSERT_FALSE(f0_flag);
        {
            weos::thread t(&f0);
            ASSERT_TRUE(t.joinable());
            t.join();
            ASSERT_FALSE(t.joinable());
        }
        ASSERT_TRUE(f0_flag);
        {
            weos::thread t(&f0);
            ASSERT_TRUE(t.joinable());
            t.join();
            ASSERT_FALSE(t.joinable());
        }
        ASSERT_FALSE(f0_flag);
    }
}

TEST(thread, f1)
{
    ASSERT_EQ(0, f1_a);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::thread t(&f1, counter);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
        ASSERT_EQ(counter, f1_a);
    }
}

TEST(thread, f2)
{
    static const char characters[6] = {'M', 'N', 'O', 'P', 'Q', 'R'};
    ASSERT_EQ(0, f2_a);
    ASSERT_EQ(0, f2_b);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::thread t(&f2, characters[counter % 6],
                       (std::uint64_t(1) << 60) + counter);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
        ASSERT_EQ('M' + (counter %6), f2_a);
        ASSERT_EQ(std::uint64_t(0x1000000000000000) + counter, f2_b);
    }
}

TEST(thread, f3)
{
    static const char characters[7] = {'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    ASSERT_EQ(0, f3_a);
    ASSERT_EQ(0, f3_b);
    ASSERT_EQ(0.0f, f3_c);
    for (unsigned counter = 0; counter < 100; ++counter)
    {
        weos::thread t(&f3, counter, characters[counter % 7],
                       2.7182f * counter);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
        ASSERT_EQ(counter, f3_a);
        ASSERT_EQ('B' + (counter % 7), f3_b);
        ASSERT_EQ(2.7182f * counter, f3_c);
    }
}

TEST(thread, f4)
{
    int x[3];
    double y[5];

    ASSERT_TRUE(f4_a == 0);
    ASSERT_TRUE(f4_b == 0);
    ASSERT_EQ(0, f4_c);
    ASSERT_EQ(0.0f, f4_d);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::thread t(&f4, &x[counter % 3], &y[counter % 5],
                       0xBEEFBEEF + counter, -1.0f * counter * counter);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
        ASSERT_TRUE(f4_a == &x[counter % 3]);
        ASSERT_TRUE(f4_b == &y[counter % 5]);
        ASSERT_EQ(int(0xBEEFBEEF) + counter, f4_c);
        ASSERT_EQ(-1.0f * counter * counter, f4_d);
    }
}

namespace
{

struct MemberFunction0
{
    MemberFunction0()
        : flag(false)
    {
    }

    void toggle()
    {
        flag = !flag;
    }

    bool flag;
};

} // anonymous namespace

TEST(thread, member_function_0)
{
    MemberFunction0 m;
    ASSERT_FALSE(m.flag);
    for (int counter = 0; counter < 100; ++counter)
    {
        weos::thread t(&MemberFunction0::toggle, &m);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
        ASSERT_TRUE(m.flag == (counter % 2) ? false : true);
    }
}

namespace
{
//! An empty thread which does nothing.
void empty_thread()
{
}

//! A thread which sleeps for \p ms milliseconds and returns afterwards.
void delay_thread(std::uint32_t ms)
{
    weos::this_thread::sleep_for(weos::chrono::milliseconds(ms));
}

} // anonymous namespace

TEST(thread, start_one_thread_very_often)
{
    for (unsigned i = 0; i < 10000; ++i)
    {
        weos::thread t(empty_thread);
        ASSERT_TRUE(t.joinable());
        t.join();
        ASSERT_FALSE(t.joinable());
    }
}

TEST(thread, start_all_in_parallel)
{
    weos::thread* threads[WEOS_MAX_NUM_CONCURRENT_THREADS];
    for (unsigned i = 0; i < WEOS_MAX_NUM_CONCURRENT_THREADS; ++i)
    {
        threads[i] = new weos::thread(delay_thread, 5);
        ASSERT_TRUE(threads[i]->joinable());
    }
    for (unsigned i = 0; i < WEOS_MAX_NUM_CONCURRENT_THREADS; ++i)
    {
        threads[i]->join();
        ASSERT_FALSE(threads[i]->joinable());
        delete threads[i];
    }
}

TEST(thread, create_and_destroy_randomly)
{
    weos::thread* threads[WEOS_MAX_NUM_CONCURRENT_THREADS] = {0};

    for (unsigned i = 0; i < 1000; ++i)
    {
        int delayTime = 1 + random() % 3;
        int index = random() % WEOS_MAX_NUM_CONCURRENT_THREADS;
        if (threads[index] == 0)
        {
            threads[index] = new weos::thread(delay_thread, delayTime);
            ASSERT_TRUE(threads[index]->joinable());
        }
        else
        {
            ASSERT_TRUE(threads[index]->joinable());
            threads[index]->join();
            delete threads[index];
            threads[index] = 0;
        }
    }

    for (unsigned i = 0; i < WEOS_MAX_NUM_CONCURRENT_THREADS; ++i)
    {
        if (threads[i])
        {
            ASSERT_TRUE(threads[i]->joinable());
            threads[i]->join();
            delete threads[i];
        }
    }
}

#if 0
TEST(thread, sleep_for)
{
    std::uint32_t delays[] = { 0,   1,   2,   3,   4,   5,
                                   10,  20,  30,  40,  50,
                                  100, 200, 300, 400, 500};
    for (unsigned i = 0; i < sizeof(delays) / sizeof(delays[0]); ++i)
    {
        std::uint32_t start = osKernelSysTick();
        weos::this_thread::sleep_for(weos::chrono::milliseconds(delays[i]));
        // Compute the time which has passed in us.
        std::uint32_t paused = static_cast<std::uint64_t>(osKernelSysTick() - start)
                               * static_cast<std::uint64_t>(1000000)
                               / static_cast<std::uint64_t>(osKernelSysTickFrequency);

        // The duration passed to this_thread::sleep_for() is a lower bound
        // of the actual delay.
        ASSERT_TRUE(paused >= delays[i] * 1000);
        // As no other thread is running, we should wake up within 1 ms.
        ASSERT_TRUE(paused <= (delays[i] + 1) * 1000);
    }
}
#endif
