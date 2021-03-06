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

#ifndef WEOS_COMMON_EXCEPTION_HPP
#define WEOS_COMMON_EXCEPTION_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../utility.hpp"
#include "../type_traits.hpp"


// ----=====================================================================----
//     WEOS_EXCEPTION
// ----=====================================================================----

#if defined(WEOS_EXCEPTION_CAN_BE_CAPTURED) && defined(WEOS_EXCEPTION_CONTAINS_LOCATION)
    #define WEOS_EXCEPTION(exc)                                                \
        ::WEOS_NAMESPACE::enable_current_exception(                            \
        ::WEOS_NAMESPACE::enable_exception_info(exc)                           \
              << ::WEOS_NAMESPACE::throw_location(__FILE__, __LINE__, __PRETTY_FUNCTION__))

#elif defined(WEOS_EXCEPTION_CAN_BE_CAPTURED)
    #define WEOS_EXCEPTION(exc)                                                \
        ::WEOS_NAMESPACE::enable_current_exception(exc)

#elif defined(WEOS_EXCEPTION_CONTAINS_LOCATION)
    #define WEOS_EXCEPTION(exc)                                                \
        ::WEOS_NAMESPACE::enable_exception_info(exc)                           \
            << ::WEOS_NAMESPACE::throw_location(__FILE__, __LINE__, __PRETTY_FUNCTION__)

#else
    #define WEOS_EXCEPTION(exc)   exc
#endif


// ----=====================================================================----
//     exception
// ----=====================================================================----

WEOS_BEGIN_NAMESPACE

class exception;

struct throw_location
{
    throw_location(const char* file, int line, const char* function)
        : m_file(file),
          m_line(line),
          m_function(function)
    {
    }

    inline
    const char* file() const { return m_file; }

    inline
    int line() const { return m_line; }

    inline
    const char* function() const { return m_function; }

private:
    const char* m_file;
    int m_line;
    const char* m_function;
};

class exception
{
public:
    exception(const exception& other) noexcept
        : m_file(other.m_file),
          m_line(other.m_line),
          m_function(other.m_function)
    {
    }

    exception(exception&& other) noexcept
        : m_file(other.m_file),
          m_line(other.m_line),
          m_function(other.m_function)
    {
    }

    virtual
    ~exception() throw()
    {
    }

    exception& operator=(const exception& other) noexcept
    {
        m_file = other.m_file;
        m_line = other.m_line;
        m_function = other.m_function;
        return *this;
    }

    exception& operator=(exception&& other) noexcept
    {
        m_file = other.m_file;
        m_line = other.m_line;
        m_function = other.m_function;
        return *this;
    }

    inline
    const char* file() const { return m_file; }

    inline
    int line() const { return m_line; }

    inline
    const char* function() const { return m_function; }

    exception& operator<<(const throw_location& loc)
    {
        m_file = loc.file();
        m_line = loc.line();
        m_function = loc.function();
        return *this;
    }

protected:
    exception() noexcept
        : m_file(""),
          m_line(0),
          m_function("")
    {
    }

private:
    const char* m_file;
    int m_line;
    const char* m_function;
};

namespace weos_detail
{

template <typename TType>
struct ExceptionInfoBase : public TType,
                           public exception
{
    explicit
    ExceptionInfoBase(const TType& exc)
        : TType(exc)
    {
    }

    virtual
    ~ExceptionInfoBase() throw() {}

    ExceptionInfoBase& operator<<(const throw_location& loc)
    {
        *static_cast<exception*>(this) << loc;
        return *this;
    }
};

} // namespace weos_detail

// ----=====================================================================----
//     enable_exception_info
// ----=====================================================================----

template <typename TType>
typename enable_if<std::is_class<typename std::remove_reference<TType>::type>::value
                   && !std::is_base_of<exception,
                                       typename std::remove_reference<TType>::type>::value,
                   weos_detail::ExceptionInfoBase<
                       typename std::remove_reference<TType>::type>
                  >::type
enable_exception_info(TType&& exc)
{
    return weos_detail::ExceptionInfoBase<typename std::remove_reference<TType>::type>(
                std::forward<TType>(exc));
}

template <typename TType>
typename enable_if<!std::is_class<typename std::remove_reference<TType>::type>::value
                   || std::is_base_of<exception,
                                      typename std::remove_reference<TType>::type>::value,
                   TType&&>::type
enable_exception_info(TType&& exc) noexcept
{
    return std::forward<TType>(exc);
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_EXCEPTION_HPP
