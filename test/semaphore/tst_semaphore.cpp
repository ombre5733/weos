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

#include <semaphore.hpp>
#include <thread.hpp>

#include "gtest/gtest.h"

namespace
{

struct SparringData
{
    enum Action
    {
        None,
        SemaphorePost,
        SemaphoreWait,
        SemaphoreTryWait,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          sparringStarted(false)
    {
    }

    weos::semaphore semaphore;
    volatile Action action;
    volatile bool busy;
    volatile bool sparringStarted;
};

void sparring(SparringData* data)
{
    data->sparringStarted = true;

    while (1)
    {
        if (data->action == SparringData::None)
        {
            weos::this_thread::sleep_for(weos::chrono::milliseconds(1));
            continue;
        }
        else if (data->action == SparringData::Terminate)
            break;

        data->busy = true;
        switch (data->action)
        {
            case SparringData::SemaphorePost:
                data->semaphore.post();
                break;
            case SparringData::SemaphoreWait:
                data->semaphore.wait();
                break;
            case SparringData::SemaphoreTryWait:
                data->semaphore.try_wait();
                break;
            default:
                break;
        }
        data->busy = false;
        data->action = SparringData::None;
    }
}

} // anonymous namespace

TEST(semaphore, Constructor)
{
    {
        weos::semaphore s;
        ASSERT_EQ(0, s.value());
    }
    for (std::uint32_t count = 0; count < 0xFFFF; count += 123)
    {
        weos::semaphore s(count);
        ASSERT_EQ(count, s.value());
    }
    {
        weos::semaphore s(65535);
        ASSERT_EQ(65535, s.value());
    }
}

TEST(semaphore, post)
{
    for (std::uint32_t count = 0; count < 0xFFFF; count += 123)
    {
        weos::semaphore s(count);
        ASSERT_EQ(count, s.value());
        s.post();
        ASSERT_EQ(count + 1, s.value());
    }
    {
        weos::semaphore s(65534);
        ASSERT_EQ(65534, s.value());
        s.post();
        ASSERT_EQ(65535, s.value());
    }
}

TEST(semaphore, wait)
{
    for (std::uint32_t count = 1; count < 0xFFFF; count += 123)
    {
        weos::semaphore s(count);
        ASSERT_EQ(count, s.value());
        s.wait();
        ASSERT_EQ(count - 1, s.value());
    }
    {
        weos::semaphore s(65535);
        ASSERT_EQ(65535, s.value());
        s.wait();
        ASSERT_EQ(65534, s.value());
    }
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(sparring_semaphore, post_and_wait)
{
    SparringData data;
    weos::thread sparringThread(sparring, &data);
    ASSERT_TRUE(sparringThread.joinable());
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    data.action = SparringData::SemaphoreWait;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.busy);
    ASSERT_EQ(0, data.semaphore.value());

    data.semaphore.post();
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.semaphore.value());

    data.semaphore.post();
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_EQ(1, data.semaphore.value());

    data.action = SparringData::SemaphoreWait;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.semaphore.value());

    data.action = SparringData::SemaphorePost;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(1, data.semaphore.value());

    data.semaphore.wait();
    ASSERT_EQ(0, data.semaphore.value());

    data.action = SparringData::Terminate;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));

    sparringThread.join();
    ASSERT_FALSE(sparringThread.joinable());
}
