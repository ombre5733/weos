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

#include "gtest/gtest.h"

TEST(thread, sleep_for)
{
    std::uint32_t delays[] = { 0,   1,   2,   3,   4,   5,
                                   10,  20,  30,  40,  50,
                                  100, 200, 300, 400, 500};
    for (unsigned i = 0; i < sizeof(delays) / sizeof(delays[0]); ++i)
    {
        weos::chrono::high_resolution_clock::time_point start
                = weos::chrono::high_resolution_clock::now();

        weos::this_thread::sleep_for(weos::chrono::milliseconds(delays[i]));

        weos::chrono::high_resolution_clock::time_point end
                = weos::chrono::high_resolution_clock::now();

        // The duration passed to this_thread::sleep_for() must be a lower
        // bound of the actual delay.
        ASSERT_TRUE(end - start >= weos::chrono::milliseconds(delays[i]));

        // As no other thread is running, we should wake up within 1 ms.
        ASSERT_TRUE(end - start < weos::chrono::milliseconds(delays[i] + 1));
    }
}
