/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#include "../exception.hpp"

#include <list>
#include <stdexcept>


struct __cxa_eh_globals;
extern "C" __cxa_eh_globals* __cxa_get_globals();


namespace std
{

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
//     Exception registration
// ----=====================================================================----

namespace weos_detail
{

static
std::list<ExceptionConverterBase*> g_exceptionConverterRegistry;

void registerConverter(ExceptionConverterBase* converter)
{
    g_exceptionConverterRegistry.push_front(converter);
}

static
std::exception_ptr tryConvert(const std::exception& exc)
{
    for (auto converter : g_exceptionConverterRegistry)
    {
        std::exception_ptr eptr = converter->doConvert(exc);
        if (eptr)
            return eptr;
    }
    return std::exception_ptr();
}

} // namespace weos_detail

// ----=====================================================================----
//     current_exception
// ----=====================================================================----

namespace weos_detail
{

exception_ptr getCurrentException()
{
    try
    {
        throw;
    }
    catch (WEOS_NAMESPACE::weos_detail::CaptureableExceptionBase& exc)
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

    catch (std::domain_error& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::invalid_argument& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::length_error& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::out_of_range& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::logic_error& exc)
    {
        return wrapStdException(exc);
    }

    catch (std::overflow_error& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::range_error& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::underflow_error& exc)
    {
        return wrapStdException(exc);
    }
    catch (std::runtime_error& exc)
    {
        return wrapStdException(exc);
    }

    catch (std::exception& exc)
    {
        if (auto ptr = tryConvert(exc))
            return ptr;

        return cloneException(UnknownStdException());
    }

    catch (...)
    {
        return cloneException(UnknownException());
    }
}

} // namespace weos_detail

exception_ptr current_exception() noexcept
{
    exception_ptr result;
    try
    {
        result = weos_detail::getCurrentException();
    }
    catch (std::bad_alloc&)
    {
        result = weos_detail::StaticExceptionFactory<weos_detail::BadAlloc>::eptr;
    }
    catch (...)
    {
        result = weos_detail::StaticExceptionFactory<weos_detail::BadException>::eptr;
    }
    return result;
}

} // namespace std
