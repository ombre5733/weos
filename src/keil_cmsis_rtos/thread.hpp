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
#include "algorithm.hpp"
#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "system_error.hpp"

#include <boost/config.hpp>
#include <boost/static_assert.hpp>

#include <cstdint>


//! \todo Remove
#include <cstdio>

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

    BOOST_STATIC_ASSERT(osFeature_Signals > 0 && osFeature_Signals <= 16);

    // Represents a set of signals.
    typedef std::uint16_t signal_set;

    // Returns the number of signals in a set.
    inline
    static int signals_count()
    {
        return osFeature_Signals;
    }

    // Returns a signal set with all flags being set.
    inline
    static signal_set all_signals()
    {
        return (std::uint32_t(1) << osFeature_Signals) - 1;
    }

    // Clears the given signal flags of the thread selected by the threadId.
    static void clear_signals(thread_id_type threadId, signal_set flags)
    {
        WEOS_ASSERT(flags < (std::uint32_t(1) << (osFeature_Signals)));
        std::int32_t result = osSignalClear(threadId, flags);
        WEOS_ASSERT(result >= 0);
        (void)result;
    }

    // Sets the given signal flags of the thread selected by the threadId.
    static void set_signals(thread_id_type threadId, signal_set flags)
    {
        WEOS_ASSERT(flags < (std::uint32_t(1) << (osFeature_Signals)));
        std::int32_t result = osSignalSet(threadId, flags);
        WEOS_ASSERT(result >= 0);
        (void)result;
    }
};

} // namespace detail
} // namespace weos

#include "../common/thread_detail.hpp"

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

inline
void clear_signals(thread::signal_set flags)
{
    if (osThreadGetId() == 0)
    {
        ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }
    detail::native_thread_traits::clear_signals(osThreadGetId(), flags);
}

inline
void set_signals(thread::signal_set flags)
{
    if (osThreadGetId() == 0)
    {
        ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }
    detail::native_thread_traits::set_signals(osThreadGetId(), flags);
}

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    osStatus status = osThreadYield();
    WEOS_ASSERT(status == osOK);
    (void)status;
}

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

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
    // Creates an object which waits for all signals specified by the \p flags.
    explicit signal_waiter(thread::signal_set flags)
        : m_flags(flags)
    {
    }

    // Waits up to \p millisec milliseconds for a signal. Returns \p true,
    // if a signal has arrived and no further waiting is necessary.
    bool operator() (std::int32_t millisec)
    {
        osEvent result = osSignalWait(m_flags, millisec);
        if (result.status == osEventSignal)
        {
            m_flags = result.value.signals;
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

    thread::signal_set flags() const
    {
        return m_flags;
    }

private:
    thread::signal_set m_flags;

   signal_waiter(const signal_waiter&);
    const signal_waiter& operator= (const signal_waiter&);
};

inline
thread::signal_set wait_for_signal_flags(thread::signal_set flags)
{
    osEvent result = osSignalWait(flags, osWaitForever);
    if (result.status != osEventSignal)
    {
        ::weos::throw_exception(weos::system_error(
                                    result.status, cmsis_category()));
    }
    return result.value.signals;
}

inline
thread::signal_set try_wait_for_signal_flags(thread::signal_set flags)
{
    osEvent result = osSignalWait(flags, 0);
    /*
std::printf("try_wait_for_signal_flags:\n  status = %d   signals = %04x %04x\n",
           result.status,
           unsigned(result.value.v >> 16),
           unsigned(result.value.v & 0xFFFF));*/
    if (result.status != osOK && result.status != osEventSignal)
    {
        ::weos::throw_exception(weos::system_error(
                                    result.status, cmsis_category()));
    }
    return result.value.signals;
}

template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_signal_flags_for(
        thread::signal_set flags,
        const chrono::duration<RepT, PeriodT>& d)
{
    signal_waiter waiter(flags);
    if (chrono::detail::cmsis_wait<
            RepT, PeriodT, signal_waiter>::wait(d, waiter))
    {
        return waiter.flags();
    }

    return 0;
}

} // namespace detail

//! Puts the current thread to sleep.
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
inline
void sleep_for(const chrono::duration<RepT, PeriodT>& d) BOOST_NOEXCEPT
{
    detail::thread_sleeper sleeper;
    chrono::detail::cmsis_wait<RepT, PeriodT, detail::thread_sleeper>::wait(
                d, sleeper);
}

template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& timePoint) BOOST_NOEXCEPT;

//! Waits for any signal.
//! Blocks the current thread until one or more signal flags have been set,
//! returns these flags and resets them.
inline
thread::signal_set wait_for_any_signal()
{
    return detail::wait_for_signal_flags(0);
}

//! Checks if any signal has arrived.
//! Checks if one or more signal flags have been set for the current thread,
//! returns these flags and resets them. If no signal is set, zero
//! is returned.
inline
thread::signal_set try_wait_for_any_signal()
{
    return detail::try_wait_for_signal_flags(0);
}

//! Waits until any signal arrives or a timeout occurs.
//! Waits up to the timeout period \p d for one or more signals to be set for
//! the current thread. The set signals will be returned. If the timeout
//! expires, zero is returned.
template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_any_signal_for(
        const chrono::duration<RepT, PeriodT>& d)
{
    return detail::try_wait_for_signal_flags_for(0, d);
}

//! Waits for a set of signals.
//! Blocks the current thread until all signal flags selected by \p flags have
//! been set, returns those flags and resets them. Signal flags which are
//! not selected by \p flags are not reset.
inline
thread::signal_set wait_for_all_signals(thread::signal_set flags)
{
    WEOS_ASSERT(flags > 0 && flags < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::wait_for_signal_flags(flags);
}

//! Checks if a set of signals has been set.
//! Checks if all signal flags selected by \p flags have been set, returns
//! those flags and resets them. Signal flags which are not selected
//! through \p flags are not reset.
//! If not all signal flags specified by \p flags are set, zero is returned
//! and no flag is reset.
inline
thread::signal_set try_wait_for_all_signals(
        thread::signal_set flags)
{
    WEOS_ASSERT(flags > 0 && flags < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::try_wait_for_signal_flags(flags);
}

//! Blocks until a set of signals arrives or a timeout occurs.
//! Waits up to the timeout duration \p d for all signals specified by the
//! \p flags to be set. If these signals are set, they are returned and
//! reset. In the case of a timeout, zero is returned and the signal flags
//! are not modified.
template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_all_signals_for(
        thread::signal_set flags,
        const chrono::duration<RepT, PeriodT>& d)
{
    WEOS_ASSERT(flags > 0 && flags < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::try_wait_for_signal_flags_for(flags, d);
}

} // namespace this_thread
} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
