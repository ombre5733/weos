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

#ifndef WEOS_CMSIS_RTOS_SYSTEM_ERROR_HPP
#define WEOS_CMSIS_RTOS_SYSTEM_ERROR_HPP

#include "core.hpp"

#include "../_common/system_error.hpp"


WEOS_BEGIN_NAMESPACE

//! Returns the category for CMSIS errors.
const error_category& cmsis_category();

namespace cmsis_error
{
//! An enumeration of CMSIS error codes.
enum cmsis_error_t
{
    osOK                   =    0,
    osEventSignal          = 0x08,
    osEventMessage         = 0x10,
    osEventMail            = 0x20,
    osEventTimeout         = 0x40,
    osErrorParameter       = 0x80,
    osErrorResource        = 0x81,
    osErrorTimeoutResource = 0xC1,
    osErrorISR             = 0x82,
    osErrorISRRecursive    = 0x83,
    osErrorPriority        = 0x84,
    osErrorNoMemory        = 0x85,
    osErrorValue           = 0x86,
    osErrorOS              = 0xFF
};

} // namespace cmsis_error

// Specialization for CMSIS error enum.
template <>
struct is_error_code_enum<cmsis_error::cmsis_error_t> : public true_type
{
};

namespace cmsis_error
{

//! Creates a CMSIS error code.
//! Creates a CMSIS error code whose error value will be set to \p err. The
//! category is the one returned by cmsis_category().
inline
weos::error_code make_error_code(cmsis_error_t err)
{
    return error_code(static_cast<int>(err), cmsis_category());
}

/*
inline
weos::error_code make_error_condition(cmsis_error_t err)
{
    return error_condition(err, cmsis_category());
}
*/

} // namespace cmsis_error

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_SYSTEM_ERROR_HPP
