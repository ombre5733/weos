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
{
    m_data = detail::ThreadData::pool().construct();
    WEOS_ASSERT(m_data != 0);

    m_data->function = fun;
    m_data->arg = arg;

    // Increase the reference count before creating the new thread.
    m_data->ref();
    osThreadDef_t threadDef = { weos_threadInvoker, osPriorityNormal, 1, 0 };
    m_data->m_threadId = osThreadCreate(&threadDef, m_data);
    if (!m_data->m_threadId)
    {
        m_data->deref();
        m_data->deref();
        m_data = 0;
    }
}

} // namespace weos
