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

#ifndef WEOS_KEIL_RL_RTX_HAL_HPP
#define WEOS_KEIL_RL_RTX_HAL_HPP

#include "../config.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

//! Hardware abstraction layer.
//! The hal namespace contains functions and classes which are
//! hardware-specific.
namespace hal
{

//! Returns the current value of the SysTick timer. When the SysTick timer
//! reaches zero, it triggers the RL RTX kernel.
inline std::uint32_t getSysTickValue()
{
    return os_trv - NVIC_ST_CURRENT;
}

//! Returns \p true, if the processor is in an interrupt service routine.
inline bool isInIsr()
{
    return __get_IPSR() != 0;
}

//! Returns \p true, if the processor is in an interrupt service routine or
//! in privileged mode.
inline bool isInIsrOrPrivileged()
{
    return __get_IPSR() != 0 || (__get_CONTROL() & 1) == 0;
}

} // namespace hal

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_RL_RTX_HAL_HPP
