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

#ifndef WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
#define WEOS_KEIL_CMSIS_RTOS_THREAD_HPP

#include "../config.hpp"
#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "system_error.hpp"

#include <boost/config.hpp>

#include <cstdint>
#include <utility>

namespace weos
{
class thread;

namespace detail
{

//! Traits for native threads.
struct native_thread_traits
{
    // The native type for a thread ID.
    typedef osThreadId thread_id_type;

    // The stack must be able to hold the registers R0-R15.
    static const std::size_t minimum_custom_stack_size = 64;
};

} // namespace detail
} // namespace weos

#include "../common/thread.hpp"

namespace weos
{
namespace this_thread
{

//! Returns the id of the current thread.
inline
weos::thread::id get_id()
{
    return weos::thread::id(osThreadGetId());
}

namespace detail
{
// A helper to put a thread to sleep.
struct thread_sleeper
{
    thread_sleeper()
    {
    }

    // Waits for millisec milliseconds. The method always returns false because
    // we cannot shortcut a delay.
    bool operator() (std::int32_t millisec) const
    {
        osStatus status = osDelay(millisec);
        WEOS_ASSERT(   (millisec == 0 && status == osOK)
                    || (millisec != 0 && status == osEventTimeout));
        (void)status;
        return false;
    }

private:
    thread_sleeper(const thread_sleeper&);
    const thread_sleeper& operator= (const thread_sleeper&);
};

// A helper to wait for a signal.
struct signal_waiter
{
    // Creates an object which waits for a signal specified by the \p mask.
    explicit signal_waiter(std::uint32_t mask)
        : m_mask(mask)
    {
    }

    // Waits up to \p millisec milliseconds for a signal. Returns \p true,
    // if a signal has arrived and no further waiting is necessary.
    bool operator() (std::int32_t millisec)
    {
        osEvent result = osSignalWait(m_mask, millisec);
        if (result.status == osEventSignal)
        {
            m_mask = result.value.signals;
            return true;
        }

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            ::weos::throw_exception(weos::system_error(
                                        result.status, cmsis_category()));
        }

        return false;
    }

    std::uint32_t mask() const
    {
        return m_mask;
    }

private:
    std::uint32_t m_mask;

   signal_waiter(const signal_waiter&);
    const signal_waiter& operator= (const signal_waiter&);
};

inline
std::uint32_t wait_for_signal(std::uint32_t mask)
{
    osEvent result = osSignalWait(mask, osWaitForever);
    if (result.status != osEventSignal)
    {
        ::weos::throw_exception(weos::system_error(
                                    result.status, cmsis_category()));
    }
    return result.value.signals;
}

inline
std::pair<bool, std::uint32_t> try_wait_for_signal(std::uint32_t mask)
{
    osEvent result = osSignalWait(mask, 0);
    if (result.status == osOK)
    {
        return std::pair<bool, std::uint32_t>(false, 0);
    }
    else if (result.status != osEventSignal)
    {
        ::weos::throw_exception(weos::system_error(
                                    result.status, cmsis_category()));
    }
    return std::pair<bool, std::uint32_t>(true, result.value.signals);
}

template <typename RepT, typename PeriodT>
std::pair<bool, std::uint32_t> try_wait_for_signal_for(
        std::uint32_t mask, const chrono::duration<RepT, PeriodT>& d)
{
    signal_waiter waiter(mask);
    if (chrono::detail::cmsis_wait<
            RepT, PeriodT, signal_waiter>::wait(d, waiter))
    {
        return std::pair<bool, std::uint32_t>(true, waiter.mask());
    }

    return std::pair<bool, std::uint32_t>(false, 0);
}

} // namespace detail

//! Puts the current thread to sleep.
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
void sleep_for(const chrono::duration<RepT, PeriodT>& d) BOOST_NOEXCEPT
{
    detail::thread_sleeper sleeper;
    chrono::detail::cmsis_wait<RepT, PeriodT, detail::thread_sleeper>::wait(
                d, sleeper);
}

template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& timePoint) BOOST_NOEXCEPT;

//! Waits for a signal.
//! Blocks the current thread until it receives any signal(s) and returns the
//! signals.
inline
std::uint32_t wait_for_signal()
{
    return detail::wait_for_signal(0);
}

//! Checks if a signal has arrived.
//! Checks if any signal has reached the current thread. If so, the boolean
//! in the returned pair is set to \p true and the second member contains
//! the signal flags. Otherwise, the boolean is set to \p false.
inline
std::pair<bool, std::uint32_t> try_wait_for_signal()
{
    return detail::try_wait_for_signal(0);
}

//! Waits until any signal arrives or a timeout occurs.
//! Waits until any signal reaches the current thread or the timeout period
//! \p d expires. The boolean in the returned pair is set to \p true if a
//! signal was present. In this case, the second member contains the signal
//! flags. If the timeout expired, the boolean is set to \p false.
template <typename RepT, typename PeriodT>
std::pair<bool, std::uint32_t> try_wait_for_signal_for(
        const chrono::duration<RepT, PeriodT>& d)
{
    return detail::try_wait_for_signal_for(0, d);
}

//! Waits for a signal or a set of signals.
//! Blocks the current thread until the signals specified by the \p mask have
//! arrived. Then the signal flags are returned.
inline
std::uint32_t wait_for_signal(std::uint32_t mask)
{
    WEOS_ASSERT(mask > 0 && mask < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::wait_for_signal(mask);
}

inline
std::pair<bool, std::uint32_t> try_wait_for_signal(std::uint32_t mask)
{
    WEOS_ASSERT(mask > 0 && mask < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::try_wait_for_signal(mask);
}

template <typename RepT, typename PeriodT>
std::pair<bool, std::uint32_t> try_wait_for_signal_for(
        std::uint32_t mask, const chrono::duration<RepT, PeriodT>& d)
{
    WEOS_ASSERT(mask > 0 && mask < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::try_wait_for_signal_for(mask, d);
}

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    osStatus status = osThreadYield();
    WEOS_ASSERT(status == osOK);
    (void)status;
}

} // namespace this_thread
} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
