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

#ifndef WEOS_KEIL_RL_RTX_THREAD_HPP
#define WEOS_KEIL_RL_RTX_THREAD_HPP

#include "core.hpp"

#include "../atomic.hpp"
#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "system_error.hpp"
#include "../objectpool.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

class thread;

namespace detail
{

//! Traits for native threads.
struct native_thread_traits
{
    // The native type for a thread handle. This is a dummy type as CMSIS
    // makes no use of it.
    typedef void* thread_handle_type;

    // The native type for a thread ID.
    typedef OS_TID thread_id_type;

    // Represents a set of signals.
    typedef std::uint16_t signal_set;

    // The number of signals in a set.
    static const int signals_count = 16;

    // A signal set with all flags being set.
    static const signal_set all_signals = 0xFFFF;
};

} // namespace detail

WEOS_END_NAMESPACE

#include "../common/thread_detail.hpp"


WEOS_BEGIN_NAMESPACE

namespace this_thread
{

//! Returns the id of the current thread.
inline
WEOS_NAMESPACE::thread::id get_id()
{
    return WEOS_NAMESPACE::thread::id(os_tsk_self());
}

//! \brief Puts the current thread to sleep.
//!
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
void sleep_for(const chrono::duration<RepT, PeriodT>& d) WEOS_NOEXCEPT
{
    //! \todo Convert to the true amound of ticks, even if the
    //! system does not run with a 1ms tick.
    RepT ticks = chrono::duration_cast<chrono::milliseconds>(d);
    if (ticks <= 0)
        ticks = 0;
    else
        ++ticks;

    do
    {
        RepT delay = ticks;
        if (delay > 0xFFFE)
            delay = 0xFFFE;
        ticks -= delay;
        os_dly_wait(delay);
    } while (delay);
}

//! \brief Puts the current thread to sleep.
//!
//! Blocks the execution of the current thread until the given \p timePoint.
template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& timePoint) WEOS_NOEXCEPT
{
    typedef typename DurationT::rep rep_t;
    do
    {
        //! \todo Convert to the true amound of ticks, even if the
        //! system does not run with a 1ms tick.
        rep_t ticks = chrono::duration_cast<chrono::milliseconds>(
                          timePoint - ClockT::now());
        if (ticks < 0)
            ticks = 0;
        else if (ticks > 0xFFFE)
            ticks = 0xFFFE;

        os_dly_wait(ticks);

    } while (ticks > 0);
}

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    os_tsk_pass();
}

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

//! Waits until any signal arrives or a timeout occurs.
//! Waits up to the timeout period \p d for one or more signals to be set for
//! the current thread. The set signals will be returned. If the timeout
//! expires, zero is returned.
template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_any_signal_for(
            const chrono::duration<RepT, PeriodT>& d)
{
    return detail::try_wait_for_signalflags_for(0, d);
}

template <typename ClockT, typename DurationT>
inline
thread::signal_set try_wait_for_any_signal_until(
            const chrono::time_point<ClockT, DurationT>& timePoint)
{
????????????
}

//! Waits for a set of signals.
//! Blocks the current thread until all signal flags selected by \p flags have
//! been set, returns those flags and resets them. Signal flags which are
//! not selected by \p flags are not reset.
inline
thread::signal_set wait_for_all_signals(thread::signal_set flags)
{
    WEOS_ASSERT(flags > 0 && flags < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::wait_for_signalflags(flags);
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
    return detail::try_wait_for_signalflags(flags);
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
    return detail::try_wait_for_signalflags_for(flags, d);
}

} // namespace this_thread

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_RL_RTX_THREAD_HPP
