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

#include <mutex.hpp>
#include <thread.hpp>

#include "gtest/gtest.h"

namespace
{

struct SparringData
{
    enum Action
    {
        None,
        MutexLock,
        MutexTryLock,
        MutexUnlock,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          mutexLocked(false),
          sparringStarted(false)
    {
    }

    weos::mutex mutex;
    volatile Action action;
    volatile bool busy;
    volatile bool mutexLocked;
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
            case SparringData::MutexLock:
                data->mutex.lock();
                data->mutexLocked = true;
                break;
            case SparringData::MutexTryLock:
                data->mutexLocked = data->mutex.try_lock();
                break;
            case SparringData::MutexUnlock:
                data->mutex.unlock();
                data->mutexLocked = false;
                break;
            default:
                break;
        }
        data->busy = false;
        data->action = SparringData::None;
    }
}

} // anonymous namespace

TEST(mutex, construct_and_destruct)
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

    m.lock();
    result = m.try_lock();
    ASSERT_FALSE(result);
    m.unlock();
}

// ----=====================================================================----
//     Tests together with a sparring thread
// ----=====================================================================----

TEST(sparring_mutex, lock)
{
    SparringData data;
    weos::thread sparringThread(sparring, &data);
    ASSERT_TRUE(sparringThread.joinable());
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    data.mutex.lock();
    data.action = SparringData::MutexLock;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.mutex.unlock();
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    sparringThread.join();
    ASSERT_FALSE(sparringThread.joinable());
}

TEST(sparring_mutex, try_lock)
{
    SparringData data;
    weos::thread sparringThread(sparring, &data);
    ASSERT_TRUE(sparringThread.joinable());
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    data.mutex.lock();
    data.action = SparringData::MutexTryLock;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.mutex.unlock();
    data.action = SparringData::MutexTryLock;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_TRUE(data.mutexLocked);

    ASSERT_FALSE(data.mutex.try_lock());

    data.action = SparringData::MutexUnlock;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_FALSE(data.mutexLocked);

    data.action = SparringData::Terminate;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    sparringThread.join();
    ASSERT_FALSE(sparringThread.joinable());
}
