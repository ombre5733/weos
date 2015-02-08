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
#include "../memorypool.hpp"

#include <cstdint>


// The stack must be able to hold the registers R0-R15.
static const std::size_t minimum_custom_stack_size = 64;


using namespace std;


//! A helper function to invoke a thread.
//! An RL RTX thread is a C function taking a <tt>void*</tt> argument. This
//! helper function adhers to this specification. The \p arg is a pointer to
//! a weos::ThreadData object which contains thread-specifica data such as
//! the actual function to start.
extern "C" void weos_threadInvoker(void* arg);


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     SharedThreadData
// ----=====================================================================----

namespace detail
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

SharedThreadData::SharedThreadData()
    : m_referenceCount(0),
      m_threadId(0)
{
}

SharedThreadData::~SharedThreadData()
{
}

void SharedThreadData::addRef()
{
    ++m_referenceCount;
}

void SharedThreadData::release()
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

} // namespace detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

void thread::join()
{
    if (!joinable())
        WEOS_THROW_SYSTEM_ERROR(errc::operation_not_permitted,
                                "thread::join: thread is not joinable");

    m_data->m_finished.wait();

    // The thread data is not needed any longer.
    m_data.reset();
}

void thread::clear_signals(signal_set flags)
{
    if (!joinable())
    {
        WEOS_THROW_SYSTEM_ERROR(
                    errc::operation_not_permitted,
                    "thread::clear_signals: thread is not joinable");
    }

    WEOS_ASSERT(flags <= all_signals);
    std::int32_t result = osSignalClear(threadId, flags);
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

    WEOS_ASSERT(flags <= all_signals);
    std::int32_t result = osSignalSet(threadId, flags);
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
                           toNativePriority(attrs.m_priority),
                           attrs.m_customStack, attrs.m_customStackSize,
                           m_data.get());
        m_data->m_threadId = static_cast<osThreadId>(taskId);
    }
    else
    {
        osThreadDef_t threadDef = { weos_threadInvoker,
                                    toNativePriority(attrs.m_priority),
                                    1, 0 };
        m_data->m_threadId = osThreadCreate(&threadDef, m_data.get());
    }

    if (m_data->m_threadId)
    {
        m_data->m_initializationDone.post();
    }
    else
    {
        // Destroy the thread-data.
        m_data.reset();

        WEOS_THROW_SYSTEM_ERROR(
                    errc::no_child_process,
                    "thread::invoke: new thread was not created");
    }
}

// ----=====================================================================----
//     Waiting for signals
// ----=====================================================================----

namespace this_thread
{

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

} // namespace this_thread

WEOS_END_NAMESPACE
