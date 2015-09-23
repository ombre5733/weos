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
#include "../memory.hpp"
#include "../semaphore.hpp"
#include "../system_error.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"
#include "_sleep.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

// ----=====================================================================----
//     SharedThreadData
// ----=====================================================================----

//! Data which is shared between the threaded function and the thread handle.
struct SharedThreadData
{
    //! Increases the reference counter by one.
    void addReferenceCount() noexcept
    {
        ++m_referenceCount;
    }

    //! Decreases the reference counter by one. If the reference counter reaches
    //! zero, this object is destructed and returned to the pool.
    void decReferenceCount() noexcept;

    //! Allocates a ThreadData object from the global pool. An exception is
    //! thrown if the pool is empty.
    static SharedThreadData* allocate();



    //! The bound function which will be called in the new thread.
    function<void()> m_threadedFunction;

    //! This semaphore is increased by the threaded function when it's
    //! execution finishes. thread::join() can block on it.
    semaphore m_finished;

    //! This semaphore is increased by the thread handle when join() or
    //! detach() has been called. The threaded function blocks on it
    //! in order to keep the thread alive (e.g. for setting signals).
    semaphore m_joinedOrDetached;

    //! The native thread id.
    osThreadId m_threadId;

    SharedThreadData(const SharedThreadData&) = delete;
    const SharedThreadData& operator= (const SharedThreadData&) = delete;

private:
    //! The number of references to this shared data.
    atomic_int m_referenceCount;

    //! Creates the shared thread data.
    SharedThreadData() noexcept;
};

struct SharedThreadDataDeleter
{
    void operator()(SharedThreadData* data) noexcept
    {
        data->decReferenceCount();
    }
};

} // namespace weos_detail

//! A thread handle.
class thread
{
public:
    //! The type of the native thread handle.
    typedef thread* native_handle_type;

    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    class id
    {
    public:
        //! \cond
        explicit id(osThreadId _id) noexcept
            : m_id(_id)
        {
        }
        //! \endcond

        id() noexcept
            : m_id(0)
        {
        }

    private:
        osThreadId m_id;

        friend bool operator== (id lhs, id rhs) noexcept;
        friend bool operator!= (id lhs, id rhs) noexcept;
        friend bool operator< (id lhs, id rhs) noexcept;
        friend bool operator<= (id lhs, id rhs) noexcept;
        friend bool operator> (id lhs, id rhs) noexcept;
        friend bool operator>= (id lhs, id rhs) noexcept;

        friend class thread;
    };

    // -------------------------------------------------------------------------
    // Attributes
    // -------------------------------------------------------------------------

    //! The thread attributes.
    class attributes
    {
    public:
        //! An enumeration of thread priorities.
        WEOS_SCOPED_ENUM_BEGIN(priority)
        {
            idle = osPriorityIdle,
            low = osPriorityLow,
            belowNormal = osPriorityBelowNormal,
            normal = osPriorityNormal,
            aboveNormal = osPriorityAboveNormal,
            high = osPriorityHigh,
            realtime = osPriorityRealtime
        };
        WEOS_SCOPED_ENUM_END(priority)

        //! Creates default thread attributes.
        attributes()
            : m_priority(priority::normal),
              m_customStackSize(0),
              m_customStack(nullptr),
              m_name("")
        {
        }

        //! Creates thread attributes.
        //!
        //! Creates thread attributes from a priority \p prio and a \p stack.
        template <typename T>
        attributes(priority prio, T& stack)
            : m_priority(prio),
              m_customStackSize(sizeof(T)),
              m_customStack(&stack),
              m_name("")
        {
            static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
        }

        attributes(const attributes&) = default;
        attributes& operator=(const attributes&) = default;

        //! Sets the name.
        //! Sets the name to \p name. The default is the empty string.
        attributes& setName(const char* name)
        {
            m_name = name;
            return *this;
        }

        //! Sets the priority.
        //! Sets the thread priority to \p prio.
        //!
        //! The default value is priority::normal.
        attributes& setPriority(priority prio)
        {
            m_priority = prio;
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
            static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
            m_customStack = &stack;
            m_customStackSize = sizeof(T);
            return *this;
        }

        //! Returns the start of the stack.
        void* stackBegin() const
        {
            return m_customStack;
        }

        //! Returns the size of the stack.
        std::size_t stackSize() const
        {
            return m_customStackSize;
        }

    private:
        //! The thread's priority.
        priority m_priority;
        //! The size of the custom stack.
        std::size_t m_customStackSize;
        //! A pointer to the custom stack.
        void* m_customStack;
        //! The thread's name.
        const char* m_name;

        friend class thread;
    };

    // -------------------------------------------------------------------------
    // thread
    // -------------------------------------------------------------------------

    //! Creates a thread handle without a thread.
    //! Creates a thread handle which is not associated with any thread. The
    //! new thread handle is not joinable.
    thread() noexcept
        : m_data(nullptr)
    {
    }

    template <typename F, typename... TArgs,
              typename _ = typename enable_if<!is_same<typename decay<F>::type, thread>::value &&
                                              !is_same<typename decay<F>::type, attributes>::value>::type>
    explicit
    thread(F&& f, TArgs&&... args)
        : m_data(weos_detail::SharedThreadData::allocate())
    {
        unique_ptr<weos_detail::SharedThreadData,
                   weos_detail::SharedThreadDataDeleter> data(m_data);
        m_data->m_threadedFunction = bind(WEOS_NAMESPACE::forward<F>(f),
                                          WEOS_NAMESPACE::forward<TArgs>(args)...);

        invoke(attributes());
        data.release();
    }

    template <typename F, typename... TArgs>
    thread(const attributes& attrs,
           F&& f, TArgs&&... args)
        : m_data(weos_detail::SharedThreadData::allocate())
    {
        unique_ptr<weos_detail::SharedThreadData,
                   weos_detail::SharedThreadDataDeleter> data(m_data);
        m_data->m_threadedFunction = bind(WEOS_NAMESPACE::forward<F>(f),
                                          WEOS_NAMESPACE::forward<TArgs>(args)...);

        invoke(attrs);
        data.release();
    }

    //! Move constructor.
    //! Constructs a thread by moving from the \p other thread.
    thread(thread&& other) noexcept
        : m_data(other.m_data)
    {
        other.m_data = nullptr;
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

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    //! Move assignment.
    //! Move-assigns the \p other thread to this thread.
    thread& operator=(thread&& other) noexcept
    {
        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    //! Separates the executing thread from this thread handle.
    void detach();

    //! Returns the id of the thread.
    id get_id() const noexcept
    {
        if (m_data)
            return id(m_data->m_threadId);
        else
            return id();
    }

    //! Blocks until the associated thread has been finished.
    //! Blocks the calling thread until the thread which is associated with
    //! this thread handle has been finished.
    void join();

    //! Checks if the thread is joinable.
    //! Returns \p true, if the thread is joinable.
    //! \note If a thread is joinable, either join() or detach() must be
    //! called before the destructor is executed.
    inline
    bool joinable() const noexcept
    {
        return m_data != nullptr;
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    inline
    static unsigned hardware_concurrency() noexcept
    {
        return 1;
    }

    //! Returns the native thread handle.
    native_handle_type native_handle() noexcept
    {
        return this;
    }

    // -------------------------------------------------------------------------
    // Signals
    // -------------------------------------------------------------------------

    static_assert(osFeature_Signals > 0 && osFeature_Signals <= 16,
                  "The wrapper supports only up to 16 signals.");

    //! Represents a set of signal flags.
    typedef std::uint16_t signal_set;

    //! Returns the number of signals in a set.
    constexpr static
    int signals_count() noexcept
    {
        return osFeature_Signals;
    }

    //! Returns a signal set with all flags being set.
    constexpr static
    signal_set all_signals() noexcept
    {
        return (std::uint32_t(1) << osFeature_Signals) - 1;
    }

    //! Clears a set of signals.
    //! Clears the signals which are specified by the \p flags.
    void clear_signals(signal_set flags);

    //! Sets a set of signals.
    //! Sets the signals which are specified by the \p flags.
    void set_signals(signal_set flags);

private:
    //! The thread-data which is shared by this class and the invoker
    //! function.
    weos_detail::SharedThreadData* m_data;

    //! Invokes the function which is stored in the shared data in a new
    //! thread which is created with the attributes \p attrs.
    void invoke(const attributes& attrs);
};

//! Compares two thread ids for equality.
//! Returns \p true, if \p lhs and \p rhs are equal.
inline
bool operator== (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id == rhs.m_id;
}

//! Compares two thread ids for inequality.
//! Returns \p true, if \p lhs and \p rhs are not equal.
inline
bool operator!= (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id != rhs.m_id;
}

//! Less-than comparison for thread ids.
//! Returns \p true, if \p lhs is less than \p rhs.
inline
bool operator< (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id < rhs.m_id;
}

//! Less-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is less than or equal to \p rhs.
inline
bool operator<= (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id <= rhs.m_id;
}

//! Greater-than comparison for thread ids.
//! Returns \p true, if \p lhs is greater than \p rhs.
inline
bool operator> (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id > rhs.m_id;
}

//! Greater-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is greater than or equal to \p rhs.
inline
bool operator>= (thread::id lhs, thread::id rhs) noexcept
{
    return lhs.m_id >= rhs.m_id;
}

// ----=====================================================================----
//     this_thread
// ----=====================================================================----

namespace this_thread
{

//! Returns the id of the current thread.
inline
thread::id get_id()
{
    return thread::id(osThreadGetId());
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
template <typename TRep, typename TPeriod>
inline
void sleep_for(const chrono::duration<TRep, TPeriod>& d)
{
    using namespace chrono;
    if (d > d.zero())
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
template <typename TClock, typename TDuration>
inline
void sleep_until(const chrono::time_point<TClock, TDuration>& time)
{
    weos_detail::sleep_until(time);
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

template <typename TClock, typename TDuration>
thread::signal_set try_wait_for_any_signal_until(
            const chrono::time_point<TClock, TDuration>& time)
{
    using namespace chrono;

    bool timeout = false;
    while (!timeout)
    {
        osEvent result;
        auto remainingSpan = time - TClock::now();
        if (remainingSpan <= TDuration::zero())
        {
            result = osSignalWait(0, 0);
            timeout = true;
        }
        else
        {
            milliseconds converted = duration_cast<milliseconds>(remainingSpan);
            if (converted < milliseconds(1))
                converted = milliseconds(1);
            else if (converted > milliseconds(0xFFFE))
                converted = milliseconds(0xFFFE);
            result = osSignalWait(0, converted.count());
        }

        if (result.status == osEventSignal)
            return result.value.signals;

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_any_signal_until failed");
        }
    }

    return 0;
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

template <typename TClock, typename TDuration>
bool try_wait_for_all_signals_until(
            thread::signal_set flags,
            const chrono::time_point<TClock, TDuration>& time)
{
    using namespace chrono;
    WEOS_ASSERT(flags > 0 && flags <= thread::all_signals());

    bool timeout = false;
    while (!timeout)
    {
        osEvent result;
        auto remainingSpan = time - TClock::now();
        if (remainingSpan <= TDuration::zero())
        {
            result = osSignalWait(flags, 0);
            timeout = true;
        }
        else
        {
            milliseconds converted = duration_cast<milliseconds>(remainingSpan);
            if (converted < milliseconds(1))
                converted = milliseconds(1);
            else if (converted > milliseconds(0xFFFE))
                converted = milliseconds(0xFFFE);
            result = osSignalWait(flags, converted.count());
        }

        if (result.status == osEventSignal)
            return true;

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_all_signals_until failed");
        }
    }

    return false;
}

} // namespace this_thread

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_HPP
