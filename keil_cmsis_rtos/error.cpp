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

#include "error.hpp"

namespace weos
{

//! An error category for CMSIS errors.
class cmsis_category_impl : public error_category
{
public:
    virtual const char* name() const BOOST_NOEXCEPT
    {
        return "CMSIS";
    }

    virtual const char* message(int err_val) const
    {
        switch (err_val)
        {
            default:
                // Not an error.
                return "";
            case osErrorParameter:
                return "A parameter was incorrect.";
            case osErrorResource:
                return "A resource was not available.";
            case osErrorTimeoutResource:
                return "A resource was not available before the timeout.";
            case osErrorISR:
            case osErrorISRRecursive:
                return "The function cannot be called from an interrupt.";
            case osErrorPriority:
                return "The priority is illegal.";
            case osErrorNoMemory:
                return "Could not reserve memory.";
            case osErrorValue:
                return "A parameter is out of range.";
            case osErrorOS:
                return "Unspecified error.";
        }
    }
};

const error_category& cmsis_category()
{
    static cmsis_category_impl categoryInstance;
    return categoryInstance;
}

} // namespace weos
