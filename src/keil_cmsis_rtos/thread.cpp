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

#include "thread.hpp"
#include "memorypool.hpp"

// The function which actually creates a thread. The signature can be found
// in ../3rdparty/keil_cmsis_rtos/SRC/rt_Task.h.
extern "C" std::uint32_t rt_tsk_create(void(*task)(void),
                                       std::uint32_t prio_stksz,
                                       void* stk, void* argv);

// The function which is called when a thread exits. This function has to
// be set as return address for every new thread.
extern "C" void osThreadExit(void);

// An array of pointers to task/thread control blocks. The declaration is
// from ../3rdparty/keil_cmsis_rtos/INC/RTX_Config.h.
extern void* os_active_TCB[];

namespace
{

//! A helper function to invoke a thread.
//! A CMSIS thread is a C function taking a <tt>const void*</tt> argument. This
//! helper function adhers to this specification. The \p arg is a pointer to
//! a weos::ThreadSharedData object which contains thread-specific data such as
//! the actual function to start.
extern "C" void weos_threadInvoker(const void* arg)
{
    weos::detail::ThreadSharedData* data
            = static_cast<weos::detail::ThreadSharedData*>(const_cast<void*>(arg));

    // Call the threaded function.
    data->invoke();
    // Use the semaphore to signal that the thread has been completed.
    data->m_finished.post();
    data->deref();
}

} // anonymous namespace

namespace weos
{
namespace detail
{

// ----=====================================================================----
//     ThreadSharedData
// ----=====================================================================----

namespace
{
typedef shared_memory_pool<ThreadSharedData,
                           WEOS_MAX_NUM_CONCURRENT_THREADS>
    ThreadSharedDataPool;

ThreadSharedDataPool& threadSharedDataPool()
{
    static ThreadSharedDataPool pool;
    return pool;
}

} // anonymous namespace

ThreadSharedData::ThreadSharedData()
    : m_threadId(0)
{
    m_referenceCount.value = 1;
}

void ThreadSharedData::deref()
{
    int newValue;
    {
        lock_guard<mutex> lock(m_referenceCount.mtx);
        --m_referenceCount.value;
        newValue = m_referenceCount.value;
    }
    if (newValue == 0)
    {
        this->~ThreadSharedData();
        threadSharedDataPool().free(this);
    }
}

void ThreadSharedData::ref()
{
    lock_guard<mutex> lock(m_referenceCount.mtx);
    ++m_referenceCount.value;
}

ThreadSharedData* ThreadSharedData::allocate()
{
    void* mem = threadSharedDataPool().try_allocate();
    if (!mem)
    {
        ::weos::throw_exception(system_error(cmsis_error::osErrorResource, cmsis_category())); //! \todo Use correct value
    }

    return new (mem) ThreadSharedData;
}

} // namespace detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

void thread::invokeWithCustomStack(const attributes& attrs)
{
    if (   attrs.m_customStack == 0
        || attrs.m_customStackSize < detail::native_thread_traits::minimum_custom_stack_size
        || attrs.m_customStackSize >= (std::uint32_t(1) << 24))
    {
        ::weos::throw_exception(weos::system_error(
                                    cmsis_error::osErrorParameter,
                                    cmsis_category()));
    }

    // Increase the reference count before creating the new thread.
    m_data->ref();
    // Start the new thread with a custom stack.
    std::uint32_t taskId = rt_tsk_create(
                               (void (*)(void))weos_threadInvoker,
                               (attrs.m_priority - osPriorityIdle + 1)
                               | (attrs.m_customStackSize << 8),
                               attrs.m_customStack,
                               m_data);
    if (taskId)
    {
        // Set R13 to the address of osThreadExit, which has to be invoked
        // when the thread exits.
        *(static_cast<std::uint32_t*>(attrs.m_customStack) + 13)
                = (std::uint32_t)osThreadExit;
        m_data->m_threadId = (osThreadId)os_active_TCB[taskId - 1];
    }

    if (!m_data->m_threadId)
    {
        // Destroy the thread-data.
        m_data->deref();
        m_data->deref();
        m_data = 0;

        //! \todo Use correct error code
        ::weos::throw_exception(weos::system_error(
                                    cmsis_error::osErrorResource,
                                    cmsis_category()));
    }
}

void thread::invokeWithDefaultStack(const attributes& attrs)
{
    WEOS_ASSERT(attrs.m_customStack == 0 && attrs.m_customStackSize == 0);

    // Increase the reference count before creating the new thread.
    m_data->ref();
    // Start the new thread.
    osThreadDef_t threadDef = { weos_threadInvoker,
                                static_cast<osPriority>(attrs.m_priority),
                                1, 0 };
    m_data->m_threadId = osThreadCreate(&threadDef, m_data);

    if (!m_data->m_threadId)
    {
        // Destroy the thread-data.
        m_data->deref();
        m_data->deref();
        m_data = 0;

        //! \todo Use correct error code
        ::weos::throw_exception(weos::system_error(
                                    cmsis_error::osErrorResource,
                                    cmsis_category()));
    }
}

} // namespace weos
