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

#include "thread.hpp"
#include "svc_indirection.hpp"
#include "../memorypool.hpp"
#include "../memory.hpp"

#include <cstdint>
#include <cstdlib>
#include <new>

using namespace std;


// The stack must be able to hold the registers R0-R15.
static constexpr size_t minimum_custom_stack_size = 64;

// The code below directly accesses OS_TCB defined in
// ${CMSIS-RTOS}/SRC/rt_TypeDef.h. The following offsets are needed:
static constexpr auto offsetof_priv_stack = 38;
static constexpr auto offsetof_ptask = 48;
static_assert(osCMSIS_RTX <= ((4<<16) | 78), "Check that layout of OS_TCB.");

static constexpr auto STACK_WATERMARK = 0xE25A2EA5U;

// ----=====================================================================----
//     Functions imported from the CMSIS implementation
// ----=====================================================================----

extern "C"
{

// The function which actually creates a thread. The signature can be found
// in ${CMSIS-RTOS}/SRC/rt_Task.h.
uint32_t rt_tsk_create(void(*task)(void),
                       uint32_t prio_stksz,
                       void* stk, void* argv);

// The function which is called when a thread exits.
int svcThreadTerminate(void* thread_id);

// An array of pointers to task/thread control blocks. The declaration is
// from ${CMSIS-RTOS}/INC/RTX_Config.h.
extern void* os_active_TCB[];

} // extern "C"

// ----=====================================================================----
//     Shared thread states list
// ----=====================================================================----

static WEOS_NAMESPACE::weos_detail::SharedThreadStateBase* g_sharedThreadStates;

extern "C"
int weos_unlinkSharedState(void* s) noexcept
{
    using namespace WEOS_NAMESPACE::weos_detail;

    SharedThreadStateBase* state = static_cast<SharedThreadStateBase*>(s);
    if (g_sharedThreadStates == state)
    {
        g_sharedThreadStates = state->m_next;
    }
    else
    {
        for (SharedThreadStateBase* iter = g_sharedThreadStates;
             iter != nullptr; iter = iter->m_next)
        {
            if (iter->m_next == state)
            {
                iter->m_next = state->m_next;
                break;
            }
        }
    }
    return 0;
}

SVC_1(weos_unlinkSharedState, int,   void*)

// ----=====================================================================----
//     Thread observers
// ----=====================================================================----

WEOS_BEGIN_NAMESPACE

static inline
bool weos_forEachThreadWorkaround(void* cbk,
                                  weos_detail::SharedThreadStateBase* iter) noexcept
{
    using f_type = function<bool(expert::thread_info)>;
    f_type& f = *static_cast<f_type*>(cbk);
    return f(iter->info());
}

WEOS_END_NAMESPACE


extern "C"
int weos_forEachThread(void* cbk) noexcept
{
    using namespace WEOS_NAMESPACE;

    for (weos_detail::SharedThreadStateBase* iter = g_sharedThreadStates;
         iter != nullptr; iter = iter->m_next)
    {
        if (!weos_forEachThreadWorkaround(cbk, iter))
            break;
    }

    return 0;
}

SVC_1(weos_forEachThread, int,   void*)


WEOS_BEGIN_NAMESPACE

namespace expert
{
void for_each_thread(function<bool(thread_info)> f)
{
    if (__get_IPSR() != 0U)
    {
        // In an interrupt context, loop over the threads directly.
        weos_forEachThread(&f);
    }
    else
    {
        // In a task context, the loop has to be executed indirectly in
        // an ISR.
        weos_forEachThread_indirect(&f);
    }
}
} // namespace expert

WEOS_END_NAMESPACE

// ----=====================================================================----
//     Helper functions
// ----=====================================================================----

#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

WEOS_BEGIN_NAMESPACE
void unhandled_thread_exception(exception_ptr exc);
WEOS_END_NAMESPACE

#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

// Terminates the current thread.
//
// The only reason for this function is that the finished signal and the
// thread termination have to be done together. This is really important
// because the joining thread might re-use the stack of this thread. So we
// must ensure that there is no context switch between setting the semaphore
// signal and terminating the thread.
extern "C"
int weos_terminateTask(void* sema, void* threadId) noexcept
{
    using namespace WEOS_NAMESPACE;

    // Use the semaphore to signal that the thread has been completed.
    if (sema)
        static_cast<semaphore*>(sema)->post();
    //rt_tsk_delete(threadId);
    svcThreadTerminate(threadId);
    return 0;
}

SVC_2(weos_terminateTask, int,   void*, void*)


//! A helper function to invoke a thread.
//! A CMSIS thread is a C function taking a <tt>const void*</tt> argument. This
//! helper function adheres to this specification. The \p arg is a pointer to
//! a weos_detail::SharedThreadState object which contains thread-specific data
//! such as the actual function to start.
extern "C"
void weos_threadInvoker(const void* arg) noexcept
{
    using namespace WEOS_NAMESPACE;

    auto state = static_cast<weos_detail::SharedThreadStateBase*>(
                    const_cast<void*>(arg));

#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    try
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    {
        // Call the threaded function.
        state->execute();
    }
#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    catch (...)
    {
        ::WEOS_NAMESPACE::unhandled_thread_exception(weos::current_exception());
    }
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

    // Keep the thread alive because someone might still set a signal.
    state->m_joinedOrDetached.wait();

    // We do not want to deallocate the shared data in the interrupt context
    // because the allocator might want to lock a mutex. So the reference
    // counter is decreased right now although (part of) the shared data
    // is still needed.
    if (--state->m_referenceCount == 0)
    {
        // The invokee has to destroy the shared data. After that it will
        // cancel the thread.
        osThreadId threadId = state->m_threadId;
        state->destroy();
        weos_terminateTask_indirect(nullptr, threadId);
    }
    else
    {
        // The invoker has to destroy the shared data. The invokee signals
        // the end of the threaded function and cancels the thread.
        weos_terminateTask_indirect(&state->m_finished, state->m_threadId);
    }
}


extern "C"
void* weos_createTask(void* stack, uint32_t stackSize_and_priority,
                      void* state, uint32_t debugFunctionPtr) noexcept
{
    using namespace WEOS_NAMESPACE::weos_detail;

    uint32_t taskId = rt_tsk_create(
                          (void (*)(void))weos_threadInvoker,
                          stackSize_and_priority,
                          stack,
                          state);
    if (taskId)
    {
        void* pTCB = os_active_TCB[taskId - 1];

        // The stack memory was not allocated from the CMSIS pool. Set the
        // private stack size 'priv_stack' to zero, such that CMSIS won't add
        // the memory to its pool when the thread finishes.
        *reinterpret_cast<std::uint16_t*>(static_cast<char*>(pTCB) + offsetof_priv_stack) = 0;

        // Set the field ptask in OS_TCB to the invoked function (needed for
        // the uVision debugger).
        *reinterpret_cast<std::uint32_t*>(static_cast<char*>(pTCB) + offsetof_ptask)
                = debugFunctionPtr;

        // Add the new thread to the linked list.
        static_cast<SharedThreadStateBase*>(state)->m_next = g_sharedThreadStates;
        g_sharedThreadStates = static_cast<SharedThreadStateBase*>(state);

        return pTCB;
    }

    return 0;
}

SVC_4(weos_createTask, void*,   void*, uint32_t, void*, uint32_t)


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     thread_info
// ----=====================================================================----

namespace expert
{

const char* thread_info::get_name() const noexcept
{
    return m_state->m_name;
}

void* thread_info::get_stack_begin() const noexcept
{
    return m_state->m_initialStackBase;
}

std::size_t thread_info::get_stack_size() const noexcept
{
    return static_cast<const char*>(m_state->m_stackBegin)
            + m_state->m_stackSize
            - static_cast<const char*>(m_state->m_initialStackBase);
}

std::size_t thread_info::get_used_stack() const noexcept
{
    if (m_usedStack == std::size_t(-1))
    {
        uintptr_t endAddr = uintptr_t(m_state->m_stackBegin) + m_state->m_stackSize;
        const uint32_t* iter = static_cast<const uint32_t*>(m_state->m_stackBegin);
        while (*iter == STACK_WATERMARK && uintptr_t(iter) < endAddr)
        {
            ++iter;
        }

        m_usedStack = static_cast<const char*>(m_state->m_stackBegin)
                      - static_cast<const char*>(m_state->m_initialStackBase);
        if (uintptr_t(iter) < endAddr)
            m_usedStack += endAddr - uintptr_t(iter);
    }
    return m_usedStack;
}

weos_detail::thread_id thread_info::get_id() const noexcept
{
    return weos_detail::thread_id(m_state->m_threadId);
}

thread_attributes::priority thread_info::get_priority() const noexcept
{
    auto priority = osThreadGetPriority(m_state->m_threadId);
    WEOS_ASSERT(priority != osPriorityError);
    return static_cast<thread_attributes::priority>(priority);
}

const void* thread_info::native_handle() const noexcept
{
    return m_state->m_threadId;
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
//     ThreadProperties
// ----=====================================================================----

namespace weos_detail
{

ThreadProperties::Deleter::~Deleter()
{
    if (m_ownedStack)
        std::free(m_ownedStack);
}

ThreadProperties::ThreadProperties(const thread_attributes& attrs) noexcept
    : m_name(attrs.get_name()),
      m_priority(static_cast<int>(attrs.get_priority())),
      m_initialStackBase(attrs.get_stack_begin()),
      m_stackBegin(attrs.get_stack_begin()),
      m_stackSize(attrs.get_stack_size())
{
}

ThreadProperties::Deleter ThreadProperties::allocate()
{
    if (!m_stackBegin)
    {
        std::size_t size = m_stackSize;
        if (size == 0)
            size = expert::g_default_stack_size;

        if (!expert::g_stack_allocation_enabled || size == 0)
        {
            WEOS_THROW_SYSTEM_ERROR(
                        errc::not_enough_memory,
                        "ThreadProperties::allocate: stack allocation not allowed");
        }

        m_stackBegin = std::malloc(m_stackSize);
        if (!m_stackBegin)
            throw std::bad_alloc();

        m_stackSize = size;
        m_initialStackBase = m_stackBegin;
        return Deleter(m_stackBegin);
    }
    return Deleter(nullptr);
}

void* ThreadProperties::align(std::size_t alignment, std::size_t size) noexcept
{
    return ::WEOS_NAMESPACE::align(alignment, size, m_stackBegin, m_stackSize);
}

void* ThreadProperties::max_align() noexcept
{
    using namespace std;

    static constexpr size_t alignment = alignment_of<long double>::value;
    uintptr_t address = uintptr_t(m_stackBegin);
    uintptr_t aligned_address = (address + (alignment - 1)) & -alignment;
    size_t diff = aligned_address - address;
    if (m_stackSize >= diff)
    {
        m_stackSize -= diff;
        m_stackBegin = reinterpret_cast<void*>(aligned_address);
        return m_stackBegin;
    }
    return nullptr;
}

void ThreadProperties::offset_by(std::size_t size) noexcept
{
    m_stackBegin = static_cast<char*>(m_stackBegin) + size;
    m_stackSize -= size;
}

} // namespace weos_detail

// ----=====================================================================----
//     SharedThreadState
// ----=====================================================================----

namespace weos_detail
{

SharedThreadStateBase::SharedThreadStateBase(const ThreadProperties& props,
                                             void* ownedStack) noexcept
    : m_threadId(0),
      m_referenceCount(1),
      m_next(nullptr),
      m_ownedStack(ownedStack),
      m_name(props.m_name),
      m_initialStackBase(props.m_initialStackBase)
{
}

void SharedThreadStateBase::setStack(const ThreadProperties& props)
{
    m_stackBegin = props.m_stackBegin;
    m_stackSize = props.m_stackSize;
}

void SharedThreadStateBase::destroy() noexcept
{
    weos_unlinkSharedState_indirect(this);
    this->~SharedThreadStateBase();
    if (m_ownedStack)
        std::free(m_ownedStack);
}

} // namespace weos_detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

void thread::detach()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(errc::operation_not_permitted,
                                "thread::detach: thread is not joinable");

    m_data->m_joinedOrDetached.post();
    // If the invokee has already decreased the reference count, we have to
    // deallocate the shared data.
    if (--m_data->m_referenceCount == 0)
    {
        // Watch out: The invokee still needs to access m_finished.
        m_data->m_finished.wait();
        m_data->destroy();
    }

    m_data = nullptr;
}

void thread::join()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(errc::operation_not_permitted,
                                "thread::join: thread is not joinable");

    m_data->m_joinedOrDetached.post();
    m_data->m_finished.wait();
    // If the invokee has already decreased the reference count, we have to
    // deallocate the shared data.
    if (--m_data->m_referenceCount == 0)
        m_data->destroy();

    m_data = nullptr;
}

void thread::clear_signals(signal_set flags)
{
    if (!joinable())
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::operation_not_permitted,
                    "thread::clear_signals: thread is not joinable");
    }

    WEOS_ASSERT(flags <= thread::all_signals());
    std::int32_t result = osSignalClear(m_data->m_threadId, flags);
    WEOS_ASSERT(result >= 0);
    (void)result;
}

void thread::set_signals(signal_set flags)
{
    if (!joinable())
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::operation_not_permitted,
                    "thread::set_signals: thread is not joinable");
    }

    WEOS_ASSERT(flags <= thread::all_signals());
    std::int32_t result = osSignalSet(m_data->m_threadId, flags);
    WEOS_ASSERT(result >= 0);
    (void)result;
}

void thread::do_create(weos_detail::ThreadProperties& props,
                       weos_detail::SharedThreadStateBase* state)
{
    if (!props.max_align())
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::not_enough_memory,
                    "thread::do_create: stack size is too small");
    }

    if (   props.m_stackSize < minimum_custom_stack_size
        || props.m_stackSize >= (std::size_t(1) << 24))
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::invalid_argument,
                    "thread::do_create: invalid stack size");
    }

    // Tell the thread state, where the real stack (after alignment...) resides.
    state->setStack(props);

    // Fill the stack with the watermark pattern.
    for (auto iter = static_cast<uint32_t*>(props.m_stackBegin),
               end = static_cast<uint32_t*>(props.m_stackBegin) + props.m_stackSize / 4;
         iter < end; ++iter)
    {
        *iter = STACK_WATERMARK;
    }

    // Start the new thread.
    void* taskId = weos_createTask_indirect(
                       props.m_stackBegin,
                       uint32_t(props.m_priority - osPriorityIdle + 1)
                       | (props.m_stackSize << 8),
                       state,
                       (std::uint32_t)(&weos_threadInvoker));
    if (taskId)
    {
        state->m_threadId = static_cast<osThreadId>(taskId);
        // The invoked thread will only decrease the reference count. It is
        // impossible that the threaded function has already decreased the
        // reference count. Even if it has already ended, the wrapping
        // function blocks on the m_joinedOrDetached semaphore.
        ++state->m_referenceCount;
    }
    else
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::no_child_process,
                    "thread::invoke: new thread was not created");
    }
}

namespace this_thread
{
// ----=====================================================================----
//     Sleeping
// ----=====================================================================----

void sleep_for(chrono::milliseconds ms)
{
    using namespace chrono;

    if (ms <= ms.zero())
        return;

    // An osDelay() of 1ms only blocks until the following time slot.
    // As some time already has passed in this slot, the true delay is
    // shorter than what the user specified.

    // We increase the number of milliseconds by one here.
    // TODO: Fix this for clocks other than milliseconds.
    static_assert(   chrono::system_clock::period::num == 1
                  && chrono::system_clock::period::den == 1000,
                  "Only implemented for milliseconds");
    ++ms;

    while (ms > ms.zero())
    {
        static_assert(osCMSIS_RTX <= ((4<<16) | 78),
                      "Check the maximum timeout.");
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;
        osStatus result = osDelay(truncated.count());
        if (result != osOK && result != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "sleep_for failed");
        }
    }
}

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

thread::signal_set wait_for_any_signal()
{
    osEvent result = osSignalWait(0, osWaitForever);
    if (result.status != osEventSignal)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                "wait_for_any_signal failed");

    return result.value.signals;
}

thread::signal_set try_wait_for_any_signal()
{
    osEvent result = osSignalWait(0, 0);
    if (result.status == osEventSignal)
    {
        return result.value.signals;
    }

    if (   result.status != osOK
        && result.status != osEventTimeout)
    {
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                "try_wait_for_any_signal failed");
    }

    return 0;
}

thread::signal_set try_wait_for_any_signal_for(chrono::milliseconds ms)
{
    using namespace chrono;

    if (ms < ms.zero())
        ms = ms.zero();

    do
    {
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;

        osEvent result = osSignalWait(0, truncated.count());
        if (result.status == osEventSignal)
        {
            return result.value.signals;
        }

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_any_signal_for failed");
        }

    } while (ms > ms.zero());

    return 0;
}

void wait_for_all_signals(thread::signal_set flags)
{
    WEOS_ASSERT(flags > 0 && flags <= thread::all_signals());
    osEvent result = osSignalWait(flags, osWaitForever);
    if (result.status != osEventSignal)
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                "wait_for_signalflags failed");
}

bool try_wait_for_all_signals(thread::signal_set flags)
{
    WEOS_ASSERT(flags > 0 && flags <= thread::all_signals());
    osEvent result = osSignalWait(flags, 0);
    if (result.status == osEventSignal)
    {
        return true;
    }

    if (   result.status != osOK
        && result.status != osEventTimeout)
    {
        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                "try_wait_for_all_signals failed");
    }

    return false;
}

bool try_wait_for_all_signals_for(thread::signal_set flags,
                                  chrono::milliseconds ms)
{
    using namespace chrono;

    WEOS_ASSERT(flags > 0 && flags <= thread::all_signals());

    if (ms < ms.zero())
        ms = ms.zero();

    do
    {
        milliseconds truncated = ms <= milliseconds(0xFFFE)
                                 ? ms
                                 : milliseconds(0xFFFE);
        ms -= truncated;

        osEvent result = osSignalWait(flags, truncated.count());
        if (result.status == osEventSignal)
        {
            return true;
        }

        if (   result.status != osOK
            && result.status != osEventTimeout)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "try_wait_for_all_signals_for failed");
        }

    } while (ms > ms.zero());

    return false;
}

} // namespace this_thread

WEOS_END_NAMESPACE
