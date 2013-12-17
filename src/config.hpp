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

// If the compile switch WEOS_USER_CONFIG is set, it points to the user's
// configuration file. If the switch is not set, we assume that the
// user configuration is somewhere in the path.
#if defined(WEOS_USER_CONFIG)
#  include WEOS_USER_CONFIG
#else
#  include "weos_user_config.hpp"
#endif // WEOS_USER_CONFIG

// Check the version of the user configuration file.
#if WEOS_USER_CONFIG_VERSION != 1
#  error "Version 1 of the WEOS user configuration is required."
#endif // WEOS_USER_CONFIG_VERSION

// -----------------------------------------------------------------------------
// Compatibility checks
// -----------------------------------------------------------------------------

// Include the CMSIS header file if necessary.
#if defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#  include "3rdparty/keil_cmsis_rtos/INC/cmsis_os.h"
#  if osCMSIS_RTX != ((4<<16) | 70)
#    error "The Keil CMSIS RTOS version must be 4.70."
#  endif
#endif // WEOS_WRAP_KEIL_CMSIS_RTOS

// Include the RL RTX header file if necessary.
#if defined(WEOS_WRAP_KEIL_RL_RTX)
#  include "3rdparty/keil_rl_rtx/INC/RTL.h"
#  if __RL_ARM_VER != 472
#    error "The Keil RL RTX version must be 4.72."
#  endif
#endif // WEOS_WRAP_KEIL_RL_RTX

// -----------------------------------------------------------------------------
// Exception handling
// -----------------------------------------------------------------------------

#if !defined(WEOS_CUSTOM_THROW_EXCEPTION)
    namespace weos
    {
        template <typename ExceptionT>
        /*BOOST_ATTRIBUTE_NORETURN*/ inline void throw_exception(const ExceptionT& e)
        {
            throw e;
        }
    } // namespace weos
#else
#   include <exception>
    namespace weos
    {
        // This is only a declaration - the definition has to be provided by the user.
        void throw_exception(const std::exception& e);
     } // namespace weos
#endif // WEOS_CUSTOM_THROW_EXCEPTION

// -----------------------------------------------------------------------------
// Assertion handling
// -----------------------------------------------------------------------------

#if defined(WEOS_ENABLE_ASSERT)
#  if defined(WEOS_CUSTOM_ASSERT_HANDLER)
     namespace weos
     {
         void assert_failed(const char* condition, const char* function,
                            const char* file, int line);
     } // namespace weos
#    define WEOS_ASSERT(cond)                                                  \
         do { if (!(cond)) ::weos::assert_failed(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__) } while(0)
#  else
#    include <cassert>
#    define WEOS_ASSERT(cond)   assert(cond)
#  endif // WEOS_CUSTOM_ASSERT_HANDLER
#else
#  define WEOS_ASSERT(cond)   ((void)0)
#endif // WEOS_ENABLE_ASSERT

#endif // WEOS_CONFIG_HPP
