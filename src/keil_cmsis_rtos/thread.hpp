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

#ifndef WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
#define WEOS_KEIL_CMSIS_RTOS_THREAD_HPP

#include "core.hpp"

#include "../atomic.hpp"
#include "../functional.hpp"
#include "../chrono.hpp"
#include "../mutex.hpp"
#include "../semaphore.hpp"
#include "../system_error.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

class thread;

namespace weos_detail
{

//! Traits for native threads.
struct native_thread_traits
{
    // The native type for a thread handle. This is a dummy type as CMSIS
    // makes no use of it.
    typedef void* thread_handle_type;

    // The native type for a thread ID.
    typedef osThreadId thread_id_type;

    static_assert(osFeature_Signals > 0 && osFeature_Signals <= 16,
                  "The wrapper supports only up 16 signals.");

    // Represents a set of signals.
    typedef std::uint16_t signal_set;

    // The number of signals in a set.
    static const int signals_count = osFeature_Signals;

    // A signal set with all flags being set.
    static const signal_set all_signals
                = (std::uint32_t(1) << osFeature_Signals) - 1;


    //! A thread id.
    class id
    {
    public:
        id() noexcept
            : m_id(0)
        {
        }

        explicit id(thread_id_type _id) noexcept
            : m_id(_id)
        {
        }

        thread_id_type m_id;
    };


    //! Thread attributes.
    class attributes
    {
    public:
        //! An enumeration of thread priorities.
        enum Priority
        {
            Idle = osPriorityIdle,
            Low = osPriorityLow,
            BelowNormal = osPriorityBelowNormal,
            Normal = osPriorityNormal,
            AboveNormal = osPriorityAboveNormal,
            High = osPriorityHigh,
            Realtime = osPriorityRealtime,
            Error = osPriorityError
        };

        //! Creates default thread attributes.
        attributes()
            : m_priority(Normal),
              m_customStackSize(0),
              m_customStack(0)
        {
        }

        //! Creates thread attributes.
        //!
        //! Creates thread attributes from a given \p priority and a \p stack.
        template <typename T>
        attributes(Priority priority, T& stack)
            : m_priority(priority),
              m_customStackSize(sizeof(T)),
              m_customStack(&stack)
        {
            static_assert(sizeof(T) >= 4*16, "The stack is too small.");
        }

        //! Sets the priority.
        //! Sets the thread priority to \p priority.
        //!
        //! The default value is Priority::Normal.
        attributes& setPriority(Priority priority)
        {
            m_priority = priority;
            return *this;
        }

        //! Provides a custom stack.
        //! Makes the thread use the memory pointed to by \p stack whose size
        //! in bytes is passed in \p stackSize rather than the default stack.
        //!
        //! The default is a null-pointer for the stack and zero for its size.
        attributes& setStack(void* stack, std::size_t stackSize)
        {
            m_customStack = stack;
            m_customStackSize = stackSize;
            return *this;
        }

        //! Provides a custom stack.
        //!
        //! Sets the thread's stack to \p stack.
        template <typename T>
        attributes& setStack(T& stack)
        {
            static_assert(sizeof(T) >= 4*16, "The stack is too small.");
            m_customStack = &stack;
            m_customStackSize = sizeof(T);
            return *this;
        }

    private:
        //! The thread's priority.
        Priority m_priority;
        //! The size of the custom stack.
        std::size_t m_customStackSize;
        //! A pointer to the custom stack.
        void* m_customStack;

        friend class WEOS_NAMESPACE::thread;
    };
};

} // namespace weos_detail

WEOS_END_NAMESPACE


#include "../common/thread_detail.hpp"


WEOS_BEGIN_NAMESPACE

namespace this_thread
{

//! Returns the id of the current thread.
inline
WEOS_NAMESPACE::thread::id get_id()
{
    return WEOS_NAMESPACE::thread::id(osThreadGetId());
}

//! \cond
//! Puts the current thread to sleep.
//!
//! This is an overload if the duration is specified in milliseconds.
void sleep_for(chrono::milliseconds ms);
//! \endcond

//! \brief Puts the current thread to sleep.
//!
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
inline
void sleep_for(const chrono::duration<RepT, PeriodT>& d)
{
    using namespace chrono;
    if (d > duration<RepT, PeriodT>::zero())
    {
        milliseconds converted = duration_cast<milliseconds>(d);
        if (converted < d)
            ++converted;
        sleep_for(converted);
    }
}

//! \brief Puts the current thread to sleep.
//!
//! Blocks the execution of the current thread until the given \p time point.
template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& time) noexcept
{
    typedef typename WEOS_NAMESPACE::common_type<
                         typename ClockT::duration,
                         DurationT>::type difference_type;
    typedef chrono::detail::internal_time_cast<difference_type> caster;

    while (true)
    {
        typename caster::type millisecs
                = caster::convert_and_clip(time - ClockT::now());

        osStatus result = osDelay(millisecs);
        if (result != osOK && result != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "sleep_until failed");
        }

        if (millisecs == 0)
            return;
    }
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

//! Waits for any signal.
//! Blocks the current thread until one or more signal flags have been set,
//! returns these flags and resets them.
thread::signal_set wait_for_any_signal();

//! Checks if any signal has arrived.
//! Checks if one or more signal flags have been set for the current thread,
//! returns these flags and resets them. If no signal is set, zero
//! is returned.
thread::signal_set try_wait_for_any_signal();

//! \cond
//! Waits for any signal.
//!
//! This is an overload if the duration is specified in milliseconds.
thread::signal_set try_wait_for_any_signal_for(chrono::milliseconds ms);
//! \endcond

//! Waits until any signal arrives or a timeout occurs.
//! Waits up to the timeout period \p d for one or more signals to be set for
//! the current thread. The set signals will be returned. If the timeout
//! expires, zero is returned.
template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_any_signal_for(
            const chrono::duration<RepT, PeriodT>& d)
{
    using namespace chrono;

    milliseconds converted = duration_cast<milliseconds>(d);
    if (converted < d)
        ++converted;

    return try_wait_for_any_signal_for(converted);
}

template <typename ClockT, typename DurationT>
thread::signal_set try_wait_for_any_signal_until(
            const chrono::time_point<ClockT, DurationT>& time)
{
    while (1)
    {
        typedef typename WEOS_NAMESPACE::common_type<
                             typename ClockT::duration,
                             DurationT>::type difference_type;
        typedef chrono::detail::internal_time_cast<difference_type> caster;

        typename caster::type millisecs
                = caster::convert_and_clip(time - ClockT::now());

        osEvent result = osSignalWait(0, millisecs);
        if (result.status == osEventSignal)
        {
            return result.value.signals;
        }

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_any_signal_until failed");
        }

        if (millisecs == 0)
            return 0;
    }
}

//! Waits for a set of signals.
//! Blocks the current thread until all signal flags selected by \p flags have
//! been set, returns those flags and resets them. Signal flags which are
//! not selected by \p flags are not reset.
void wait_for_all_signals(thread::signal_set flags);

//! Checks if a set of signals has been set.
//! Checks if all signal flags selected by \p flags have been set, returns
//! those flags and resets them. Signal flags which are not selected
//! through \p flags are not reset.
//! If not all signal flags specified by \p flags are set, zero is returned
//! and no flag is reset.
bool try_wait_for_all_signals(thread::signal_set flags);

//! \cond
//! Waits for all specified signals.
//!
//! This is an overload if the duration is specified in milliseconds.
bool try_wait_for_all_signals_for(thread::signal_set flags,
                                  chrono::milliseconds ms);
//! \endcond

//! Blocks until a set of signals arrives or a timeout occurs.
//! Waits up to the timeout duration \p d for all signals specified by the
//! \p flags to be set. If these signals are set, they are returned and
//! reset. In the case of a timeout, zero is returned and the signal flags
//! are not modified.
template <typename RepT, typename PeriodT>
inline
bool try_wait_for_all_signals_for(thread::signal_set flags,
                                  const chrono::duration<RepT, PeriodT>& d)
{
    using namespace chrono;

    milliseconds converted = duration_cast<milliseconds>(d);
    if (converted < d)
        ++converted;

    return try_wait_for_all_signals_for(flags, converted);
}

template <typename ClockT, typename DurationT>
bool try_wait_for_all_signals_until(
            thread::signal_set flags,
            const chrono::time_point<ClockT, DurationT>& time)
{
    WEOS_ASSERT(flags > 0 && flags <= thread::all_signals());

    while (true)
    {
        typedef typename WEOS_NAMESPACE::common_type<
                             typename ClockT::duration,
                             DurationT>::type difference_type;
        typedef chrono::detail::internal_time_cast<difference_type> caster;

        typename caster::type millisecs
                = caster::convert_and_clip(time - ClockT::now());

        osEvent result = osSignalWait(flags, millisecs);
        if (result.status == osEventSignal)
        {
            return true;
        }

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_all_signals_until failed");
        }

        if (millisecs == 0)
            return false;
    }
}

} // namespace this_thread

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
