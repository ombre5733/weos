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

#include "chrono.hpp"

#include "../atomic.hpp"
#include "../thread.hpp"
#include "../type_traits.hpp"

#include <cstdint>

// Declaration from ../3rdparty/keil_cmsis_rtos/SRC/rt_Time.h.
extern "C" std::uint32_t rt_time_get(void);

WEOS_BEGIN_NAMESPACE

static atomic<std::uint32_t> weos_chrono_overflowData(0);

std::int64_t getPrecisionTicks()
{
    static const std::uint64_t ticks_per_time_interval = 1000/*TODO*/;

    // Dealing with the precision time is a bit tricky:
    // - The resultant time is made up by combining the OS time
    //   (a low frequency counter) with the SysTick (a high frequency counter).
    // - We have to read the SysTick and the OS time in a consistent state,
    //   i.e. within one SysTick interval.
    // - If we read the SysTick just before an overflow and the OS time
    //   just after it and simply combine the two values, we would report a
    //   time which is somewhere in the future (because SysTick is too large).
    // - We want to have a monotonic clock, so reporting a time which is
    //   somewhere in the future is no option.
    // - The OS time is only 32-bit, but we would like to have 64-bit bit
    //   so that the user code does not have to worry about overflows.

    // Let's generate a valid pair of (OS time, SysTick). Read the SysTick and
    // then the OS time. Finally, read the SysTick again to detect overflows.
    std::uint32_t ticks = osKernelSysTick();
    std::uint32_t time = rt_time_get();
    std::uint32_t ticksAgain = osKernelSysTick();

    // If ticksAgain >= ticks, there is no problem even if there have been
    // multiple overflows of the SysTick in between. The relation
    // (time, ticks) <= (OS time, SysTick) always holds in this case, i.e.
    // (time, ticks) is not somewhere in the future.

    if (ticksAgain < ticks)
    {
        // The SysTick overflowed at least once. At this point, we do not know
        // if we have called os_time_get() before the first overflow or
        // after it. The table below lists the pairs
        //     x := (time, ticks)
        //     y := (time, ticksAgain)
        //     z := (time + 1, ticksAgain)
        //
        // 1. case: Fetched OS time before overflow.
        // (0x0010, 0x0FF1) --> ticks      = 0x0FF1
        // (0x0010, 0x0FFC) --> time       = 0x0010
        // (0x0011, 0x0008) --> ticksAgain = 0x0008
        //     x = (0x0010, 0x0FF1) ... valid
        //     y = (0x0010, 0x0008) ... invalid (in the past)
        //     z = (0x0011, 0x0008) ... valid
        //
        // 2. case: Fetched OS time after overflow.
        // (0x0010, 0x0FF1) --> ticks      = 0x0FF1
        // (0x0011, 0x0002) --> time       = 0x0011
        // (0x0011, 0x0008) --> ticksAgain = 0x0008
        //     x = (0x0011, 0x0FF1) ... invalid (in the future)
        //     y = (0x0011, 0x0008) ... valid
        //     z = (0x0012, 0x0008) ... invalid (in the future)
        //
        // We choose z for the first case and y for the second.

        ticks = ticksAgain;
        std::uint32_t timeAgain = rt_time_get();
        if (time != timeAgain)
            ++time;

        // Note: There is the potential problem that timeAgain = time + 2^32,
        // i.e. there have been 2^32 overflows. However, this would mean
        // that the current task had been blocked for days.
    }

    // Read the shared overflow data. It consists of an overflow counter plus
    // the highest four bits of the OS time.
    std::uint32_t previousData = weos_chrono_overflowData;
    std::uint32_t previousHighBits = previousData & 0x0F;
    std::uint32_t previousTimeOverflows = previousData >> 4;

    std::uint32_t highBits = time >> 28;
    std::uint32_t timeOverflows = previousTimeOverflows;

    // If the current time is less than the time stored previously, there has
    // been an overflow.
    if (highBits < previousHighBits)
        ++timeOverflows;

    // Now it comes to updating the shared overflow data. It is possible that
    // other threads want to do the same in parallel. We try to update the
    // shared data as long as our time is ahead of the shared previous time.
    while (highBits != previousHighBits
           && (highBits - previousHighBits) % 16 < 7)
    {
        if (weos_chrono_overflowData.compare_exchange_weak(
                previousData, (timeOverflows << 4) | highBits))
        {
            break;
        }

        // Another thread has changed the shared data in between. Update
        // the previous high bits.
        previousHighBits = previousData & 0x0F;
    }

    // By combining the time with its overflow counter, we can create a
    // 60-bit time.
    std::uint64_t longTime = (std::uint64_t(timeOverflows) << 32) + time;
    return longTime * ticks_per_time_interval + ticks;
}

static aligned_storage<200>::type precisionTimeReaderStack;

static void readPrecisionTimePeriodically()
{
    while (true)
    {
        thread::signal_set signal = this_thread::try_wait_for_any_signal_for(
                                        chrono::seconds(1));
        if (signal)
            break;
        getPrecisionTicks();
    }
}

WEOS_END_NAMESPACE
