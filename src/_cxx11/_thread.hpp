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

#ifndef WEOS_CXX11_THREAD_HPP
#define WEOS_CXX11_THREAD_HPP

#include "_core.hpp"

#include "_thread_detail.hpp"
#include "../atomic.hpp"
#include "../condition_variable.hpp"
#include "../chrono.hpp"
#include "../functional.hpp"
#include "../memory.hpp"
#include "../mutex.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"
#include "../_common/_index_sequence.hpp"

#include <cstddef>
#include <cstdint>
#include <thread>


// ----=====================================================================----
//     forward declarations
// ----=====================================================================----

WEOS_BEGIN_NAMESPACE

class thread;

namespace expert
{
class thread_info;

bool set_stack_allocation_enabled(bool enable);
std::size_t set_default_stack_size(std::size_t size);

} // namespace expert

// ----=====================================================================----
//     thread_info
// ----=====================================================================----

namespace weos_detail
{
class SharedThreadStateBase;
} // namespace weos_detail

namespace expert
{

//! A collection of thread data.
//!
//! The thread_info contains a collection of thread data. It is generated
//! when the user iterates over the current list of threads.
class thread_info
{
public:
    thread_info(const thread_info&) = default;
    thread_info& operator=(const thread_info&) = default;

    //! Returns the name of the stack.
    const char* get_name() const noexcept;

    //! Returns a pointer to the beginning of the thread stack.
    void* get_stack_begin() const noexcept;

    //! Returns the total size of the thread stack.
    std::size_t get_stack_size() const noexcept;

    //! Returns the amount of stack space which has been used.
    std::size_t get_used_stack() const noexcept;

    //! Returns the ID of the thread.
    std::thread::id get_id() const noexcept;

    //! Returns the thread priority.
    thread_attributes::priority get_priority() const noexcept;

    //! Returns the native thread handle.
    const void* native_handle() const noexcept;

private:
    explicit
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

// A manager relating thread-ids to the shared thread state.
class SharedThreadStateManager
{
public:
    SharedThreadStateManager();

    SharedThreadStateManager(const SharedThreadStateManager&) = delete;
    const SharedThreadStateManager& operator=(const SharedThreadStateManager&) = delete;

    void add(std::thread::id id, SharedThreadStateBase* data);
    SharedThreadStateBase& find(std::thread::id id);
    void remove(std::thread::id id);

    static SharedThreadStateManager& instance();

private:
    ~SharedThreadStateManager();

    class SharedThreadStateManagerPrivate* m_pimpl;
};

// Data which is shared between the threaded function and the thread handle.
struct SharedThreadStateBase
{
    SharedThreadStateBase(const thread_attributes& attrs) noexcept;

    virtual
    ~SharedThreadStateBase() {}

    SharedThreadStateBase(const SharedThreadStateBase&) = delete;
    SharedThreadStateBase& operator=(const SharedThreadStateBase&) = delete;

    virtual
    void execute() = 0;

    expert::thread_info info() const noexcept
    {
        return expert::thread_info(this);
    }


    std::mutex m_mutex;
    std::condition_variable m_signal;

    // Set when the thread data has been registered with the manager.
    bool m_isRegistered;
    // Set when thread::join() or thread::detach() has been called.
    bool m_joinedOrDetached;

    // The signal flags.
    std::uint16_t m_signalFlags;

    // The native thread handle.
    std::thread m_thread;

    // Thread attributes
    thread_attributes m_attrs;
};

template <typename TF, typename... TArgs>
class SharedThreadState : public SharedThreadStateBase
{
public:
    SharedThreadState(const thread_attributes& attrs, TF&& f, TArgs&&... args)
        : SharedThreadStateBase(attrs),
          m_fun(std::move(f), std::move(args)...)
    {
    }

    virtual
    void execute() override
    {
        using indices_type = typename WEOS_NAMESPACE::weos_detail::makeIndexSequence<
                                 1 + sizeof...(TArgs), 1>::type;
        return doExecute(indices_type());
    }

private:
    std::tuple<TF, TArgs...> m_fun;

    template <std::size_t... TIndices>
    void doExecute(WEOS_NAMESPACE::weos_detail::IndexSequence<TIndices...>)
    {
        WEOS_NAMESPACE::weos_detail::invoke(
                    std::move(std::get<0>(m_fun)),
                    std::move(std::get<TIndices>(m_fun))...);
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
    using native_handle_type = thread*;

    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    using id = std::thread::id;

    using attributes = thread_attributes;

    // -------------------------------------------------------------------------
    // thread
    // -------------------------------------------------------------------------

    //! Creates a thread handle without a thread.
    //! Creates a thread handle which is not associated with any thread. The
    //! new thread handle is not joinable.
    thread() noexcept
    {
    }

    template <typename F, typename... TArgs,
              typename = typename enable_if<
                  !is_same<typename decay<F>::type, thread>::value &&
                  !is_same<typename decay<F>::type, thread_attributes>::value
              >::type>
    explicit
    thread(F&& f, TArgs&&... args)
    {
        create(thread_attributes(),
               std::forward<F>(f), std::forward<TArgs>(args)...);
    }

    template <typename F, typename... TArgs>
    thread(const thread_attributes& attrs,
           F&& f, TArgs&&... args)
    {
        create(attrs, std::forward<F>(f), std::forward<TArgs>(args)...);
    }

    //! Move constructor.
    //! Constructs a thread by moving from the \p other thread.
    thread(thread&& other) noexcept;

    //! Destroys the thread handle.
    //! Destroys this thread handle.
    //! \note If the thread handle is still associated with a joinable thread,
    //! its destruction will call std::terminate(). It is mandatory to either
    //! call join() or detach() before destructing a thread.
    ~thread();

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    //! Move assignment.
    //! Move-assigns the \p other thread to this thread.
    thread& operator=(thread&& other) noexcept;

    //! Separates the executing thread from this thread handle.
    void detach();

    //! Returns the id of the thread.
    id get_id() const noexcept;

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

    //! Represents a set of signal flags.
    using signal_set = std::uint16_t;

    //! Returns the number of signals in a set.
    static constexpr
    int signals_count() noexcept
    {
        return 16;
    }

    //! Returns a signal set with all flags being set.
    static constexpr
    signal_set all_signals() noexcept
    {
        return 0xFFFF;
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
    std::shared_ptr<weos_detail::SharedThreadStateBase> m_data;

    template <typename F, typename... TArgs>
    void create(const thread_attributes& attrs, F&& f, TArgs&&... args)
    {
        using namespace weos_detail;

        using shared_state_type = SharedThreadState<
            typename decay<F>::type, typename decay<TArgs>::type...>;

        m_data = std::make_shared<shared_state_type>(
                    attrs,
                    decay_copy(std::forward<F>(f)),
                    decay_copy(std::forward<TArgs>(args))...);
        m_data->m_thread = std::thread(&thread::threadedFunction, m_data);

        std::unique_lock<std::mutex> lock(m_data->m_mutex);
        m_data->m_signal.wait(lock, [&] { return m_data->m_isRegistered; });
    }

    static
    void threadedFunction(std::shared_ptr<weos_detail::SharedThreadStateBase> state) noexcept;
};

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

namespace this_thread
{

//! Waits for any signal.
//! Blocks the current thread until one or more signal flags have been set,
//! returns these flags and resets them.
thread::signal_set wait_for_any_signal();

//! Checks if any signal has arrived.
//! Checks if one or more signal flags have been set for the current thread,
//! returns these flags and resets them. If no signal is set, zero
//! is returned.
thread::signal_set try_wait_for_any_signal();

//! Waits until any signal arrives or a timeout occurs.
//! Waits up to the timeout period \p d for one or more signals to be set for
//! the current thread. The set signals will be returned. If the timeout
//! expires, zero is returned.
template <typename RepT, typename PeriodT>
inline
thread::signal_set try_wait_for_any_signal_for(
            const chrono::duration<RepT, PeriodT>& d)
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    if (!data.m_signal.wait_for(
            lock, d, [&]{ return data.m_signalFlags != 0; }))
    {
        return 0;
    }
    thread::signal_set temp = data.m_signalFlags;
    data.m_signalFlags = 0;
    return temp;
}

//! Waits until a signal occurs or the time-point \p time is reached. Returns
//! the signals that arrive or a zero in case of a timeout.
template <typename TClock, typename TDuration>
thread::signal_set try_wait_for_any_signal_until(
            const chrono::time_point<TClock, TDuration>& time)
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    if (!data.m_signal.wait_until(
            lock, time, [&]{ return data.m_signalFlags != 0; }))
    {
        return 0;
    }
    thread::signal_set temp = data.m_signalFlags;
    data.m_signalFlags = 0;
    return temp;
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
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    if (!data.m_signal.wait_for(
            lock, d,
            [&]{ return (data.m_signalFlags & flags) == flags; }))
    {
        return false;
    }
    data.m_signalFlags &= ~flags;
    return true;
}

//! Waits until a set of signals is set or the time-point \p time is reached.
//! Returns \p true if the signals are set before the timeout and \p false
//! otherwise.
template <typename TClock, typename TDuration>
bool try_wait_for_all_signals_until(
            thread::signal_set flags,
            const chrono::time_point<TClock, TDuration>& time)
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    if (!data.m_signal.wait_until(
            lock, time,
            [&]{ return (data.m_signalFlags & flags) == flags; }))
    {
        return false;
    }
    data.m_signalFlags &= ~flags;
    return true;
}

} // namespace this_thread

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_THREAD_HPP
