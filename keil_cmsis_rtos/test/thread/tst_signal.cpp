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

#include "sparring.hpp"
#include "../common/testutils.hpp"
#include "gtest/gtest.h"

TEST(signal, wait)
{
    SparringData data;
    weos::thread t(sparring, &data);
    osDelay(10);
    ASSERT_TRUE(data.sparringStarted);

    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned mask = 1 << i;

        data.mask = 0;
        data.action = SparringData::WaitForSignal;
        osDelay(10);
        ASSERT_TRUE(data.busy);
        ASSERT_TRUE(data.mask == 0);

        t.set_signal(mask);
        osDelay(10);
        ASSERT_FALSE(data.busy);
        ASSERT_TRUE(data.mask == mask);
    }

    for (unsigned i = 0; i < 100; ++i)
    {
        unsigned mask = (random() % 0xFFFF) + 1;

        data.mask = 0;
        data.action = SparringData::WaitForSignal;
        osDelay(10);
        ASSERT_TRUE(data.busy);
        ASSERT_TRUE(data.mask == 0);

        t.set_signal(mask);
        osDelay(10);
        ASSERT_FALSE(data.busy);
        ASSERT_TRUE(data.mask == mask);
    }

    data.action = SparringData::Terminate;
    t.join();
}
