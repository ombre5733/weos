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

#include <exception>

namespace weos
{

class error_code
{
};

class error : public std::exception
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

class cmsis_category_impl : public boost::error_category
{
public:
    virtual const char* name() const
    {
        return "CMSIS";
    }

    virtual std::string message(int err_val) const;
};

const boost::error_category& cmsis_category()
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
