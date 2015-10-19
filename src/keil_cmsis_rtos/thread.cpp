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

#include "thread.hpp"
#include "svc_indirection.hpp"
#include "../memorypool.hpp"

#include <cstdint>

using namespace std;


// The stack must be able to hold the registers R0-R15.
static constexpr size_t minimum_custom_stack_size = 64;

// The code below directly accesses OS_TCB defined in
// ${CMSIS-RTOS}/SRC/rt_TypeDef.h. The following offsets are needed:
static constexpr auto offsetof_priv_stack = 38;
static constexpr auto offsetof_ptask = 48;
static_assert(osCMSIS_RTX <= ((4<<16) | 78), "Check that layout of OS_TCB.");

// ----=====================================================================----
//     Functions imported from the CMSIS implementation
// ----=====================================================================----

extern "C"
{

// The function which actually creates a thread. The signature can be found
// in ../3rdparty/keil_cmsis_rtos/SRC/rt_Task.h.
uint32_t rt_tsk_create(void(*task)(void),
                       uint32_t prio_stksz,
                       void* stk, void* argv);

// The function which is called when a thread exits.
int svcThreadTerminate(void* thread_id);

// An array of pointers to task/thread control blocks. The declaration is
// from ../3rdparty/keil_cmsis_rtos/INC/RTX_Config.h.
extern void* os_active_TCB[];

} // extern "C"


//! Converts a weos priority to a CMSIS priority.
static inline constexpr
osPriority toNativePriority(weos::thread::attributes::priority priority) noexcept
{
    return static_cast<osPriority>(int(priority));
}

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
extern "C" int weos_terminateTask(void* sema, void* threadId) noexcept
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
//! a weos::SharedThreadData object which contains thread-specific data such as
//! the actual function to start.
extern "C" void weos_threadInvoker(const void* arg) noexcept
{
    using namespace WEOS_NAMESPACE;

    auto data = static_cast<weos_detail::SharedThreadData*>(
                    const_cast<void*>(arg));

    // The stack memory was not allocated from the pool. Set the private stack
    // size 'priv_stack' to zero, such that CMSIS won't add the memory to
    // its pool when the thread finishes.
    void* ptcb = osThreadGetId();
    *reinterpret_cast<std::uint16_t*>(static_cast<char*>(ptcb) + offsetof_priv_stack) = 0;

#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    try
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    {
        // Call the threaded function.
        data->m_threadedFunction();
    }
#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER
    catch (...)
    {
        ::WEOS_NAMESPACE::unhandled_thread_exception(weos::current_exception());
    }
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

    // Keep the thread alive because someone might still set a signal.
    data->m_joinedOrDetached.wait();

    // We do not want to deallocate the shared data in the interrupt context
    // because the allocator might want to lock a mutex. So the reference
    // counter is decreased right now although (part of) the shared data
    // is still needed.
    if (--data->m_referenceCount == 0)
    {
        // The invokee has to destroy the shared data. After that it will
        // cancel the thread.
        data->destroy();
        weos_terminateTask_indirect(nullptr, ptcb);
    }
    else
    {
        // The invoker has to destroy the shared data. The invokee signals
        // the end of the threaded function and cancels the thread.
        weos_terminateTask_indirect(&data->m_finished, ptcb);
    }
}


extern "C" void* weos_createTask(
        void* stack, uint32_t stackSize_and_priority,
        void* data, uint32_t debugFunctionPtr) noexcept
{
    uint32_t taskId = rt_tsk_create(
                          (void (*)(void))weos_threadInvoker,
                          stackSize_and_priority,
                          stack,
                          data);
    if (taskId)
    {
        void* pTCB = os_active_TCB[taskId - 1];

        // Set the field ptask in OS_TCB to the invoked function (needed for
        // the uVision debugger).
        *reinterpret_cast<std::uint32_t*>(static_cast<char*>(pTCB) + offsetof_ptask)
                = debugFunctionPtr;
        return pTCB;
    }

    return 0;
}

SVC_4(weos_createTask, void*,   void*, uint32_t, void*, uint32_t)


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     SharedThreadData
// ----=====================================================================----

namespace weos_detail
{

namespace
{
typedef shared_memory_pool<SharedThreadData, WEOS_MAX_NUM_CONCURRENT_THREADS>
    SharedThreadDataPool;

SharedThreadDataPool& sharedThreadDataPool() noexcept
{
    static SharedThreadDataPool pool;
    return pool;
}

} // anonymous namespace

SharedThreadData::SharedThreadData() noexcept
    : m_threadId(0),
      m_referenceCount(1)
{
}

void SharedThreadData::destroy() noexcept
{
    this->~SharedThreadData();
    sharedThreadDataPool().free(this);
}

SharedThreadData* SharedThreadData::allocate()
{
    void* mem = sharedThreadDataPool().try_allocate();
    if (!mem)
        WEOS_THROW_SYSTEM_ERROR(
                    errc::not_enough_memory,
                    "SharedThreadData::allocate: no more thread handle");

    return new (mem) SharedThreadData;
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

void thread::invoke(const attributes& attrs)
{
    if (attrs.m_customStack != nullptr
        && (   attrs.m_customStackSize < minimum_custom_stack_size
            || attrs.m_customStackSize >= (std::uint32_t(1) << 24)))
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::invalid_argument,
                    "thread::invoke: invalid thread attributes");
    }

    // Start the new thread.
    if (attrs.m_customStack)
    {
        void* taskId = weos_createTask_indirect(
                           attrs.m_customStack,
                           uint32_t(int(attrs.m_priority) - osPriorityIdle + 1)
                           | (attrs.m_customStackSize << 8),
                           m_data,
                           (std::uint32_t)(&weos_threadInvoker));
        m_data->m_threadId = static_cast<osThreadId>(taskId);
    }
    else
    {
        osThreadDef_t threadDef = { weos_threadInvoker,
                                    toNativePriority(attrs.m_priority),
                                    1, 0 };
        m_data->m_threadId = osThreadCreate(&threadDef, m_data);
    }

    if (m_data->m_threadId)
    {
        // The invoked thread will only decrease the reference count. It is
        // impossible that the threaded function has already decreased the
        // reference count. Even if it has already finished, the wrapper
        // function blocks on the m_joinedOrDetached semaphore.
        ++m_data->m_referenceCount;
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
