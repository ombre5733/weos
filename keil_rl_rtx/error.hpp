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

#ifndef WEOS_KEIL_RL_RTX_ERROR_HPP
#define WEOS_KEIL_RL_RTX_ERROR_HPP

#include "../config.hpp"
#include "../common/error.hpp"

namespace weos
{

//! Returns the category for RL RTX errors.
const error_category& rl_rtx_category();

namespace rl_rtx_error
{
enum rl_rtx_error_t
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

//! Creates a RL RTX error code.
//! Creates a RL RTX error code whose error value will be set to \p err. The
//! category is the one returned by rl_rtx_category().
inline
weos::error_code make_error_code(rl_rtx_error_t err)
{
    return error_code(err, rl_rtx_category());
}

/*
inline
weos::error_code make_error_condition(rl_rtx_error_t err)
{
    return error_condition(err, rl_rtx_category());
}
*/

} // namespace rl_rtx_error

// Specialization for RL RTX error enum.
template <>
struct is_error_code_enum<rl_rtx_error::rl_rtx_error_t> : public boost::true_type
{
};

} // namespace weos

#endif // WEOS_KEIL_RL_RTX_ERROR_HPP
