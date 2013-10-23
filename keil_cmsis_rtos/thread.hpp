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
#include "error.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "../objectpool.hpp"

#if 0
extern "C" {
#include "../3rdparty/keil_cmsis_rtos/SRC/rt_Task.h"
}
#include "../3rdparty/keil_cmsis_rtos/SRC/rt_TypeDef.h"
#endif

#include <boost/config.hpp>
#include <boost/utility.hpp>

#include <cstdint>
#include <exception>
#include <utility>

//! A helper function to invoke a thread.
//! A CMSIS thread is a C function taking a <tt>const void*</tt> argument. This
//! helper function adhers to this specification. The \p arg is a pointer to
//! a weos::ThreadData object which contains thread-specifica data such as
//! the actual function to start.
extern "C" void weos_threadInvoker(const void* arg);

namespace weos
{
class thread;

namespace detail
{
//! Data which is shared between the thread and its handle.
class ThreadData : boost::noncopyable
{
    typedef object_pool<ThreadData, WEOS_MAX_NUM_CONCURRENT_THREADS,
                        mutex> pool_t;

public:
    ThreadData();

    //! Decreases the reference counter by one. If the reference counter reaches
    //! zero, this ThreadData is returned to the pool.
    void deref();
    //! Increases the reference counter by one.
    void ref();

    //! Returns the global pool from which thread-data objects are allocated.
    static pool_t& pool();

private:
    //! The function to execute in the new thread.
    void (*m_function)(void*);
    //! The argument which is supplied to the threaded function.
    void* m_arg;

    //! This semaphore is increased by the thread when it's execution finishes.
    semaphore m_finished;
    volatile int m_referenceCount;
    //! The system-specific thread id.
    osThreadId m_threadId;

    friend class ::weos::thread;
    friend void ::weos_threadInvoker(const void* m_arg);
};

} // namespace detail

//! A thread.
class thread : boost::noncopyable
{
public:
    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    class id
    {
    public:
        id() BOOST_NOEXCEPT
            : m_id(0)
        {
        }

        explicit id(osThreadId _id) BOOST_NOEXCEPT
            : m_id(_id)
        {
        }

    private:
        friend bool operator== (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator!= (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator< (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator<= (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator> (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator>= (id lhs, id rhs) BOOST_NOEXCEPT;

        osThreadId m_id;
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

        //! Creates stack attributes.
        attributes()
            : m_priority(Normal),
              m_customStackSize(0),
              m_customStack(0)
        {
        }

        //! Provides a custom stack.
        //! Makes the thread use the memory pointed to by \p stack whose size
        //! in bytes is passed in \p stackSize rather than the default stack.
        //!
        //! The default is a null-pointer for the stack and zero for its size.
        void setCustomStack(void* stack, std::uint32_t stackSize)
        {
            m_customStack = stack;
            m_customStackSize = stackSize;
        }

        //! Sets the priority.
        //! Sets the thread priority to \p priority.
        //!
        //! The default value is Priority::Normal.
        void setPriority(Priority priority)
        {
            m_priority = priority;
        }

    private:
        //! The thread's priority.
        Priority m_priority;
        //! The size of the custom stack.
        std::uint32_t m_customStackSize;
        //! A pointer to the custom stack.
        void* m_customStack;

        friend class thread;
    };

    //! Creates a thread handle without a thread.
    //! Creates a thread handle which is not associated with any thread. The
    //! new thread handle is not joinable.
    thread() BOOST_NOEXCEPT
        : m_data(0)
    {
    }

    //! Creates a thread.
    //! Starts the function \p fun with the argument \p arg in a new thread.
    thread(void (*fun)(void*), void* arg)
        : m_data(0)
    {
        attributes attrs;
        invokeWithDefaultStack(attrs, fun, arg);
    }

    //! Creates a thread.
    //! Starts the function \p fun with the argument \p arg in a new thread.
    //! The thread attributes are passed in \p attrs.
    thread(const attributes& attrs, void (*fun)(void*), void* arg)
        : m_data(0)
    {
        if (attrs.m_customStack || attrs.m_customStackSize)
            invokeWithCustomStack(attrs, fun, arg);
        else
            invokeWithDefaultStack(attrs, fun, arg);
    }

    //! Destroys the thread handle.
    //! Destroys this thread handle.
    //! \note If the thread handle is still associated with a joinable thread,
    //! its destruction will call std::terminate(). It is mandatory to either
    //! call join() or detach().
    ~thread()
    {
        if (joinable())
            std::terminate();
    }

    //! Separates the executing thread from this thread handle.
    void detach()
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }
        m_data->deref();
        m_data = 0;
    }

    //! Returns the id of the thread.
    id get_id() const BOOST_NOEXCEPT
    {
        if (m_data)
            return id(m_data->m_threadId);
        else
            return id();
    }

    //! Blocks until this thread finishes.
    //! Blocks the calling thread until this thread finishes.
    void join()
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }
        m_data->m_finished.wait();
        // The thread data is not needed any longer.
        m_data->deref();
        m_data = 0;
    }

    //! Checks if the thread is joinable.
    //! Returns \p true, if the thread is joinable.
    //! \note If a thread is joinable, either join() or detach() must be
    //! called before the destructor is executed.
    bool joinable() BOOST_NOEXCEPT
    {
        return m_data != 0;
    }

    void clear_signal(std::uint32_t mask)
    {
        WEOS_ASSERT(m_data);
        std::int32_t result = osSignalClear(m_data->m_threadId, mask);
        WEOS_ASSERT(result >= 0);
    }

    void set_signal(std::uint32_t mask)
    {
        WEOS_ASSERT(m_data);
        std::int32_t result = osSignalSet(m_data->m_threadId, mask);
        WEOS_ASSERT(result >= 0);
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    static unsigned hardware_concurrency() BOOST_NOEXCEPT
    {
        return 1;
    }

protected:
    //! Invokes the function \p fun with the argument \p arg. The thread
    //! attributes are passed in \p attrs. This method may only be called
    //! if \p attrs contains a valid custom stack configuration.
    void invokeWithCustomStack(const attributes& attrs,
                               void (*fun)(void*), void* arg);

    //! Invokes the function \p fun with the argument \p arg. The thread
    //! attributes are passed in \p attrs. This method may only be called
    //! if \p attrs does not contain a custom stack configuration.
    void invokeWithDefaultStack(const attributes& attrs,
                                void (*fun)(void*), void* arg);

private:
    //! The thread-data which is shared by this class and the invoker
    //! function.
    detail::ThreadData* m_data;
};

//! A thread with a custom stack.
//! The custom_stack_thread is a convenience class for creating a thread with a
//! custom stack. The memory for the stack is held statically by the object.
//! Its size in bytes is determined at compile-time by the template
//! parameter \p TStackSize.
template <unsigned TStackSize>
class custom_stack_thread : public thread
{
    // The stack must be able to hold the registers R0-R13.
    BOOST_STATIC_ASSERT(TStackSize >= 14*4);

public:
    //! Creates a thread with a custom stack.
    //! Starts the function \p fun with the argument \p arg in a new thread.
    custom_stack_thread(void (*fun)(void*), void* arg)
    {
        thread::attributes attrs;
        attrs.setCustomStack(m_stack.address(), TStackSize);
        this->invokeWithCustomStack(attrs, fun, arg);
    }

    //! Creates a thread with a custom stack and a custom priority.
    //! Runs the function \p fun with the argument \p arg in a new thread,
    //! which has a custom stack and a user-defined priority of \p priority.
    custom_stack_thread(thread::attributes::Priority priority,
                        void (*fun)(void*), void* arg)
    {
        thread::attributes attrs;
        attrs.setCustomStack(m_stack.address(), TStackSize);
        attrs.setPriority(priority);
        this->invokeWithCustomStack(attrs, fun, arg);
    }

private:
    //! The custom stack.
    typename ::boost::aligned_storage<TStackSize>::type m_stack;
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
weos::thread::id get_id()
{
    return weos::thread::id(osThreadGetId());
}

namespace detail
{
// A helper to put a thread to sleep.
struct thread_sleeper
{
    // Waits for millisec milliseconds. The method always returns false because
    // we cannot shortcut a delay.
    bool operator() (std::int32_t millisec) const
    {
        osStatus status = osDelay(millisec);
        WEOS_ASSERT(   (millisec == 0 && status == osOK)
                    || (millisec != 0 && status == osEventTimeout));
        return false;
    }
};

// A helper to wait for a signal.
struct signal_waiter
{
    // Creates an object which waits for a signal specified by the \p mask.
    signal_waiter(std::uint32_t mask)
        : m_mask(mask)
    {
    }

    // Waits up to \p millisec milliseconds for a signal. Returns \p true,
    // if a signal has arrived and no further waiting is necessary.
    bool operator() (std::int32_t millisec) const
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
    mutable std::uint32_t m_mask;
};

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

namespace detail
{

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
std::pair<bool, std::uint32_t> wait_for_signal_for(
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

inline
std::uint32_t wait_for_signal()
{
    return detail::wait_for_signal(0);
}

inline
std::pair<bool, std::uint32_t> try_wait_for_signal()
{
    return detail::try_wait_for_signal(0);
}

template <typename RepT, typename PeriodT>
std::pair<bool, std::uint32_t> wait_for_signal_for(
        const chrono::duration<RepT, PeriodT>& d)
{
    return detail::wait_for_signal_for(0, d);
}

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
std::pair<bool, std::uint32_t> wait_for_signal_for(
        std::uint32_t mask, const chrono::duration<RepT, PeriodT>& d)
{
    WEOS_ASSERT(mask > 0 && mask < (std::uint32_t(1) << (osFeature_Signals)));
    return detail::wait_for_signal_for(mask, d);
    detail::signal_waiter waiter(mask);
    if (chrono::detail::cmsis_wait<
            RepT, PeriodT, detail::signal_waiter>::wait(d, waiter))
    {
        return std::pair<bool, std::uint32_t>(true, waiter.mask());
    }

    return std::pair<bool, std::uint32_t>(false, 0);
}

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    osStatus status = osThreadYield();
    WEOS_ASSERT(status == osOK);
}

} // namespace this_thread

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
