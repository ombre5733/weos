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

#ifndef WEOS_CONFIG_HPP
#define WEOS_CONFIG_HPP

#if !defined(WEOS_USER_CONFIG)
//#  error "The user config has not been defined."
#define WEOS_USER_CONFIG "user_config.hpp"
#endif

#include WEOS_USER_CONFIG

// ----=====================================================================----
//     C++11
// ----=====================================================================----

#define OSL_IMPLEMENTATION_CXX11
#if defined(OSL_IMPLEMENTATION_CXX11)

// The frequency of the high-resolution timer (in Hz).
#  define OS_CLOCK   12000000
// The time interval between two sys-ticks (in us).
#  define OS_TICK    1000

#endif // OSL_IMPLEMENTATION_CXX11

// ----=====================================================================----
//     Keil CMSIS-RTOS
// ----=====================================================================----

// #define OSL_IMPLEMENTATION_KEIL_CMSIS

// ----=====================================================================----
//     Private section
// ----=====================================================================----

#if defined(OS_IMPLEMENTATION_KEIL_CMSIS)
#  if osCMSIS_RTX != ((4<<16)|70)
#    error "The Keil CMSIS version must be 4.70."
#  endif
#endif

#endif // WEOS_CONFIG_HPP
