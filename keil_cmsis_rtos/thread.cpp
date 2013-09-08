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
