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

#include <boost/config.hpp>
#include <boost/utility.hpp>

namespace weos
{

class thread : boost::noncopyable //! \todo Or must it be copyable?
{
    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that copied can be passed around.
    class id
    {
    public:
        id() : m_id(0) {}

        id(osThreadId _id) : m_id(_id) {}

    private:
        friend bool operator== (id lhs, id rhs);
        friend bool operator!= (id lhs, id rhs);
        friend bool operator< (id lhs, id rhs);
        friend bool operator<= (id lhs, id rhs);
        friend bool operator> (id lhs, id rhs);
        friend bool operator>= (id lhs, id rhs);

        osThreadId m_id;
    };

    class attributes
    {
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

        Priority priority;
        uint32_t stackSize;
    };

    thread()
    {
    }

    thread(const attributes& attrs)
    {
    }

    thread(void (*fun)(void*), void* arg)
    {
        osThreadDef_t threadDef = { fun, DEFAULT_PRIORITY, 0, 0 };
        m_id = osThreadCreate(&threadDef, arg);
    }

    ~thread()
    {
        //! \todo Check if the thread is still running. If so, call
        //! std::terminate(). The C++11 standard does not allow to invoke
        //! the destructor when the thread is joinable()
    }

    //! Returns the id of the thread.
    id get_id() const BOOST_NOEXCEPT
    {
        return m_id;
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    static unsigned hardware_concurrency() BOOST_NOEXCEPT
    {
        return 1;
    }

private:
    id m_id;
};

//! Compares two thread ids for equality.
//! Returns \p true, if \p lhs and \p rhs are equal.
inline
bool operator== (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id == rhs.m_id;
}

//! Compares two thread ids for inequality.
//! Returns \p true, if \p lhs and \p rhs are not equal.
inline
bool operator!= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id != rhs.m_id;
}

//! Less-than comparison for thread ids.
//! Returns \p true, if \p lhs is less than \p rhs.
inline
bool operator< (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id < rhs.m_id;
}

//! Less-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is less than or equal to \p rhs.
inline
bool operator<= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id <= rhs.m_id;
}

//! Greater-than comparison for thread ids.
//! Returns \p true, if \p lhs is greater than \p rhs.
inline
bool operator> (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id > rhs.m_id;
}

//! Greater-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is greater than or equal to \p rhs.
inline
bool operator>= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id >= rhs.m_id;
}

namespace this_thread
{

//! Returns the id of the current thread.
inline
osl::thread::id get_id()
{
    return osl::thread::id(osThreadGetId());
}

namespace detail
{

class thread_sleeper
{
public:
    bool operator() (int32_t timeout)
    {
        status = osDelay(timeout);
        OSL_ASSERT(status == osEventTimeout);
    }
};

} // namespace detail

//! Puts the current thread to sleep.
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
void sleep_for(const chrono::duration<RepT, PeriodT>& d) BOOST_NOEXCEPT
{
    detail::thread_sleeper sleeper;
    chrono::detail::cmsis_wait<RepT, PeriodT, detail::thread_sleeper>(
                d, sleeper);
}

template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& timePoint) BOOST_NOEXCEPT;

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    osStatus status = osThreadYield();
    OSL_ASSERT(status == osOK);
    OSL_UNUSED(status);
}

} // namespace this_thread

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
