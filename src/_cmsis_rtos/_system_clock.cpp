/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#include "_system_clock.hpp"

#include "../atomic.hpp"
#include "../thread.hpp"
#include "../type_traits.hpp"

#include <cstdint>


extern "C"
{
// Declarations from ${CMSIS-RTOS}/SRC/rt_Time.h.
extern std::uint32_t os_time;

// Declarations from ${CMSIS-RTOS}/SRC/RTX_Config.h.
extern std::uint32_t const os_trv;
extern std::uint32_t os_tick_val(void);
extern std::uint32_t os_tick_ovf(void);

} // extern "C"


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     Internals
// ----=====================================================================----

// Keep track of the overflows of os_time. For this, we store the
// highest four bits of the os_time together with an overflow counter:
// +-----------------+-----------------+
// | os_time[31:28]  | OverflowCounter |
// |     (4 bit)     |    (28 bit)     |
// +-----------------+-----------------+
//
// This is shared memory and will be updated from multiple threads
// simultaneously.
static atomic<std::uint32_t> weos_chrono_overflowData{0};

// Returns the tick counter of the fastest timer, which is available.
// The function keeps track of the number of overflows in os_time.
// This can only work correctly, if the function is called often enough
// such that no overflow is lost. To be more precise, it has to be
// called at least once per 2^28 os_time-intervals.
static std::int64_t weos_chrono_getSystemClockTicks()
{
    // 1. step: Get a consistent pair of (time, ticks). The following loop
    // might be interrupted at any point and os_time might be changed then.
    std::uint32_t ticks, time, ticksAgain, timeAgain;
    do
    {
        atomic_thread_fence(memory_order_seq_cst);
        ticks = os_tick_val();
        time = os_tick_ovf() ? os_time + 1 : os_time;
        atomic_thread_fence(memory_order_seq_cst);
        ticksAgain = os_tick_val();
        timeAgain = os_tick_ovf() ? os_time + 1 : os_time;
    } while (ticksAgain <= ticks || time != timeAgain);

    // 2. step: Keep track of the overflows in os_time.
    std::uint32_t highBits = time & 0xF0000000;
    std::uint32_t overflows;

    while (1)
    {
        std::uint32_t data = weos_chrono_overflowData;
        std::uint32_t prevHighBits = data & 0xF0000000;
        overflows = data & 0x0FFFFFFF;

        if (highBits == prevHighBits)
        {
            // The likely case: The highest bits have not changed so we are
            // done.
            break;
        }
        else
        {
            // The high bits have changed. If os_time also wrapped around,
            // the overflow counter must be increased. The new counter has
            // to be written back to the shared data. When another thread
            // interfered, start over again.
            if (highBits < prevHighBits)
                ++overflows;
            if (weos_chrono_overflowData.compare_exchange_weak(data, highBits | overflows))
                break;
        }
    }

    return (((std::uint64_t)overflows << 32) | time) * (os_trv + 1) + ticks;
}

namespace chrono
{

// ----=====================================================================----
//     system_clock
// ----=====================================================================----

system_clock::time_point system_clock::now()
{
    static_assert(WEOS_SYSTEM_CLOCK_FREQUENCY % WEOS_SYSTICK_FREQUENCY == 0,
                  "The system clock must be an integer multiple of the SysTick");

    static constexpr std::uint64_t sys_clock_ticks_per_time_interval
            = WEOS_SYSTEM_CLOCK_FREQUENCY / WEOS_SYSTICK_FREQUENCY;

    return time_point(duration(weos_chrono_getSystemClockTicks()
                               / sys_clock_ticks_per_time_interval));
}

// ----=====================================================================----
//     high_resolution_clock
// ----=====================================================================----

high_resolution_clock::time_point high_resolution_clock::now()
{
    return time_point(duration(weos_chrono_getSystemClockTicks()));
}

} // namespace chrono

WEOS_END_NAMESPACE
