/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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

#ifndef WEOS_COMMON_EXCEPTION_HPP
#define WEOS_COMMON_EXCEPTION_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../atomic.hpp"
#include "../intrusive_ptr.hpp"
#include "../type_traits.hpp"

#include <new>
#include <stdexcept>


WEOS_BEGIN_NAMESPACE

//! A pointer-like type to access an exception.
//!
//! The exception_ptr is a pointer-like type, which manages an exception that
//! has been thrown and captured with current_exception().
class exception_ptr;

//! Captures the current exception for later use.
//!
//! The currently active function is captured and returned in an exception_ptr.
//! Later the exception can be re-thrown or disposed.
//!
//! \code{.cpp}
//! struct MyException {};
//! exception_ptr exc;
//! try
//! {
//!     throw enable_current_exception(MyException());
//! }
//! catch (...)
//! {
//!     exc = current_exception();
//! }
//!
//! // ... do something ...
//!
//! if (exc)
//!     rethrow_exception(exc);
//! \endcode
//!
//! Note that the support for current_exception() has to be
//! enabled at the throw site. This is done by calling
//! enable_current_exception() on the object to be thrown as shown in the
//! code above.
//!
//! If enable_current_exception(), it might not be possible to capture the
//! the correct type of the exception. This is relevant, if the exception
//! is rethrown through rethrow_exception() and caught in another catch
//! block. Consider the following example:
//!
//! \code{.cpp}
//! struct MyException {};
//! exception_ptr exc;
//! try
//! {
//!     throw MyException();
//!     //OR: throw enable_current_exception(MyException());
//! }
//! catch (...)
//! {
//!     exc = current_exception();
//! }
//!
//! try
//! {
//!     rethrow_exception(exc);
//! }
//! catch (MyException&)
//! {
//!     // With enable_current_exception(), execution resumes here.
//! }
//! catch (...)
//! {
//!     // Without enable_current_exception(), execution might resume here.
//! }
//! \endcode
//! Have a look at the second try-catch block. Without
//! enable_current_exception() it might be that the catch-all block
//! is entered instead of the more specific one because current_exception() was
//! not able to capture the correct exception type.
exception_ptr current_exception() WEOS_NOEXCEPT;

//! Rethrows a captured exception.
//!
//! Rethrows the exception \p eptr, which must have been captured with
//! current_exception().
void rethrow_exception(const exception_ptr& eptr);

//! \fn template <typename TType> /*unspecified*/ enable_current_exception(TType&& t)
//! Enables capturing the exception object \p t through a call to
//! current_exception().


#define WEOS_THROW_EXCEPTION(exc)                                              \
    throw WEOS_NAMESPACE::enable_current_exception(exc)

// ----=====================================================================----
//     exception
// ----=====================================================================----

class exception
{
public:
    virtual ~exception() throw() = 0;

protected:
    exception()
        : m_errorInfoList()
    {
    }

private:
    mutable void* m_errorInfoList;
};

inline
exception::~exception() throw()
{
}

namespace detail_exception
{
void cloneErrorInfoList(const exception* src, exception* dest)
{
}

void cloneErrorInfoList(const void* /*src*/, void* /*dest*/)
{
}

} // namespace detail_exception

// ----=====================================================================----
//     CaptureableException
// ----=====================================================================----

namespace detail_exception
{

// The base class for all captureable exceptions.
// A captureable exception can be cloned and rethrown.
class CaptureableExceptionBase
{
public:
    CaptureableExceptionBase() WEOS_NOEXCEPT
        : m_refCount(0)
    {
    }

    CaptureableExceptionBase(const CaptureableExceptionBase&) WEOS_NOEXCEPT
        : m_refCount(0)
    {
    }

    virtual const CaptureableExceptionBase* clone() const = 0;

    virtual void rethrow() const = 0;

private:
    mutable WEOS_NAMESPACE::atomic<int> m_refCount;

    CaptureableExceptionBase& operator=(const CaptureableExceptionBase&);

    friend void intrusive_ptr_add_ref(const CaptureableExceptionBase* exc);
    friend void intrusive_ptr_release_ref(const CaptureableExceptionBase* exc);
};

inline
void intrusive_ptr_add_ref(const CaptureableExceptionBase* exc) WEOS_NOEXCEPT
{
    ++exc->m_refCount;
}

inline
void intrusive_ptr_release_ref(const CaptureableExceptionBase* exc) WEOS_NOEXCEPT
{
    if (--exc->m_refCount == 0)
        delete exc;
}

template <typename TType>
class CaptureableException : public TType,
                             public virtual CaptureableExceptionBase
{
    static const bool derivesTwice = is_base_of<CaptureableExceptionBase,
                                                typename decay<TType>::type>::value;
public:
    static_assert(!derivesTwice, "Derived twice from CaptureableExceptionBase");

    explicit CaptureableException(const TType& other)
        : TType(other)
    {
        cloneErrorInfoList(&other, this);
    }

private:
    virtual const CaptureableException* clone() const /*override*/
    {
        return new CaptureableException(*this);
    }

    virtual void rethrow() const /*override*/
    {
        throw *this;
    }
};

} // namespace detail_exception

// ----=====================================================================----
//     enable_current_exception
// ----=====================================================================----

template <typename TType>
typename enable_if<is_base_of<detail_exception::CaptureableExceptionBase,
                              typename remove_reference<TType>::type>::value,
                   TType&>::type
enable_current_exception(TType& exc) WEOS_NOEXCEPT
{
    return exc;
}

template <typename TType>
typename enable_if<is_base_of<detail_exception::CaptureableExceptionBase,
                              typename remove_reference<TType>::type>::value,
                   const TType&>::type
enable_current_exception(const TType& exc) WEOS_NOEXCEPT
{
    return exc;
}

template <typename TType>
typename enable_if<!is_base_of<detail_exception::CaptureableExceptionBase,
                               typename remove_reference<TType>::type>::value,
                   detail_exception::CaptureableException<TType> >::type
enable_current_exception(const TType& exc)
{
    return detail_exception::CaptureableException<TType>(exc);
}

// ----=====================================================================----
//     exception_ptr
// ----=====================================================================----

class exception_ptr;

namespace detail_exception
{
exception_ptr getCurrentException();
} // namespace detail_exception

class exception_ptr
{
public:
    typedef intrusive_ptr<const detail_exception::CaptureableExceptionBase> pointer_type;

    exception_ptr() WEOS_NOEXCEPT
    {
    }

    exception_ptr(nullptr_t) WEOS_NOEXCEPT
    {
    }

    explicit exception_ptr(const pointer_type& ptr)
        : m_capturedException(ptr)
    {
    }

    exception_ptr(const exception_ptr& other) WEOS_NOEXCEPT
        : m_capturedException(other.m_capturedException)
    {
    }

    exception_ptr& operator=(const exception_ptr& other) WEOS_NOEXCEPT
    {
        m_capturedException = other.m_capturedException;
        return *this;
    }

    exception_ptr& operator=(nullptr_t) WEOS_NOEXCEPT
    {
        m_capturedException = nullptr;
        return *this;
    }

    bool operator==(const exception_ptr& other) const
    {
        return m_capturedException == other.m_capturedException;
    }

    bool operator!=(const exception_ptr& other) const
    {
        return m_capturedException != other.m_capturedException;
    }

    /*explicit*/ operator bool() const
    {
        return m_capturedException;
    }

private:
    pointer_type m_capturedException;

    friend void rethrow_exception(const exception_ptr&);
};

inline
bool operator==(const exception_ptr& eptr, nullptr_t)
{
    return !eptr;
}

inline
bool operator==(nullptr_t, const exception_ptr& eptr)
{
    return !eptr;
}

inline
bool operator!=(const exception_ptr& eptr, nullptr_t)
{
    return !!eptr;
}

inline
bool operator!=(nullptr_t, const exception_ptr& eptr)
{
    return !!eptr;
}

// ----=====================================================================----
//     Helpers for current_exception()
// ----=====================================================================----

namespace detail_exception
{

template <typename TType>
exception_ptr cloneException(const TType& exc)
{
    try
    {
        throw enable_current_exception(exc);
    }
    catch (...)
    {
        return current_exception();
    }
}

template <typename TStdException>
class StdExceptionWrapper : public TStdException,
                            public WEOS_NAMESPACE::exception
{
public:
    explicit StdExceptionWrapper(const TStdException& exc)
        : TStdException(exc)
    {
    }

    StdExceptionWrapper(const TStdException& stdExc,
                        const WEOS_NAMESPACE::exception& weosExc)
        : TStdException(stdExc),
          WEOS_NAMESPACE::exception(weosExc)
    {
    }

    virtual ~StdExceptionWrapper() throw() {}
};

template <typename TStdException>
inline
exception_ptr wrapStdException(const TStdException& exc)
{
    if (const WEOS_NAMESPACE::exception* weosExc
            = dynamic_cast<const WEOS_NAMESPACE::exception*>(&exc))
    {
        return cloneException(StdExceptionWrapper<TStdException>(exc, *weosExc));
    }
    else
    {
        return cloneException(StdExceptionWrapper<TStdException>(exc));
    }
}

struct BadAlloc : public WEOS_NAMESPACE::exception,
                  public std::bad_alloc
{
    virtual ~BadAlloc() throw() {}
};

struct BadException : public WEOS_NAMESPACE::exception,
                      public std::bad_exception
{
    virtual ~BadException() throw() {}
};

struct UnknownStdException : public WEOS_NAMESPACE::exception,
                             public std::exception
{
    virtual ~UnknownStdException() throw() {}
};

struct UnknownException : public WEOS_NAMESPACE::exception,
                          public std::exception
{
    virtual ~UnknownException() throw() {}
};

inline
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

template <typename TException>
struct StaticExceptionFactory
{
    static const exception_ptr eptr;

    static exception_ptr get()
    {
        TException exc;
        return exception_ptr(exception_ptr::pointer_type(
                                 new CaptureableException<TException>(exc)));
    }
};

template <typename TException>
const exception_ptr StaticExceptionFactory<TException>::eptr
    = StaticExceptionFactory<TException>::get();

} // namespace detail_exception

// ----=====================================================================----
//     current_exception
// ----=====================================================================----

inline
exception_ptr current_exception() WEOS_NOEXCEPT
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

// ----=====================================================================----
//     rethrow_exception
// ----=====================================================================----

void rethrow_exception(const exception_ptr& eptr)
{
    WEOS_ASSERT(eptr != nullptr);
    eptr.m_capturedException->rethrow();
    WEOS_ASSERT(false);
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_EXCEPTION_HPP
