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

// The function which actually creates a thread. The signature can be found
// in ../3rdparty/keil_cmsis_rtos/SRC/rt_Task.h.
extern "C" std::uint32_t rt_tsk_create(void(*task)(void),
                                       std::uint32_t prio_stksz,
                                       void* stk, void* argv);

extern "C" void osThreadExit(void);

// An array of pointers to task/thread control blocks. The declaration is
// from ../3rdparty/keil_cmsis_rtos/INC/RTX_Config.h.
extern void* os_active_TCB[];

extern "C" void weos_threadInvoker(const void* arg)
{
    weos::detail::ThreadData* data
            = static_cast<weos::detail::ThreadData*>(const_cast<void*>(arg));

    data->function(data->arg);
    data->m_finished.post();
    data->deref();
}

namespace weos
{
namespace detail
{

ThreadData::ThreadData()
    : m_referenceCount(1),
      m_threadId(0)
{
}

// ----=====================================================================----
//     ThreadData
// ----=====================================================================----

void ThreadData::deref()
{
    --m_referenceCount;
    if (m_referenceCount == 0)
        pool().destroy(this);
}

void ThreadData::ref()
{
    ++m_referenceCount;
}

ThreadData::pool_t& ThreadData::pool()
{
    static pool_t instance;
    return instance;
}

} // namespace detail

// ----=====================================================================----
//     thread
// ----=====================================================================----

thread::thread(void (*fun)(void*), void* arg)
    : m_data(0)
{
    m_data = detail::ThreadData::pool().construct();
    WEOS_ASSERT(m_data != 0);
    m_data->function = fun;
    m_data->arg = arg;

    // Increase the reference count before creating the new thread.
    m_data->ref();

    // Start the new thread.
    osThreadDef_t threadDef = { weos_threadInvoker, osPriorityNormal, 1, 0 };
    m_data->m_threadId = osThreadCreate(&threadDef, m_data);
    if (!m_data->m_threadId)
    {
        // Destroy the thread-data.
        m_data->deref();
        m_data->deref();
        m_data = 0;
    }
}

thread::thread(const attributes& attrs, void (*fun)(void*), void* arg)
    : m_data(0)
{
    invoke(attrs, fun, arg);
}

void thread::invoke(const attributes& attrs, void (*fun)(void*), void* arg)
{
    WEOS_ASSERT(attrs.m_customStack != 0);
    WEOS_ASSERT(attrs.m_customStackSize >= 14*4);

    m_data = detail::ThreadData::pool().construct();
    WEOS_ASSERT(m_data != 0);
    m_data->function = fun;
    m_data->arg = arg;

    // Increase the reference count before creating the new thread.
    m_data->ref();

    // Start the new thread with a custom stack.
    std::uint32_t taskId = rt_tsk_create(
                               (void (*)(void))weos_threadInvoker,
                               (attrs.m_priority - osPriorityIdle + 1)
                               | (attrs.m_customStackSize << 8),
                               attrs.m_customStack,
                               arg);
    if (taskId)
    {
        // Set R13 to the address of osThreadExit, which has to be invoked
        // when the thread exits.
        *((std::uint32_t*)attrs.m_customStack + 13)
                = (std::uint32_t)osThreadExit;
        m_data->m_threadId = (osThreadId)os_active_TCB[taskId - 1];
    }

    if (!m_data->m_threadId)
    {
        // Destroy the thread-data.
        m_data->deref();
        m_data->deref();
        m_data = 0;
    }
}

} // namespace weos
