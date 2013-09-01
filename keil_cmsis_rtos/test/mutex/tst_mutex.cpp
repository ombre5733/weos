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

TEST(mutex, Constructor)
{
    weos::mutex m;
}

TEST(mutex, lock)
{
    weos::mutex m;
    m.lock();
    m.unlock();
}

TEST(mutex, try_lock)
{
    weos::mutex m;
    bool result;
    result = m.try_lock();
    ASSERT_TRUE(result);
    result = m.try_lock();
    ASSERT_FALSE(result);
    m.unlock();
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----
#if 0
TEST(sparring_mutex, lock)
{
    SparringData data(SparringData::MutexLock);
    data.mutex.lock();

    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);

    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);
    ASSERT_FALSE(data.mutexLocked);
    data.mutex.unlock();
    osDelay(10);
    ASSERT_TRUE(data.mutexLocked);

    osStatus result = osThreadTerminate(sparringId);
    ASSERT_EQ(osOK, result);
}

TEST(sparring_mutex, try_lock)
{
    SparringData data(SparringData::MutexTryLock);
    data.mutex.lock();

    osThreadId sparringId = osThreadCreate(&sparringThread, &data);
    ASSERT_TRUE(sparringId != 0);

    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);
    ASSERT_FALSE(data.mutexLocked);
    data.mutex.unlock();
    osDelay(10);
    ASSERT_TRUE(data.mutexLocked);

    osStatus result = osThreadTerminate(sparringId);
    ASSERT_EQ(osOK, result);
}
#endif
