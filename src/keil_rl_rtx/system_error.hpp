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

#ifndef WEOS_KEIL_RL_RTX_SYSTEM_ERROR_HPP
#define WEOS_KEIL_RL_RTX_SYSTEM_ERROR_HPP

#include "core.hpp"

#include "../common/system_error.hpp"


WEOS_BEGIN_NAMESPACE

//! Returns the category for RL RTX errors.
const error_category& rl_rtx_category();

namespace rl_rtx_error
{
enum rl_rtx_error_t
{
    None  = OS_R_OK,
    Timeout = OS_R_TMO,
    Error = OS_R_NOK
};

//! Creates a RL RTX error code.
//! Creates a RL RTX error code whose error value will be set to \p err. The
//! category is the one returned by rl_rtx_category().
inline
WEOS_NAMESPACE::error_code make_error_code(rl_rtx_error_t err)
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
struct is_error_code_enum<rl_rtx_error::rl_rtx_error_t> : public true_type
{
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_RL_RTX_SYSTEM_ERROR_HPP
