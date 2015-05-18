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


// The stack must be able to hold the registers R0-R15.
static const std::size_t minimum_custom_stack_size = 64;


using namespace std;

// The function which actually creates a thread. The signature can be found
// in ../3rdparty/keil_cmsis_rtos/SRC/rt_Task.h.
extern "C" uint32_t rt_tsk_create(void(*task)(void),
                                  uint32_t prio_stksz,
                                  void* stk, void* argv);

// The function which is called when a thread exits. This function has to
// be set as return address for every new thread.
extern "C" void osThreadExit(void);

// An array of pointers to task/thread control blocks. The declaration is
// from ../3rdparty/keil_cmsis_rtos/INC/RTX_Config.h.
extern void* os_active_TCB[];

//! Converts a weos priority to a CMSIS priority.
static inline
osPriority toNativePriority(weos::thread::attributes::priority priority)
{
    return static_cast<osPriority>(int(priority));
}

#ifdef WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

WEOS_BEGIN_NAMESPACE
void unhandled_thread_exception(exception_ptr exc);
WEOS_END_NAMESPACE

#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

extern "C" void weos_threadExit(void)
{
    static_assert(osCMSIS_RTX <= ((4<<16) | 75),
                  "Check that offsetof(OS_TCB, priv_stack) == 38.");
    static constexpr auto priv_stack_offset = 38;

    void* ptcb = osThreadGetId();
    // The memory was not allocated from the pool. Set the private stack
    // size 'priv_stack' to zero, such that CMSIS won't add the memory to
    // its pool.
    *reinterpret_cast<std::uint16_t*>(static_cast<char*>(ptcb) + priv_stack_offset) = 0;

    osThreadExit();
}

//! A helper function to invoke a thread.
//! A CMSIS thread is a C function taking a <tt>const void*</tt> argument. This
//! helper function adheres to this specification. The \p arg is a pointer to
//! a weos::SharedThreadData object which contains thread-specific data such as
//! the actual function to start.
extern "C" void weos_threadInvoker(const void* arg)
{
    using namespace WEOS_NAMESPACE;

    unique_ptr<weos_detail::SharedThreadData,
               weos_detail::SharedThreadDataDeleter> data(
        static_cast<weos_detail::SharedThreadData*>(const_cast<void*>(arg)));

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
        weos::unhandled_thread_exception(weos::current_exception());
    }
#endif // WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

    // Use the semaphore to signal that the thread has been completed.
    data->m_finished.post();
    // Keep the thread alive because someone might still set a signal.
    data->m_joinedOrDetached.wait();
}

extern "C" void* weos_createTask(
        void* stack, uint32_t stackSize_and_priority,
        void* data, uint32_t debugFunctionPtr)
{
    uint32_t taskId = rt_tsk_create(
                          (void (*)(void))weos_threadInvoker,
                          stackSize_and_priority,
                          stack,
                          data);
    if (taskId)
    {
        void* pTCB = os_active_TCB[taskId - 1];

        // Set R13 to the address of osThreadExit, which has to be invoked
        // when the thread exits.
        static_cast<uint32_t*>(stack)[13] = (uint32_t)&weos_threadExit;

        // Set the field ptask in OS_TCB to the invoked function (needed for
        // the uVision debugger).
        static_assert(osCMSIS_RTX <= ((4<<16) | 75),
                      "Check that offsetof(OS_TCB, ptask) == 48.");
        static constexpr auto ptask_offset = 48;
        *reinterpret_cast<std::uint32_t*>(static_cast<char*>(pTCB) + ptask_offset)
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

SharedThreadDataPool& sharedThreadDataPool()
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

void SharedThreadData::decReferenceCount() noexcept
{
    if (--m_referenceCount == 0)
    {
        this->~SharedThreadData();
        sharedThreadDataPool().free(this);
    }
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

    unique_ptr<weos_detail::SharedThreadData,
               weos_detail::SharedThreadDataDeleter> data(m_data);

    m_data->m_joinedOrDetached.post();
    m_data = nullptr;
}

void thread::join()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(errc::operation_not_permitted,
                                "thread::join: thread is not joinable");

    unique_ptr<weos_detail::SharedThreadData,
               weos_detail::SharedThreadDataDeleter> data(m_data);

    m_data->m_joinedOrDetached.post();
    m_data->m_finished.wait();
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
    if (attrs.m_customStack != 0
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
        // The invoked thread will only decrease the reference count.
        m_data->addReferenceCount();
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

    if (ms > milliseconds::zero())
    {
        // An osDelay() of 1ms only blocks until the following time slot.
        // As some time already has passed in this slot, the true delay is
        // shorter than what the user specified.

        // We increase the number of milliseconds by one here.
        // TODO: Fix this for clocks other than milliseconds.
        static_assert(   chrono::system_clock::period::num == 1
                      && chrono::system_clock::period::den == 1000,
                      "Only implemented for milliseconds");
        ++ms;

        while (true)
        {
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

            if (ms <= milliseconds::zero())
                break;
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

    if (ms < milliseconds::zero())
        ms = milliseconds::zero();

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

    } while (ms > milliseconds::zero());

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

    if (ms < milliseconds::zero())
        ms = milliseconds::zero();

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
                                    "try_wait_for_any_signal_for failed");
        }

    } while (ms > milliseconds::zero());

    return false;
}

} // namespace this_thread

WEOS_END_NAMESPACE
