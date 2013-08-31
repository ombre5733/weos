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

#ifndef WEOS_KEIL_CMSIS_RTOS_ERROR_HPP
#define WEOS_KEIL_CMSIS_RTOS_ERROR_HPP

#include "../config.hpp"

#include <boost/config.hpp>
#include <boost/utility.hpp>

#include <exception>

namespace weos
{

//! An error category.
//! The error_category is the base class for all error categories. These
//! categories are used for grouping error values and error conditions. An
//! error category provides a context for an error value and allows to
//! distinguish otherwise ambiguous error values. As an example, imagine that
//! a stepper motor driver and an external flash both can return an error
//! value "1". For the stepper driver this value has the meaning
//! "Overtemperature detected" while for the flash it means
//! "Wrong CRC". Without a context it would not be possible to map the error
//! value "1" back to a string.
//!
//! A solution is to add two error categories (e.g. a
//! \p stepper_driver_category and a \p flash_category). Instead of returning
//! only an error value, every module returns a pair of
//! (error value, error category), i.e. an error_code, providing the full
//! context of the error to the caller.
//!
//! Every error category is a singleton and categories are passed around by
//! reference.
class error_category : boost::noncopyable
{
public:
    BOOST_CONSTEXPR error_category() BOOST_NOEXCEPT {}

    virtual ~error_category() BOOST_NOEXCEPT {}

    virtual const char* message(int condition) const = 0;

    //! Returns the name of this error category.
    virtual const char* name() const BOOST_NOEXCEPT = 0;
};

//! An error value with context.
//! The error_code is a pair of (error value, error category). Different
//! modules can return the same error value. The context of the error value
//! is provided by the error category which has to derive from error_category.
//!
//! By definition, an error value of zero is defined as success (i.e. no error).
class error_code
{
public:
    error_code(int value, const error_category& category) BOOST_NOEXCEPT
        : m_value(value),
          m_category(category)
    {
    }

    //! Returns the error category of this error code.
    const error_category& category() const BOOST_NOEXCEPT
    {
        return m_category;
    }

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
    int m_value;
    const error_category& m_category;
};

class system_error : public std::exception
{
public:
};

namespace cmsis_error
{
enum cmsis_error_enum
{
    Resource,
    Isr
};
} // namespace cmsis_error

//! An error category for CMSIS errors.
class cmsis_category_impl : public error_category
{
public:
    virtual const char* name() const
    {
        return "CMSIS";
    }

    //virtual const char* message(int err_val) const;
};

const error_category& cmsis_category()
{
    static cmsis_category_impl categoryInstance;
    return categoryInstance;
}

namespace cmsis_error
{
boost::error_code make_error_code(cmsis_error_enum e)
{
    return boost::error_code(static_cast<int>(e),
                             cmsis_category());
}

boost::error_condition make_error_condition(cmsis_error_enum e)
{
    return boost::error_condition(static_cast<int>(e),
                                  cmsis_category());
}
} // namespace cmsis_error

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_ERROR_HPP
