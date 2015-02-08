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

#include <thread.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

namespace
{

#ifdef WEOS_MAX_NUM_CONCURRENT_THREADS
const int MAX_NUM_PARALLEL_TEST_THREADS = WEOS_MAX_NUM_CONCURRENT_THREADS;
#else
const int MAX_NUM_PARALLEL_TEST_THREADS = 10;
#endif // WEOS_MAX_NUM_CONCURRENT_THREADS

struct SparringData
{
    enum Action
    {
        None,
        TryWaitForAnySignal,
        TryWaitForAllSignals,
        WaitForAnySignal,
        WaitForAllSignals,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          caughtSignals(0),
          waitFlags(0),
          sparringStarted(false)
    {
    }

    volatile Action action;
    volatile bool busy;
    volatile weos::thread::signal_set caughtSignals;
    volatile weos::thread::signal_set waitFlags;
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
        case SparringData::TryWaitForAnySignal:
        {
            data->caughtSignals
                    = weos::this_thread::try_wait_for_any_signal();
        } break;

        case SparringData::TryWaitForAllSignals:
        {
            data->caughtSignals = 0;
            if (weos::this_thread::try_wait_for_all_signals(
                    data->waitFlags))
                data->caughtSignals = data->waitFlags;
        } break;

        case SparringData::WaitForAnySignal:
        {
            data->caughtSignals = weos::this_thread::wait_for_any_signal();
        } break;

        case SparringData::WaitForAllSignals:
        {
            weos::this_thread::wait_for_all_signals(data->waitFlags);
            data->caughtSignals = data->waitFlags;
        } break;

        default:
            break;
        }

        data->busy = false;
        data->action = SparringData::None;
    }
}

} // anonymous namespace

TEST(signal, no_signals_in_new_thread)
{
    weos::thread threads[MAX_NUM_PARALLEL_TEST_THREADS];
    SparringData data[MAX_NUM_PARALLEL_TEST_THREADS];

    for (int i = 0; i < 10 * MAX_NUM_PARALLEL_TEST_THREADS; ++i)
    {
        int idx = i % MAX_NUM_PARALLEL_TEST_THREADS;
        if (threads[idx].joinable())
        {
            data[idx].action = SparringData::Terminate;
            threads[idx].join();
        }

        threads[idx] = weos::thread(sparring, &data[idx]);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(5));
        ASSERT_TRUE(data[idx].sparringStarted);

        data[idx].caughtSignals = 0;
        data[idx].action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(5));
        ASSERT_FALSE(data[idx].busy);
        ASSERT_EQ(0, data[idx].caughtSignals);

        // Set all flags to ensure that they are erased when a new thread is
        // created.
        threads[idx].set_signals(weos::thread::all_signals());
    }

    for (int i = 0; i < MAX_NUM_PARALLEL_TEST_THREADS; ++i)
    {
        if (threads[i].joinable())
        {
            data[i].action = SparringData::Terminate;
            threads[i].join();
        }
    }
}

TEST(signal, set_signals)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Set all signal flags and catch them.
    t.set_signals(weos::thread::all_signals());
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    data.caughtSignals = 0;
    data.action = SparringData::TryWaitForAnySignal;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(weos::thread::all_signals(), data.caughtSignals);

    // Make sure that the signal flags have been cleared.
    data.action = SparringData::TryWaitForAnySignal;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.caughtSignals);

    // Set the signal flags one by one and catch them all.
    for (int i = 0; i < weos::thread::signals_count(); ++i)
    {
        weos::thread::signal_set flag = 1 << i;
        t.set_signals(flag);
    }
    data.action = SparringData::TryWaitForAnySignal;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(weos::thread::all_signals(), data.caughtSignals);

    // Make sure that the signal flags have been cleared.
    data.action = SparringData::TryWaitForAnySignal;
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_FALSE(data.busy);
    ASSERT_EQ(0, data.caughtSignals);

    data.action = SparringData::Terminate;
    t.join();
}

TEST(signal, wait_for_any_signal)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Set a single signal and assert that it is caught.
    for (int i = 0; i < weos::thread::signals_count(); ++i)
    {
        weos::thread::signal_set flag = 1 << i;

        data.caughtSignals = 0;
        data.action = SparringData::WaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        t.set_signals(flag);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(flag, data.caughtSignals);

        // Make sure that the signal flags have been cleared.
        data.action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);
    }

    // Test with signal_traits::all_flags.
    {
        data.caughtSignals = 0;
        data.action = SparringData::WaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        t.set_signals(weos::thread::all_signals());
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(weos::thread::all_signals(), data.caughtSignals);

        // Make sure that the signal flags have been cleared.
        data.action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);
    }

    data.action = SparringData::Terminate;
    t.join();
}

TEST(signal, wait_for_any_signal_randomly)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Set a bunch of signals and assert that all of them are caught.
    for (int i = 0; i < 100; ++i)
    {
        weos::thread::signal_set flag
                = 1 + (testing::random() % weos::thread::all_signals());

        data.caughtSignals = 0;
        data.action = SparringData::WaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        t.set_signals(flag);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(flag, data.caughtSignals);

        // Make sure that the signal flags have been cleared.
        data.action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);
    }

    data.action = SparringData::Terminate;
    t.join();
}

TEST(signal, wait_for_all_signals)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Wait for a single signal.
    for (int i = 0; i < weos::thread::signals_count(); ++i)
    {
        weos::thread::signal_set flag = 1 << i;

        data.caughtSignals = 0;
        data.waitFlags = flag;
        data.action = SparringData::WaitForAllSignals;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        // Set all signals except the one for which we wait. The sparring
        // thread must still block.
        for (int j = 0; j < weos::thread::signals_count(); ++j)
        {
            if (j != i)
                t.set_signals(1 << j);
        }
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        // Set the remaining signal.
        t.set_signals(flag);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(flag, data.caughtSignals);

        // The other signals should still be intact.
        data.action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(weos::thread::all_signals() & ~flag, data.caughtSignals);
    }

    data.action = SparringData::Terminate;
    t.join();
}

TEST(signal, wait_for_all_signals_randomly)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    for (int i = 0; i < 100; ++i)
    {
        weos::thread::signal_set flags
                = 1 + (testing::random() % weos::thread::all_signals());

        data.caughtSignals = 0;
        data.waitFlags = flags;
        data.action = SparringData::WaitForAllSignals;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_TRUE(data.caughtSignals == 0);

        weos::thread::signal_set temp = flags;
        for (int j = 0; j < weos::thread::signals_count(); ++j)
        {
            weos::thread::signal_set flag = 1 << j;
            temp &= ~flag;
            t.set_signals(flag);
            weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
            if (temp != 0)
            {
                ASSERT_TRUE(data.busy);
                ASSERT_EQ(0, data.caughtSignals);
            }
            else
            {
                ASSERT_FALSE(data.busy);
                ASSERT_TRUE(data.caughtSignals != 0);
            }
        }

        // The other signals should still be intact.
        data.action = SparringData::TryWaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(weos::thread::all_signals() & ~flags,
                  data.caughtSignals);
    }

    data.action = SparringData::Terminate;
    t.join();
}
