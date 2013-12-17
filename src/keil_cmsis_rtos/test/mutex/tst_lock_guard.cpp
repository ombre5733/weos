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

#include "../../mutex.hpp"
#include "sparring.hpp"

#include "gtest/gtest.h"

TEST(lock_guard, Constructor)
{
    weos::mutex m;
    {
        weos::lock_guard<weos::mutex> l(m);
        ASSERT_FALSE(m.try_lock());
    }
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

TEST(lock_guard, adopt_lock)
{
    weos::mutex m;
    m.lock();
    {
        weos::lock_guard<weos::mutex> l(m, weos::adopt_lock);
        ASSERT_FALSE(m.try_lock());
    }
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(sparring_lock_guard, lock)
{
    SparringData data;
    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);
    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);

    {
        weos::lock_guard<weos::mutex> lock(data.mutex);

        data.action = SparringData::MutexLock;
        osDelay(10);
        ASSERT_TRUE(data.busy);
        ASSERT_FALSE(data.mutexLocked);
    }

    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    osDelay(10);
}

TEST(sparring_lock_guard, try_lock)
{
    SparringData data;
    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);
    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);

    {
        weos::lock_guard<weos::mutex> lock(data.mutex);

        data.action = SparringData::MutexTryLock;
        osDelay(10);
        ASSERT_FALSE(data.busy);
        ASSERT_FALSE(data.mutexLocked);
    }

    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    osDelay(10);
}
