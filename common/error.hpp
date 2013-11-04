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

#ifndef WEOS_COMMON_ERROR_HPP
#define WEOS_COMMON_ERROR_HPP

#include "../config.hpp"

#include <boost/config.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits/integral_constant.hpp>

#include <exception>

namespace weos
{

// ----=====================================================================----
//     error_category
// ----=====================================================================----

//! An error category.
//! The error_category is the base class for all error categories. These
//! categories are used for grouping error values and error conditions. An
//! error category provides a context for an error value and allows to
//! distinguish otherwise ambiguous error values. As an example, imagine that
//! a stepper motor driver and an external flash both can return an error
//! value "1". For the stepper driver this value has the meaning
//! "Overtemperature detected" while for the flash it means
//! "Wrong CRC". Without a context it would not be possible to associate a
//! meaning to the error value "1".
//!
//! A solution is to add two error categories (e.g. a
//! \p stepper_driver_category and a \p flash_category). Instead of returning
//! only an error value, every object returns a pair of
//! (error value, error category), which is called an error code. The error
//! code provides the full context to the caller.
//!
//! Every error category is a singleton and categories are passed around by
//! reference.
class error_category
{
public:
    BOOST_CONSTEXPR error_category() BOOST_NOEXCEPT {}

    virtual ~error_category() BOOST_NOEXCEPT {}

    //! Returns a string representation of an error condition.
    virtual const char* message(int condition) const = 0;

    //! Returns the name of this error category.
    virtual const char* name() const BOOST_NOEXCEPT = 0;

private:
    error_category(const error_category&);
    const error_category& operator= (const error_category&);
};

// ----=====================================================================----
//     error_code
// ----=====================================================================----

//! An error value with context.
//! The error_code is a pair of (error value, error category). Different
//! objects can return the same error value. The context of the error value
//! is provided by the error category which has to derive from error_category.
//!
//! By definition, an error value of zero is defined as success (i.e. no error).
class error_code
{
public:
    //! Creates an error code.
    //! Creates an error code which is defined by the error \p value and the
    //! error \p category.
    error_code(int value, const error_category& category) BOOST_NOEXCEPT
        : m_value(value),
          m_category(&category)
    {
    }

    //! Assigns a new value and category.
    //! Assigns the error \p value and error \p category to this error code.
    void assign(int value, const error_category& category) BOOST_NOEXCEPT
    {
        m_value = value;
        m_category = &category;
    }

    //! Returns the error category of this error code.
    const error_category& category() const BOOST_NOEXCEPT
    {
        return *m_category;
    }

    //! Returns an explanatory message.
    //! Returns an explanatory message. This is a convenience method which
    //! is equivalend to
    //! \code
    //! category().message(value())
    //! \endcode
    const char* message() const
    {
        return category().message(m_value);
    }

    //! Returns the error value of this error code.
    int value() const BOOST_NOEXCEPT
    {
        return m_value;
    }

private:
    //! Error code's value.
    int m_value;
    //! The error category.
    const error_category* m_category;
};

// ----=====================================================================----
//     is_error_code_enum
// ----=====================================================================----

template <typename TType>
struct is_error_code_enum : public boost::false_type
{
};

// ----=====================================================================----
//     system_error
// ----=====================================================================----

//! A system error.
//! A system_error is an exception which wraps an error_code.
class system_error : public std::exception
{
public:
    //! Creates a system error from an error code.
    //! Creates a system error which wraps the error \p code.
    system_error(error_code code)
        : m_errorCode(code)
    {
    }

    //! Creates a system error from a (value, category) pair.
    //! Creates a system error which wraps the error code defined by the
    //! error \p value and the error \p category.
    system_error(int value, const error_category& category)
        : m_errorCode(value, category)
    {
    }

    //! Returns the error code.
    const error_code& code() const BOOST_NOEXCEPT
    {
        return m_errorCode;
    }

    //! Returns an explanatory message.
    virtual const char* what() const BOOST_NOEXCEPT
    {
        return m_errorCode.message();
    }

private:
    //! The error code wrapped by the system error.
    error_code m_errorCode;
};

} // namespace weos

#endif // WEOS_COMMON_ERROR_HPP
