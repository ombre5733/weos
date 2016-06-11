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

#include "_thread.hpp"
#include "../memory.hpp"

#include <cstdint>
#include <cstdlib>
#include <map>
#include <new>

using namespace std;

// ----=====================================================================----
//     Thread observers
// ----=====================================================================----

WEOS_BEGIN_NAMESPACE

namespace expert
{
void for_each_thread(function<bool(thread_info)> /*f*/)
{
    // TODO: Implement this if needed
    std::terminate();
}
} // namespace expert

WEOS_END_NAMESPACE

// ----=====================================================================----
//     Helper functions
// ----=====================================================================----



WEOS_BEGIN_NAMESPACE

#ifdef WEOS_ENABLE_THREAD_HOOKS

WEOS_BEGIN_NAMESPACE

// Forward declare the hooks. The implementation has to be provided by the user.
void thread_created(expert::thread_info);
void thread_destroyed(expert::thread_info);

WEOS_END_NAMESPACE

#endif // WEOS_ENABLE_THREAD_HOOKS


#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

WEOS_BEGIN_NAMESPACE
void unhandled_thread_exception(exception_ptr exc);
WEOS_END_NAMESPACE

#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

// ----=====================================================================----
//     thread_info
// ----=====================================================================----

namespace expert
{

const char* thread_info::get_name() const noexcept
{
    return m_state->m_attrs.get_name();
}

void* thread_info::get_stack_begin() const noexcept
{
    return m_state->m_attrs.get_stack_begin();
}

std::size_t thread_info::get_stack_size() const noexcept
{
    return m_state->m_attrs.get_stack_size();
}

std::size_t thread_info::get_used_stack() const noexcept
{
    return get_stack_size();
}

std::thread::id thread_info::get_id() const noexcept
{
    return m_state->m_thread.get_id();
}

thread_attributes::priority thread_info::get_priority() const noexcept
{
    return m_state->m_attrs.get_priority();
}

const void* thread_info::native_handle() const noexcept
{
    return &m_state->m_thread;
}

} // namespace expert

// ----=====================================================================----
//     stack allocation
// ----=====================================================================----

namespace expert
{

atomic<bool> g_stack_allocation_enabled{false};
atomic<std::size_t> g_default_stack_size{0};

bool set_stack_allocation_enabled(bool enable)
{
    return g_stack_allocation_enabled.exchange(enable);
}

std::size_t set_default_stack_size(std::size_t size)
{
    return g_default_stack_size.exchange(size);
}

} // namespace expert

// ----=====================================================================----
//     SharedThreadState
// ----=====================================================================----

namespace weos_detail
{

struct SharedThreadStateManagerPrivate
{
    std::mutex m_idToDataMutex;
    std::map<std::thread::id, SharedThreadStateBase*> m_idToData;
};

SharedThreadStateManager::SharedThreadStateManager()
    : m_pimpl(new SharedThreadStateManagerPrivate())
{
}

SharedThreadStateManager::~SharedThreadStateManager()
{
    delete m_pimpl;
}

SharedThreadStateManager& SharedThreadStateManager::instance()
{
    static SharedThreadStateManager manager;
    return manager;
}

void SharedThreadStateManager::add(std::thread::id id, SharedThreadStateBase* data)
{
    std::lock_guard<std::mutex> lock(m_pimpl->m_idToDataMutex);
    WEOS_ASSERT(m_pimpl->m_idToData.find(id) == m_pimpl->m_idToData.cend());
    m_pimpl->m_idToData[id] = data;
}

SharedThreadStateBase& SharedThreadStateManager::find(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_pimpl->m_idToDataMutex);
    auto iter = m_pimpl->m_idToData.find(id);
    WEOS_ASSERT(iter != m_pimpl->m_idToData.cend());
    return *iter->second;
}

void SharedThreadStateManager::remove(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_pimpl->m_idToDataMutex);
    auto iter = m_pimpl->m_idToData.find(id);
    WEOS_ASSERT(iter != m_pimpl->m_idToData.cend());
    m_pimpl->m_idToData.erase(iter);
}



SharedThreadStateBase::SharedThreadStateBase(const thread_attributes& attrs) noexcept
    : m_isRegistered(false),
      m_joinedOrDetached(false),
      m_signalFlags(0),
      m_attrs(attrs)
{
}

} // namespace weos_detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

thread::thread(thread&& other) noexcept
    : m_data(std::move(other.m_data))
{
}

thread::~thread()
{
    if (joinable())
        std::terminate();
}

thread& thread::operator=(thread&& other) noexcept
{
    m_data = std::move(other.m_data);
    return *this;
}

thread::id thread::get_id() const noexcept
{
    if (m_data)
        return m_data->m_thread.get_id();
    else
        return id();
}

void thread::detach()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(std::errc::operation_not_permitted,
                                "thread::detach: thread is not joinable");

    m_data->m_mutex.lock();
    m_data->m_joinedOrDetached = true;
    m_data->m_mutex.unlock();
    m_data->m_signal.notify_one();

    m_data->m_thread.detach();
    m_data.reset();
}

void thread::join()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(std::errc::operation_not_permitted,
                                "thread::join: thread is not joinable");

    m_data->m_mutex.lock();
    m_data->m_joinedOrDetached = true;
    m_data->m_mutex.unlock();
    m_data->m_signal.notify_one();

    m_data->m_thread.join();
    m_data.reset();
}

void thread::clear_signals(signal_set flags)
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(std::errc::operation_not_permitted,
                                "thread::clear_signals: no thread");

    std::lock_guard<std::mutex> lock(m_data->m_mutex);
    m_data->m_signalFlags &= ~flags;
    m_data->m_signal.notify_one();
}

void thread::set_signals(signal_set flags)
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(std::errc::operation_not_permitted,
                                "thread::set_signals: no thread");

    std::lock_guard<std::mutex> lock(m_data->m_mutex);
    m_data->m_signalFlags |= flags;
    m_data->m_signal.notify_one();
}

void thread::threadedFunction(std::shared_ptr<weos_detail::SharedThreadStateBase> state) noexcept
{
    // Register the shared thread state.
    auto& manager = weos_detail::SharedThreadStateManager::instance();
    manager.add(std::this_thread::get_id(), state.get());

    std::unique_lock<std::mutex> lock(state->m_mutex);
    state->m_isRegistered = true;
    lock.unlock();
    state->m_signal.notify_one();

#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    try
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    {
#ifdef WEOS_ENABLE_THREAD_HOOKS
        ::WEOS_NAMESPACE::thread_created(state);
#endif // WEOS_ENABLE_THREAD_HOOKS

        // Call the threaded function.
        state->execute();

#ifdef WEOS_ENABLE_THREAD_HOOKS
        ::WEOS_NAMESPACE::thread_destroyed(state);
#endif // WEOS_ENABLE_THREAD_HOOKS
    }
#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    catch (...)
    {
        ::WEOS_NAMESPACE::unhandled_thread_exception(std::current_exception());
    }
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

    // Keep the thread alive because someone might still set a signal.
    lock.lock();
    state->m_signal.wait(lock, [state] { return state->m_joinedOrDetached; });

    // Remove the shared thread state.
    manager.remove(std::this_thread::get_id());
}

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

namespace this_thread
{

thread::signal_set wait_for_any_signal()
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    data.m_signal.wait(lock, [&]{ return data.m_signalFlags != 0; });
    thread::signal_set temp = data.m_signalFlags;
    data.m_signalFlags = 0;
    return temp;
}

thread::signal_set try_wait_for_any_signal()
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    data.m_mutex.lock();
    thread::signal_set temp = data.m_signalFlags;
    data.m_signalFlags = 0;
    data.m_mutex.unlock();
    return temp;
}

void wait_for_all_signals(thread::signal_set flags)
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    std::unique_lock<std::mutex> lock(data.m_mutex);
    data.m_signal.wait(
                lock,
                [&]{ return (data.m_signalFlags & flags) == flags; });
    data.m_signalFlags &= ~flags;
}

bool try_wait_for_all_signals(thread::signal_set flags)
{
    weos_detail::SharedThreadStateBase& data
            = weos_detail::SharedThreadStateManager::instance().find(
                std::this_thread::get_id());

    data.m_mutex.lock();
    thread::signal_set temp = (data.m_signalFlags & flags) == flags
                              ? flags : 0;
    data.m_signalFlags &= ~temp;
    data.m_mutex.unlock();
    return temp != 0;
}

} // namespace this_thread

WEOS_END_NAMESPACE
