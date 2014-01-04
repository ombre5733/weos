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

struct SparringData
{
    enum Action
    {
        None,
        WaitForAnySignal,
        WaitForSignals,
        TryWaitForSignal,
        TryWaitForSignalFor,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          caughtSignals(0),
          waitMask(0),
          sparringStarted(false)
    {
    }

    volatile Action action;
    volatile bool busy;
    volatile std::uint32_t caughtSignals;
    volatile std::uint32_t waitMask;
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
            case SparringData::WaitForAnySignal:
                data->caughtSignals = weos::this_thread::wait_for_any_signal();
                break;
            case SparringData::WaitForSignals:
                data->caughtSignals = weos::this_thread::wait_for_signals(data->waitMask);
                break;
            default:
                break;
        }
        data->busy = false;
        data->action = SparringData::None;
    }
}

TEST(signal, wait_for_any_signal)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Set a single signal and assert that it is caught.
    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned mask = 1 << i;

        data.caughtSignals = 0;
        data.action = SparringData::WaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        t.set_signals(mask);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(mask, data.caughtSignals);
    }

    // Set a bunch of signals and assert that all of them are caught.
    for (unsigned i = 0; i < 100; ++i)
    {
        unsigned mask = (random() % 0xFFFF) + 1;

        data.caughtSignals = 0;
        data.action = SparringData::WaitForAnySignal;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        t.set_signals(mask);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_EQ(mask, data.caughtSignals);
    }

    data.action = SparringData::Terminate;
    t.join();
}

TEST(signal, wait_for_signals)
{
    SparringData data;
    weos::thread t(sparring, &data);
    weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
    ASSERT_TRUE(data.sparringStarted);

    // Wait for a single signal.
    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned mask = 1 << i;

        data.caughtSignals = 0;
        data.waitMask = mask;
        data.action = SparringData::WaitForSignals;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_EQ(0, data.caughtSignals);

        for (unsigned j = 0; j < 16; ++j)
        {
            if (j == i)
                continue;

            t.set_signals(1 << j);
            weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
            ASSERT_TRUE(data.busy);
            ASSERT_EQ(0, data.caughtSignals);
        }

        t.set_signals(mask);
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_FALSE(data.busy);
        ASSERT_TRUE(data.caughtSignals != 0);

        // Clear all signals which have not been caught.
        t.clear_signals(0xFFFF);
    }

    for (unsigned i = 0; i < 100; ++i)
    {
        unsigned mask = (random() % 0xFFFF) + 1;

        data.caughtSignals = 0;
        data.waitMask = mask;
        data.action = SparringData::WaitForSignals;
        weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
        ASSERT_TRUE(data.busy);
        ASSERT_TRUE(data.caughtSignals == 0);

        for (unsigned j = 0; j < 16; ++j)
        {
            mask &= ~(1 << j);
            t.set_signals(1 << j);
            weos::this_thread::sleep_for(weos::chrono::milliseconds(10));
            if (mask != 0)
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

        // Clear all signals which have not been caught.
        t.clear_signals(0xFFFF);
    }

    data.action = SparringData::Terminate;
    t.join();
}
