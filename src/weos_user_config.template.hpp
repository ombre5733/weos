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

#ifndef WEOS_USER_CONFIG_HPP
#define WEOS_USER_CONFIG_HPP

// ----=====================================================================----
//     Native operating system configuration
// ----=====================================================================----

// -----------------------------------------------------------------------------
//     C++11
// -----------------------------------------------------------------------------

// Set this macro to make WEOS wrap the native C++11 STL.
// #define WEOS_WRAP_CXX11

// -----------------------------------------------------------------------------
//     ARM Germany CMSIS-RTOS
// -----------------------------------------------------------------------------

// Set this macro to make WEOS wrap Keil's CMSIS-RTOS.
// #define WEOS_WRAP_KEIL_CMSIS_RTOS

#if defined(WEOS_WRAP_KEIL_CMSIS_RTOS)

// The include path for <cmsis_os.h>
#define WEOS_CMSIS_OS_INCLUDE         <cmsis_os.h>
// The include path for <core_cmX.h>
#define WEOS_CMSIS_CORE_CMX_INCLUDE   <core_cm4.h>

// The frequency of the system clock (in Hz).
// \note In ARM's CMSIS RTOS this is the value of OS_CLOCK.
static constexpr unsigned WEOS_SYSTEM_CLOCK_FREQUENCY = 12000000;
// The frequency of the SysTick timer (in Hz).
// \note In ARM's CMSIS RTOS this is the value of (1000000 / OS_TICK).
static constexpr unsigned WEOS_SYSTICK_FREQUENCY = 1000;

#endif // WEOS_WRAP_KEIL_CMSIS_RTOS

// -----------------------------------------------------------------------------
//     Keil RL-RTX
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
//     OSAL
// -----------------------------------------------------------------------------

// Set this macro to make WEOS wrap OSAL.
// #define WEOS_WRAP_OSAL



// ----=====================================================================----
//     Wrapper configuration
// ----=====================================================================----

// -----------------------------------------------------------------------------
//     Assertion handling
// -----------------------------------------------------------------------------

// If this macro is defined, assertions are enabled in the WEOS library.
// By default, the assertion is checked using the assert() function from
// <cassert>.
// #define WEOS_ENABLE_ASSERT

// If this macro is defined, the user has to provide a handler for a failed
// assertion. The function's signature is
//   [[noreturn]] void weos::assert_failed(const char* condition,
//                                         const char* function,
//                                         const char* file, int line);
// Note: If WEOS_ENABLE_ASSERT is not defined, this macro has no effect.
// #define WEOS_CUSTOM_ASSERT_HANDLER

// -----------------------------------------------------------------------------
//     Exception support
// -----------------------------------------------------------------------------

// If this macro is defined, exceptions are enabled in the WEOS library.
// #define WEOS_ENABLE_EXCEPTIONS

// If this macro is defined, the user has to provide the throw_exception()
// function, which has to throw the given exception. This is useful, if the
// exception must be decorated in a user-defined way before it can be thrown.
// The function's signature is
//   [[noreturn]] void weos::throw_exception(const std::exception& e);
// If the macro is not defined, WEOS uses a 'throw'-statement.
// Note: If WEOS_ENABLE_EXCEPTIONS is not defined, this macro has no effect.
// #define WEOS_CUSTOM_THROW_EXCEPTION

// Set this macro, to catch top-level exceptions in threaded contexts.
// There has to be a user-defined function with the signature
//   void weos::unhandled_thread_exception(weos::exception_ptr& e);
// This function is called if an exception is not caught in a threaded function.
// The exception pointer refers to the currently active exception.
// #define WEOS_ENABLE_THREAD_EXCEPTION_HANDLER

// If this macro is defined, the macro WEOS_EXCEPTION(exc) decorates the given
// exception exc such that it can be captured with current_exception. For
// example:
// struct MyException {};
// try
// {
//     throw WEOS_EXCEPTION(MyException());
// }
// catch (...)
// {
//     exception_ptr eptr = current_exception();
// }
// #define WEOS_EXCEPTION_CAN_BE_CAPTURED

// If this macro is defined, the macro WEOS_EXCEPTION(exc) adds the file name,
// line and function name to the given exception exc. This is done by
// deriving the given exception from weos::exception.
// #define WEOS_EXCEPTION_CONTAINS_LOCATION



// ----=====================================================================----
//     Private section.
//     Do not modify the code below.
// ----=====================================================================----

// The version of the WEOS user configuration file. The WEOS library can
// check this version to guarantee the compatibility of the configuration file.
#define WEOS_USER_CONFIG_VERSION   6

#endif // WEOS_USER_CONFIG_HPP
