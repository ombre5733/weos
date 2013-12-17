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

#include "../../condition_variable.hpp"
#include "sparring.hpp"

#include "gtest/gtest.h"

TEST(condition_variable, Constructor)
{
    weos::condition_variable cv;
}

TEST(condition_variable, try_wait_for)
{
    weos::condition_variable cv;
    weos::mutex m;
    weos::unique_lock<weos::mutex> lock(m);
    weos::cv_status::cv_status status
            = cv.wait_for(lock, weos::chrono::milliseconds(1));
    ASSERT_EQ(weos::cv_status::timeout, status);
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(condition_variable, notify)
{
#define clearNotifications()                                                   \
    do {                                                                       \
        data1.notified = false;                                                \
        data2.notified = false;                                                \
        data3.notified = false;                                                \
    } while (0)

#define numNotifications()                                                     \
    (int(data1.notified) + int(data2.notified) + int(data3.notified))

    weos::mutex m;
    weos::condition_variable cv;
    SparringData data1(m, cv);
    osThreadId id1 = osThreadCreate(&sparringThread, &data1);
    ASSERT_TRUE(id1 != 0);
    SparringData data2(m, cv);
    osThreadId id2 = osThreadCreate(&sparringThread, &data2);
    ASSERT_TRUE(id2 != 0);
    SparringData data3(m, cv);
    osThreadId id3 = osThreadCreate(&sparringThread, &data3);
    ASSERT_TRUE(id3 != 0);

    osDelay(10);
    ASSERT_TRUE(data1.sparringStarted);
    ASSERT_TRUE(data2.sparringStarted);
    ASSERT_TRUE(data3.sparringStarted);

    clearNotifications();
    ASSERT_EQ(0, numNotifications());

    data1.action = SparringData::ConditionVariableWait;
    data2.action = SparringData::ConditionVariableWait;
    data3.action = SparringData::ConditionVariableWait;
    osDelay(10);
    ASSERT_TRUE(data1.busy);
    ASSERT_TRUE(data2.busy);
    ASSERT_TRUE(data3.busy);

    cv.notify_one();
    osDelay(10);
    ASSERT_EQ(1, numNotifications());

    cv.notify_one();
    osDelay(10);
    ASSERT_EQ(2, numNotifications());

    cv.notify_one();
    osDelay(10);
    ASSERT_EQ(3, numNotifications());

    ASSERT_FALSE(data1.busy);
    ASSERT_FALSE(data2.busy);
    ASSERT_FALSE(data3.busy);

    clearNotifications();
    ASSERT_EQ(0, numNotifications());

    data1.action = SparringData::ConditionVariableWait;
    data2.action = SparringData::ConditionVariableWait;
    data3.action = SparringData::ConditionVariableWait;
    osDelay(10);
    ASSERT_TRUE(data1.busy);
    ASSERT_TRUE(data2.busy);
    ASSERT_TRUE(data3.busy);

    cv.notify_all();
    osDelay(10);
    ASSERT_EQ(3, numNotifications());

    ASSERT_FALSE(data1.busy);
    ASSERT_FALSE(data2.busy);
    ASSERT_FALSE(data3.busy);

    data1.action = SparringData::Terminate;
    data2.action = SparringData::Terminate;
    data3.action = SparringData::Terminate;
    osDelay(10);
}
