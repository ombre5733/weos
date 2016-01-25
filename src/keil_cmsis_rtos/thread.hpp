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
#include "_thread_detail.hpp"
#include "_sleep.hpp"

#include <cstddef>
#include <cstdint>
#include <new>


WEOS_BEGIN_NAMESPACE

class thread;

namespace expert
{
class thread_info;

bool set_stack_allocation_enabled(bool enable);
std::size_t set_default_stack_size(std::size_t size);

} // namespace expert

namespace weos_detail
{
class thread_id;
} // namespace weos_detail

namespace this_thread
{
weos_detail::thread_id get_id();
} // namespace this_thread

// ----=====================================================================----
//     thread_id
// ----=====================================================================----

namespace weos_detail
{

//! A representation of a thread identifier.
//! This class is a wrapper around a thread identifier. It has a small
//! memory footprint such that it is inexpensive to pass copies around.
class thread_id
{
public:
    thread_id() noexcept
        : m_id(0)
    {
    }

    //! Compares two thread ids for equality.
    //! Returns \p true, if \p lhs and \p rhs are equal.
    friend
    bool operator==(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id == rhs.m_id;
    }

    //! Compares two thread ids for inequality.
    //! Returns \p true, if \p lhs and \p rhs are not equal.
    friend
    bool operator!=(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id != rhs.m_id;
    }

    //! Less-than comparison for thread ids.
    //! Returns \p true, if \p lhs is less than \p rhs.
    friend
    bool operator<(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id < rhs.m_id;
    }

    //! Less-than or equal comparison for thread ids.
    //! Returns \p true, if \p lhs is less than or equal to \p rhs.
    friend
    bool operator<=(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id <= rhs.m_id;
    }

    //! Greater-than comparison for thread ids.
    //! Returns \p true, if \p lhs is greater than \p rhs.
    friend
    bool operator>(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id > rhs.m_id;
    }

    //! Greater-than or equal comparison for thread ids.
    //! Returns \p true, if \p lhs is greater than or equal to \p rhs.
    friend
    bool operator>=(thread_id lhs, thread_id rhs) noexcept
    {
        return lhs.m_id >= rhs.m_id;
    }

private:
    osThreadId m_id;

    explicit
    thread_id(osThreadId _id) noexcept
        : m_id(_id)
    {
    }

    friend class WEOS_NAMESPACE::thread;
    friend class WEOS_NAMESPACE::expert::thread_info;
    friend thread_id WEOS_NAMESPACE::this_thread::get_id();
};

} // namespace weos_detail

// ----=====================================================================----
//     thread_info
// ----=====================================================================----

namespace weos_detail
{
class SharedThreadStateBase;
} // namespace weos_detail

namespace expert
{

class thread_info
{
public:
    thread_info(const thread_info&) = default;
    thread_info& operator=(const thread_info&) = default;

    const char* get_name() const noexcept;
    void* get_stack_begin() const noexcept;
    std::size_t get_stack_size() const noexcept;
    std::size_t get_used_stack() const noexcept;
    weos_detail::thread_id get_id() const noexcept;

private:
    thread_info(const weos_detail::SharedThreadStateBase* state) noexcept
        : m_state(state),
          m_usedStack(std::size_t(-1))
    {
    }

    const weos_detail::SharedThreadStateBase* m_state;
    mutable std::size_t m_usedStack;

    friend class weos_detail::SharedThreadStateBase;
};

//! Loops over all existing threads and executes the function \p f on it.
//! The loop executes in a privileged (interrupt) context. This means that
//! locking a mutex within \p f is not possible, for example.
void for_each_thread(function<bool(thread_info)> f);

} // namespace expert

// ----=====================================================================----
//     SharedThreadState
// ----=====================================================================----

namespace weos_detail
{

// Data which is shared between the threaded function and the thread handle.
struct SharedThreadStateBase
{
    SharedThreadStateBase(const ThreadProperties& props, void* ownedStack) noexcept;

    virtual
    ~SharedThreadStateBase() {}

    SharedThreadStateBase(const SharedThreadStateBase&) = delete;
    SharedThreadStateBase& operator=(const SharedThreadStateBase&) = delete;

    // Destroys and deallocates this shared data.
    void destroy() noexcept;

    virtual
    void execute() = 0;

    expert::thread_info info() const noexcept
    {
        return expert::thread_info(this);
    }



    // This semaphore is increased by the threaded function when it's
    // execution finishes. thread::join() can block on it.
    semaphore m_finished;

    // This semaphore is increased by the thread handle when join() or
    // detach() has been called. The threaded function blocks on it
    // in order to keep the thread alive (e.g. for setting signals).
    semaphore m_joinedOrDetached;

    // The native thread id.
    osThreadId m_threadId;

    // The number of references to this shared data. Is initialized to 1.
    atomic_int m_referenceCount;

    // Points to the next state in the linked list.
    SharedThreadStateBase* m_next;

    // Pointer to the stack which is owned by the state.
    void* m_ownedStack;


    // Thread attributes
    const char* m_name;
    void* m_initialStackBase;
    void* m_stackBegin;
    std::size_t m_stackSize;
};

struct SharedThreadStateDeleter
{
    void operator()(SharedThreadStateBase* data) noexcept
    {
        data->destroy();
    }
};

template <typename TF, typename... TArgs>
class SharedThreadState : public SharedThreadStateBase
{
public:
    SharedThreadState(const ThreadProperties& props, void* ownedStack,
                      TF&& f, TArgs&&... args)
        : SharedThreadStateBase(props, ownedStack),
          m_fun(WEOS_NAMESPACE::move(f), WEOS_NAMESPACE::move(args)...)
    {
    }

    virtual
    void execute() override
    {
        typedef typename weos_detail::make_tuple_indices<
                1 + sizeof...(TArgs), 1>::type indices_type;
        return doExecute(indices_type());
    }

private:
    tuple<TF, TArgs...> m_fun;

    template <std::size_t... TIndices>
    void doExecute(weos_detail::TupleIndices<TIndices...>)
    {
        invoke(WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<0>(m_fun)),
               WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<TIndices>(m_fun))...);
    }
};

} // namespace weos_detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

//! A thread handle.
class thread
{
public:
    //! The type of the native thread handle.
    typedef thread* native_handle_type;

    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    using id = weos_detail::thread_id;

    using attributes = thread_attributes;

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
              typename = typename enable_if<
                  !is_same<typename decay<F>::type, thread>::value &&
                  !is_same<typename decay<F>::type, thread_attributes>::value &&
                  !is_same<typename decay<F>::type, weos_detail::ThreadProperties>::value
              >::type>
    explicit
    thread(F&& f, TArgs&&... args)
        : m_data(nullptr)
    {
        weos_detail::ThreadProperties props;
        create(props,
               WEOS_NAMESPACE::forward<F>(f),
               WEOS_NAMESPACE::forward<TArgs>(args)...);
    }

    template <typename F, typename... TArgs>
    thread(const thread_attributes& attrs,
           F&& f, TArgs&&... args)
        : m_data(nullptr)
    {
        weos_detail::ThreadProperties props(attrs);
        create(props,
               WEOS_NAMESPACE::forward<F>(f),
               WEOS_NAMESPACE::forward<TArgs>(args)...);
    }

    template <typename F, typename... TArgs>
    thread(weos_detail::ThreadProperties& props,
           F&& f, TArgs&&... args)
        : m_data(nullptr)
    {
        create(props,
               WEOS_NAMESPACE::forward<F>(f),
               WEOS_NAMESPACE::forward<TArgs>(args)...);
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
    static constexpr
    int signals_count() noexcept
    {
        return osFeature_Signals;
    }

    //! Returns a signal set with all flags being set.
    static constexpr
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
    weos_detail::SharedThreadStateBase* m_data;

    template <typename F, typename... TArgs>
    void create(weos_detail::ThreadProperties& props,
                F&& f, TArgs&&... args)
    {
        using namespace weos_detail;

        using shared_state_type = SharedThreadState<
            typename decay<F>::type, typename decay<TArgs>::type...>;
        static constexpr size_t alignment = alignment_of<shared_state_type>::value;
        static constexpr size_t size = sizeof(shared_state_type);

        auto deleter = props.allocate();

        if (!props.align(alignment, size))
        {
            WEOS_THROW_SYSTEM_ERROR(
                        errc::not_enough_memory,
                        "thread::create: stack size is too small");
        }

        unique_ptr<shared_state_type, SharedThreadStateDeleter> state(
            ::new (props.m_stackBegin) shared_state_type(
                        props, deleter.owned_stack(),
                        decay_copy(WEOS_NAMESPACE::forward<F>(f)),
                        decay_copy(WEOS_NAMESPACE::forward<TArgs>(args))...));
        deleter.release(); // managed by 'state' from now on

        props.offset_by(size);

        do_create(props, state.get());
        m_data = state.release();
    }

    void do_create(weos_detail::ThreadProperties& props,
                   weos_detail::SharedThreadStateBase* state);
};

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
