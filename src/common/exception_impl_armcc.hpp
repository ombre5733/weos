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

#ifndef WEOS_COMMON_EXCEPTION_IMPL_ARMCC_HPP
#define WEOS_COMMON_EXCEPTION_IMPL_ARMCC_HPP


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
exception_ptr current_exception() noexcept;

//! Rethrows a captured exception.
//!
//! Rethrows the exception \p eptr, which must have been captured with
//! current_exception().
/*TODO:[[noreturn]]*/
void rethrow_exception(const exception_ptr& eptr);

//! \fn template <typename TType> /*unspecified*/ enable_current_exception(TType&& t)
//! Enables capturing the exception object \p t through a call to
//! current_exception().



//! A mixin in order to allow nesting exceptions.
class nested_exception;

//! \fn template <typename TType> [[noreturn]] void throw_with_nested(TType&& exc);
//! Throws a new exception in which the current exception is nested.

//! \fn template <typename TType> void rethrow_if_nested(const TType& exc);

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
    CaptureableExceptionBase() noexcept
        : m_refCount(0)
    {
    }

    CaptureableExceptionBase(const CaptureableExceptionBase&) noexcept
        : m_refCount(0)
    {
    }

    virtual const CaptureableExceptionBase* clone() const = 0;

    /*TODO: [[noreturn]]*/
    virtual void rethrow() const = 0;

private:
    mutable WEOS_NAMESPACE::atomic<int> m_refCount;

    CaptureableExceptionBase& operator=(const CaptureableExceptionBase&);

    friend void intrusive_ptr_add_ref(const CaptureableExceptionBase* exc) noexcept;
    friend void intrusive_ptr_release_ref(const CaptureableExceptionBase* exc) noexcept;
};

inline
void intrusive_ptr_add_ref(const CaptureableExceptionBase* exc) noexcept
{
    ++exc->m_refCount;
}

inline
void intrusive_ptr_release_ref(const CaptureableExceptionBase* exc) noexcept
{
    if (--exc->m_refCount == 0)
        delete exc;
}

template <typename TType>
class CaptureableException : public TType,
                             public CaptureableExceptionBase
{
    static const bool derivesTwice = is_base_of<CaptureableExceptionBase,
                                                typename decay<TType>::type>::value;
public:
    static_assert(!derivesTwice, "Derived twice from CaptureableExceptionBase");

    // TODO: use perfect forwarding here
    explicit CaptureableException(const TType& other)
        : TType(other)
    {
        cloneErrorInfoList(&other, this);
    }

private:
    virtual const CaptureableException* clone() const override
    {
        return new CaptureableException(*this);
    }

    /*TODO: [[noreturn]]*/
    virtual void rethrow() const override
    {
        throw *this;
    }
};

} // namespace detail_exception

// ----=====================================================================----
//     enable_current_exception
// ----=====================================================================----

template <typename TType>
typename enable_if<is_class<typename remove_reference<TType>::type>::value
                   && !is_base_of<detail_exception::CaptureableExceptionBase,
                                  typename remove_reference<TType>::type>::value,
                   detail_exception::CaptureableException<
                       typename remove_reference<TType>::type>
                  >::type
enable_current_exception(TType&& exc)
{
    return detail_exception::CaptureableException<typename remove_reference<TType>::type>(
                WEOS_NAMESPACE::forward<TType>(exc));
}

template <typename TType>
typename enable_if<!is_class<typename remove_reference<TType>::type>::value
                   || is_base_of<detail_exception::CaptureableExceptionBase,
                                 typename remove_reference<TType>::type>::value,
                   TType&&>::type
enable_current_exception(TType&& exc) noexcept
{
    return WEOS_NAMESPACE::forward<TType>(exc);
}

// ----=====================================================================----
//     exception_ptr
// ----=====================================================================----

class exception_ptr
{
public:
    typedef intrusive_ptr<const detail_exception::CaptureableExceptionBase> pointer_type;

    exception_ptr() noexcept
    {
    }

    exception_ptr(nullptr_t) noexcept
    {
    }

    explicit exception_ptr(const pointer_type& ptr)
        : m_capturedException(ptr)
    {
    }

    exception_ptr(const exception_ptr& other) noexcept
        : m_capturedException(other.m_capturedException)
    {
    }

    exception_ptr& operator=(const exception_ptr& other) noexcept
    {
        m_capturedException = other.m_capturedException;
        return *this;
    }

    exception_ptr& operator=(nullptr_t) noexcept
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

    explicit operator bool() const
    {
        return !!m_capturedException;
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
//     nested_exception
// ----=====================================================================----

class nested_exception
{
public:
    nested_exception() noexcept;

    nested_exception(const nested_exception&) noexcept = default;

    nested_exception& operator=(const nested_exception&) noexcept = default;

    virtual ~nested_exception() = default;

    /*TODO: [[noreturn]]*/
    void rethrow_nested() const;

    exception_ptr nested_ptr() const noexcept
    {
        return m_nestedException;
    }

private:
    exception_ptr m_nestedException;
};

namespace detail_exception
{

template <typename TType>
struct NestedException : public TType,
                         public nested_exception
{
    explicit NestedException(const TType& exception)
        : TType(exception)
    {
    }
};

} // namespace detail_exception

template <typename TType>
[[noreturn]]
void throw_with_nested(TType&& exc,
                       typename enable_if<
                           is_class<typename remove_reference<TType>::type>::value
                           && !is_base_of<nested_exception, typename remove_reference<TType>::type>::value
                       >::type* = 0)
{
    throw detail_exception::NestedException<typename remove_reference<TType>::type>(
                WEOS_NAMESPACE::forward<TType>(exc));
}

template <typename TType>
[[noreturn]]
void throw_with_nested(TType&& exc,
                       typename enable_if<
                           !is_class<typename remove_reference<TType>::type>::value
                           || is_base_of<nested_exception, typename remove_reference<TType>::type>::value
                       >::type* = 0)
{
    throw WEOS_NAMESPACE::forward<TType>(exc);
}

template <typename TType>
void rethrow_if_nested(const TType& exc);

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_EXCEPTION_IMPL_ARMCC_HPP
