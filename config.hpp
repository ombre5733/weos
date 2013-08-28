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


#if defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#  include "3rdparty/keil_cmsis_rtos/INC/cmsis_os.h"
#  if osCMSIS_RTX != ((4<<16) | 70)
#    error "The Keil CMSIS RTOS version must be 4.70."
#  endif
#endif

namespace weos
{

#if !defined(WEOS_CUSTOM_THROW_EXCEPTION)
  template <typename ExceptionT>
  /*BOOST_ATTRIBUTE_NORETURN*/ inline void throw_exception(const ExceptionT& e)
  {
      while(1); //! \todo throw here
  }
#else
  // This is only a declaration - the definition has to be provided by the user.
  void throw_exception(const std::exception& e);
#endif

} // namespace weos

#endif // WEOS_CONFIG_HPP
