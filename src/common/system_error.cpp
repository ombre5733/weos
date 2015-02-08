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

#include "system_error.hpp"


WEOS_BEGIN_NAMESPACE

//! The generic error category.
class generic_category_impl : public error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "generic";
    }

    virtual const char* message(int err_val) const
    {
        switch (static_cast<errc>(err_val))
        {
        case errc::invalid_argument:
            return "Invalid argument";
        case errc::no_child_process:
            return "No child process";
        case errc::not_enough_memory:
            return "Not enough memory";
        case errc::operation_not_permitted:
            return "Operation not permitted";
        case errc::resource_deadlock_would_occur:
            return "Resource deadlock would occur";
        default:
            return "Unknown error";
        }
    }
};

const error_category& generic_category()
{
    static generic_category_impl categoryInstance;
    return categoryInstance;
}

WEOS_END_NAMESPACE
