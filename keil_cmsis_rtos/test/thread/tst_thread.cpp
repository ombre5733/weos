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

#include "../../thread.hpp"

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

void empty_thread(void* arg)
{
}

void delay_thread(void* arg)
{
    std::uint32_t delayTime = reinterpret_cast<std::uint32_t>(arg);
    osDelay(delayTime);
}

TEST(thread, DefaultConstructor)
{
    weos::thread t;
    ASSERT_FALSE(t.joinable());
}

TEST(thread, ConstructorWithFunction)
{
    weos::thread t(empty_thread, 0);
    ASSERT_TRUE(t.joinable());
    t.join();
}

#if 0
TEST(thread, FailingConstructorWithFunction)
{
    weos::thread t(empty_thread, 0);
    ASSERT_TRUE(t.joinable());
}
#endif

TEST(thread, start_one_thread_very_often)
{
    for (unsigned i = 0; i < 10000; ++i)
    {
        weos::thread t(empty_thread, 0);
        ASSERT_TRUE(t.joinable());
        t.join();
    }
}

TEST(thread, start_all_in_parallel)
{
    weos::thread* threads[WEOS_MAX_NUM_CONCURRENT_THREADS];
    for (unsigned i = 0; i < WEOS_MAX_NUM_CONCURRENT_THREADS; ++i)
    {
        threads[i] = new weos::thread(delay_thread, (void*)5);
        ASSERT_TRUE(threads[i]->joinable());
    }
    for (unsigned i = 0; i < WEOS_MAX_NUM_CONCURRENT_THREADS; ++i)
    {
        threads[i]->join();
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
            threads[index] = new weos::thread(delay_thread, (void*)delayTime);
            ASSERT_TRUE(threads[index]->joinable());
        }
        else
        {
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

TEST(thread, sleep_for)
{
    std::uint32_t delays[] = { 0,   1,   2,   3,   4,   5,
                                   10,  20,  30,  40,  50,
                                  100, 200, 300, 400, 500};
    for (unsigned i = 0; i < sizeof(delays) / sizeof(delays[0]); ++i)
    {
        std::uint32_t start = osKernelSysTick();
        weos::this_thread::sleep_for(weos::chrono::milliseconds(1));
        // Compute the time which has passed in us.
        std::uint32_t paused = static_cast<std::uint64_t>(osKernelSysTick() - start)
                               * static_cast<std::uint64_t>(1000000)
                               / static_cast<std::uint64_t>(osKernelSysTickFrequency);
        ASSERT_TRUE(paused > delays[i] * 1000);
        ASSERT_TRUE(paused < (delays[i] + 1) * 1000);
    }
}
