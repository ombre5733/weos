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

#include "exception_impl_armcc.hpp"


struct __cxa_eh_globals;
extern "C" __cxa_eh_globals* __cxa_get_globals();


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     uncaught_exceptions
// ----=====================================================================----

int uncaught_exceptions() noexcept
{
    return *reinterpret_cast<int*>(static_cast<void*>(__cxa_get_globals()));
}

// ----=====================================================================----
//     rethrow_exception
// ----=====================================================================----

/*TODO: [[noreturn]]*/
void rethrow_exception(const exception_ptr& eptr)
{
    WEOS_ASSERT(eptr != nullptr);
    eptr.m_capturedException->rethrow();
    WEOS_ASSERT(false);
}

// ----=====================================================================----
//     nested_exception
// ----=====================================================================----

nested_exception::nested_exception() noexcept
    : m_nestedException(current_exception())
{
}

/*TODO: [[noreturn]]*/
void nested_exception::rethrow_nested() const
{
    if (m_nestedException)
        rethrow_exception(m_nestedException);

    // There was no nested exception.
    std::terminate();
}

// ----=====================================================================----
//     current_exception
// ----=====================================================================----

namespace detail_exception
{

exception_ptr getCurrentException()
{
    try
    {
        throw;
    }
    catch (detail_exception::CaptureableExceptionBase& exc)
    {
        // Note: The clone() method can throw std::bad_alloc. This exception
        // is caught in the caller.
        return exception_ptr(exception_ptr::pointer_type(exc.clone()));
    }

    catch (std::bad_alloc& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::bad_exception& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::exception& exc)
    {
        return cloneException(UnknownStdException());
    }

    catch (...)
    {
        return cloneException(UnknownException());
    }
}

} // namespace detail_exception

exception_ptr current_exception() noexcept
{
    exception_ptr result;
    try
    {
        result = detail_exception::getCurrentException();
    }
    catch (std::bad_alloc&)
    {
        result = detail_exception::StaticExceptionFactory<detail_exception::BadAlloc>::eptr;
    }
    catch (...)
    {
        result = detail_exception::StaticExceptionFactory<detail_exception::BadException>::eptr;
    }
    return result;
}

WEOS_END_NAMESPACE
