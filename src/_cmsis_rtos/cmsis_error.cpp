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

#include "cmsis_error.hpp"


WEOS_BEGIN_NAMESPACE

//! An error category for CMSIS errors.
class cmsis_category_impl : public std::error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "CMSIS";
    }

    virtual const char* message(int err_val) const
    {
        switch (err_val)
        {
            case cmsis_error::osOK:
                return ""; // Not an error.
            case cmsis_error::osErrorParameter:
                return "A parameter was incorrect.";
            case cmsis_error::osErrorResource:
                return "A resource was not available.";
            case cmsis_error::osErrorTimeoutResource:
                return "A resource was not available before the timeout.";
            case cmsis_error::osErrorISR:
            case cmsis_error::osErrorISRRecursive:
                return "The function cannot be called from an interrupt.";
            case cmsis_error::osErrorPriority:
                return "The priority is illegal.";
            case cmsis_error::osErrorNoMemory:
                return "Could not reserve memory.";
            case cmsis_error::osErrorValue:
                return "A parameter is out of range.";
            default:
            case cmsis_error::osErrorOS:
                return "Unspecified error.";
        }
    }
};

const std::error_category& cmsis_category()
{
    static cmsis_category_impl categoryInstance;
    return categoryInstance;
}

WEOS_END_NAMESPACE
