#include "future.hpp"

#include "../memory.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_future_detail
{

void SharedState::attachFuture()
{
    unsigned flags = m_flags;
    do
    {
        // Make sure that no future has been attached before.
        if (flags & FutureAttached)
            throw future_error(make_error_code(future_errc::future_already_retrieved));
    } while (!m_flags.compare_exchange_strong(flags, flags | FutureAttached));
}

void SharedState::setException(exception_ptr exc)
{
    unsigned flags = m_flags;
    do
    {
        // Make sure that no other task has set a value or exception.
        if (flags & BeingSatisfied)
            throw future_error(make_error_code(future_errc::promise_already_satisfied));
    } while (!m_flags.compare_exchange_strong(flags, flags | BeingSatisfied));

    m_exception = exc;
    m_flags |= Ready;
    m_cv.notify();
}

void SharedState::setValue()
{
    unsigned flags = m_flags;
    do
    {
        // Make sure that no other task has set a value or exception.
        if (flags & BeingSatisfied)
            throw future_error(make_error_code(future_errc::promise_already_satisfied));
    } while (!m_flags.compare_exchange_strong(flags, flags | BeingSatisfied));

    m_flags |= Ready;
    m_cv.notify();
}

void SharedState::value()
{
    wait();
    if (m_exception != nullptr)
        rethrow_exception(m_exception);
}

void SharedState::wait()
{
    while (!(m_flags & Ready))
        m_cv.wait();
}

struct SharedStateDeleter
{
    void operator()(SharedState* state)
    {
        state->decReferenceCount();
    }
};

} // namespace weos_future_detail

// ----=====================================================================----
//     future<void>
// ----=====================================================================----

future<void>::future(weos_future_detail::SharedState* state)
    : m_state(state)
{
    m_state->attachFuture();
    m_state->incReferenceCount();
}

future<void>::~future()
{
    if (m_state)
        m_state->decReferenceCount();
}

void future<void>::get()
{
    unique_ptr<weos_future_detail::SharedState,
               weos_future_detail::SharedStateDeleter> ptr(m_state);
    m_state = nullptr;
    ptr->value();
}

// ----=====================================================================----
//     promise<void>
// ----=====================================================================----

promise<void>::promise()
    : m_state(new weos_future_detail::SharedState)
{
}

promise<void>::~promise()
{
    if (m_state)
    {
        // If no value or exception has been set in the shared state and
        // at least one future is still attached to it, we have to
        // signal a broken promise.
        if (!m_state->isReady() && m_state->referenceCount() > 1)
        {
            m_state->setException(make_exception_ptr(
                                       future_error(make_error_code(future_errc::broken_promise))));
        }

        m_state->decReferenceCount();
    }
}

future<void> promise<void>::get_future()
{
    if (m_state == nullptr)
        throw future_error(make_error_code(future_errc::no_state));
    return future<void>(m_state);
}

void promise<void>::set_value()
{
    if (m_state == nullptr)
        throw future_error(make_error_code(future_errc::no_state));
    m_state->setValue();
}

void promise<void>::set_exception(exception_ptr exc)
{
    if (m_state == nullptr)
        throw future_error(make_error_code(future_errc::no_state));
    m_state->setException(exc);
}

WEOS_END_NAMESPACE
