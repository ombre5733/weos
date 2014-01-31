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

#ifndef WEOS_USER_CONFIG_HPP
#define WEOS_USER_CONFIG_HPP

// ----=====================================================================----
//     C++11
// ----=====================================================================----

// Set this macro to make WEOS wrap the native C++11 STL.
// #define WEOS_WRAP_CXX11

// ----=====================================================================----
//     Keil CMSIS-RTOS
// ----=====================================================================----

// Set this macro to make WEOS wrap Keil's CMSIS-RTOS.
// #define WEOS_WRAP_KEIL_CMSIS_RTOS

#if defined(WEOS_WRAP_KEIL_CMSIS_RTOS)

// The frequency of the system clock (in Hz).
// \note In Keil's CMSIS RTOS this is the value of OS_CLOCK.
#  define WEOS_SYSTEM_CLOCK_FREQUENCY       12000000
// The frequency of the SysTick timer (in Hz).
// \note In Keil's CMSIS RTOS this is the value of (1000000 / OS_TICK).
#  define WEOS_SYSTICK_FREQUENCY            1000
// The maximum number of threads which can be active concurrently.
#  define WEOS_MAX_NUM_CONCURRENT_THREADS   3

#endif // WEOS_WRAP_KEIL_CMSIS_RTOS

// ----=====================================================================----
//     Keil RL-RTX
// ----=====================================================================----

// Set this macro to make WEOS wrap Keil's RL-RTX.
// #define WEOS_WRAP_KEIL_RL_RTX

#if defined(WEOS_WRAP_KEIL_RL_RTX)

// The frequency of the system clock (in Hz).
// \note In Keil's RL RTX this is the value of OS_CLOCK.
#  define WEOS_SYSTEM_CLOCK_FREQUENCY       12000000
// The frequency of the SysTick timer (in Hz).
// \note In Keil's RL RTX this is the value of (1000000 / OS_TICK).
#  define WEOS_SYSTICK_FREQUENCY            1000
// The maximum number of threads which can be active concurrently.
#  define WEOS_MAX_NUM_CONCURRENT_THREADS   3

#endif // WEOS_WRAP_KEIL_RL_RTX

// ----=====================================================================----
//     General settings
// ----=====================================================================----

// The default storage size of a static_function<>.
// A static_function<> holds a (member) function pointer and the bound
// arguments (the fixed parameters). If the accumulated size of the bound
// arguments exceeds the value below, a compile-time error is generated.
#define WEOS_DEFAULT_STATIC_FUNCTION_SIZE   4 * sizeof(void*)



// If this macro is defined, the user has to provide the throw_exception()
// function. The function's signature is
// void ::weos::throw_exception(const std::exception& e);
// This function must never return.
// #define WEOS_CUSTOM_THROW_EXCEPTION

// If this macro is defined, assertions are enabled in the WEOS library.
// By default, the assertion is checked using the assert() function from
// <cassert>.
// #define WEOS_ENABLE_ASSERT

// If this macro is defined, the user has to provide a handler for a failed
// assertion. The function's signature is
// void ::weos::assert_failed(const char* condition, const char* function,
//                            const char* file, int line);
// Note: If WEOS_ENABLE_ASSERT is not defined, this macro has no effect.
// #define WEOS_CUSTOM_ASSERT_HANDLER

// ----=====================================================================----
//     Private section.
//     Do not modify the code below.
// ----=====================================================================----

// The version of the WEOS user configuration file. The WEOS library can
// check this version to guarantee the compatibility of the configuration file.
#define WEOS_USER_CONFIG_VERSION   2

#endif // WEOS_USER_CONFIG_HPP
