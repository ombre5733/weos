/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2015, Manuel Freiberger
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

#include "../../messagequeue.hpp"

#include "gtest/gtest.h"

TEST(message_queue, Constructor)
{
    weos::message_queue<std::int32_t, 1> q1;
    ASSERT_EQ(1, q1.capacity());

    weos::message_queue<std::int32_t, 13> q13;
    ASSERT_EQ(13, q13.capacity());
}

TEST(message_queue, try_get)
{
    weos::message_queue<std::int32_t, 1> q;
    std::pair<bool, std::int32_t> result = q.try_receive();
    ASSERT_FALSE(result.first);
}

TEST(message_queue, put)
{
    std::pair<bool, std::int32_t> result;

    weos::message_queue<std::int32_t, 1> q;
    q.send(0x12345678);
    ASSERT_EQ(0x12345678, q.receive());

    q.send(0x23456789);
    result = q.try_receive();
    ASSERT_TRUE(result.first);
    ASSERT_EQ(0x23456789, result.second);

    q.send(0x34567890);
    result = q.try_receive_for(weos::chrono::milliseconds(1));
    ASSERT_TRUE(result.first);
    ASSERT_EQ(0x34567890, result.second);
}

#if 0

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(sparring_mutex, lock)
{
    SparringData data;
    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);
    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);

    data.mutex.lock();
    data.action = SparringData::MutexLock;
    osDelay(10);
    ASSERT_TRUE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.mutex.unlock();
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    osDelay(10);
}

TEST(sparring_mutex, try_lock)
{
    SparringData data;
    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);
    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);

    data.mutex.lock();
    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.mutex.unlock();
    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    osDelay(10);
}
#endif
