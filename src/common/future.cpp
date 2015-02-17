#include "future.hpp"


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     Error
// ----=====================================================================----

class future_category_impl : public error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "future";
    }

    virtual const char* message(int err_val) const
    {
        switch (static_cast<future_errc>(err_val))
        {
        case future_errc::broken_promise:
            return "broken promise";
        case future_errc::future_already_retrieved:
            return "future already retrieved";
        case future_errc::promise_already_satisfied:
            return "promise already satisfied";
        case future_errc::no_state:
            return "no state";
        default:
            return "Unknown error";
        }
    }
};

const error_category& future_category() noexcept
{
    static future_category_impl categoryInstance;
    return categoryInstance;
}

// ----=====================================================================----
//     Shared state
// ----=====================================================================----

namespace weos_detail
{

// ----=====================================================================----
//     SharedStateBase
// ----=====================================================================----

void SharedStateBase::attachFuture()
{
    unsigned flags = m_flags;
    do
    {
        // Make sure that no future has been attached before.
        if (flags & FutureAttached)
            throw future_error(make_error_code(future_errc::future_already_retrieved));
    } while (!m_flags.compare_exchange_strong(flags, flags | FutureAttached));
}

void SharedStateBase::startSettingValue()
{
    unsigned flags = m_flags;
    do
    {
        // Make sure that no other task has set a value or exception.
        if (flags & BeingSatisfied)
            throw future_error(make_error_code(future_errc::promise_already_satisfied));
    } while (!m_flags.compare_exchange_strong(flags, flags | BeingSatisfied));
}

void SharedStateBase::setException(exception_ptr exc)
{
    startSettingValue();
    m_exception = exc;
    m_flags |= Ready;
    m_cv.notify();
}

void SharedStateBase::setValue()
{
    startSettingValue();
    m_flags |= Ready;
    m_cv.notify();
}

void SharedStateBase::copyValue()
{
    wait();
    if (m_exception != nullptr)
        rethrow_exception(m_exception);
}

void SharedStateBase::wait()
{
    while (!(m_flags & Ready))
        m_cv.wait();
}

} // namespace weos_detail

// ----=====================================================================----
//     future<void>
// ----=====================================================================----

future<void>::future(weos_detail::SharedStateBase* state)
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
    unique_ptr<weos_detail::SharedStateBase,
               weos_detail::SharedStateBaseDeleter> state(m_state);
    m_state = nullptr;
    state->copyValue();
}

// ----=====================================================================----
//     promise<void>
// ----=====================================================================----

promise<void>::promise()
    : m_state(new weos_detail::SharedStateBase)
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