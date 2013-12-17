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
#include "sparring_recursive.hpp"

#include "gtest/gtest.h"

TEST(recursive_mutex, Constructor)
{
    weos::recursive_mutex m;
}

TEST(recursive_mutex, lock)
{
    weos::recursive_mutex m;
    m.lock();
    m.lock();
    m.unlock();
    m.unlock();
}

TEST(recursive_mutex, try_lock)
{
    weos::recursive_mutex m;
    bool result;
    result = m.try_lock();
    ASSERT_TRUE(result);
    result = m.try_lock();
    ASSERT_TRUE(result);
    m.unlock();
    m.unlock();

    m.lock();
    result = m.try_lock();
    ASSERT_TRUE(result);
    m.unlock();
    m.unlock();

    result = m.try_lock();
    ASSERT_TRUE(result);
    m.lock();
    m.unlock();
    m.unlock();
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(sparring_recursive_mutex, lock)
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
    ASSERT_EQ(0, data.numLocks);

    data.mutex.lock();
    osDelay(10);
    ASSERT_TRUE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.mutex.unlock();
    osDelay(10);
    ASSERT_TRUE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.mutex.unlock();
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(1, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(2, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(1, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.action = SparringData::Terminate;
    osDelay(10);
}

TEST(sparring_recursive_mutex, try_lock)
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
    ASSERT_EQ(0, data.numLocks);

    data.mutex.lock();
    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.mutex.unlock();
    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.mutex.unlock();
    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(1, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexTryLock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(2, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(1, data.numLocks);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    osDelay(10);
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.numLocks);

    data.action = SparringData::Terminate;
    osDelay(10);
}
