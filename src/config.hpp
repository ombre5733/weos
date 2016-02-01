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

#ifndef WEOS_CONFIG_HPP
#define WEOS_CONFIG_HPP

// If the compile switch WEOS_USER_CONFIG is set, it points to the user's
// configuration file. If the switch is not set, we assume that the
// user configuration is somewhere in the path.
#if defined(WEOS_USER_CONFIG)
    #include WEOS_USER_CONFIG
#else
    #include "weos_user_config.hpp"
#endif // WEOS_USER_CONFIG

// Check the version of the user configuration file.
#if WEOS_USER_CONFIG_VERSION != 7
    #error "Version 7 of the WEOS user configuration is required."
#endif // WEOS_USER_CONFIG_VERSION


// ----=====================================================================----
//     Validation
// ----=====================================================================----

// Include the CMSIS header file if necessary.
#if defined(WEOS_WRAP_CXX11)
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#else
    #error "No native OS has been defined in the user configuration file."
#endif // WEOS_WRAP_KEIL_RL_RTX


// ----=====================================================================----
//     Namespace
// ----=====================================================================----

#define WEOS_NAMESPACE         weos
#define WEOS_BEGIN_NAMESPACE   namespace WEOS_NAMESPACE {
#define WEOS_END_NAMESPACE     }


// ----=====================================================================----
//     Assertion handling
// ----=====================================================================----

#if defined(WEOS_ENABLE_ASSERT)

    #if defined(WEOS_CUSTOM_ASSERT_HANDLER)
        WEOS_BEGIN_NAMESPACE

        // This is only a declaration - the definition has to be provided
        // by the user.
        [[noreturn]]
        void assert_failed(const char* condition, const char* function,
                           const char* file, int line);

        WEOS_END_NAMESPACE

        #define WEOS_ASSERT(cond)                                              \
            do {                                                               \
                if (!(cond))                                                   \
                    WEOS_NAMESPACE::assert_failed(#cond, __PRETTY_FUNCTION__,  \
                                                  __FILE__, __LINE__);         \
            } while (0)
    #else
        #include <cassert>
        #define WEOS_ASSERT(cond)   assert(cond)
    #endif // WEOS_CUSTOM_ASSERT_HANDLER

#else

    #define WEOS_ASSERT(cond)   ((void)0)

#endif // WEOS_ENABLE_ASSERT


// ----=====================================================================----
//     Exception support
// ----=====================================================================----

#if defined(WEOS_ENABLE_EXCEPTIONS)
    #if !defined(WEOS_CUSTOM_THROW_EXCEPTION)
        WEOS_BEGIN_NAMESPACE

        template <typename ExceptionT>
        [[noreturn]] inline
        void throw_exception(const ExceptionT& exception)
        {
            throw exception;
        }

        WEOS_END_NAMESPACE
    #else
        #include <exception>

        WEOS_BEGIN_NAMESPACE

        // This is only a declaration - the definition has to be provided
        // by the user.
        [[noreturn]] void throw_exception(const std::exception& exception);

        WEOS_END_NAMESPACE
    #endif // WEOS_CUSTOM_THROW_EXCEPTION

    #define WEOS_THROW_SYSTEM_ERROR(err, msg)                                  \
        WEOS_NAMESPACE::throw_exception(system_error(err))

#elif defined(WEOS_ENABLE_ASSERT)

    #define WEOS_THROW_SYSTEM_ERROR(err, msg)   WEOS_ASSERT(0 && msg)

#else

    #define WEOS_THROW_SYSTEM_ERROR(err, msg)   while(1);

#endif // WEOS_ENABLE_EXCEPTIONS

#endif // WEOS_CONFIG_HPP
