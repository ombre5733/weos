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

#ifndef WEOS_CORE_HPP
#define WEOS_CORE_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#if __ARMCC_VERSION < 5050169 // Format is Mmmbbbb
    #error "Only armcc 5.05u2 and greater is supported."
#endif

#if __cplusplus < 201103L
    #error "Must be compiled in C++11 mode. Use the '--cpp11' flag. Remove the '--gnu' flag."
#endif

#if defined(__CC_ARM) && defined(__GNUC__)
    #error "Must not be compiled in GNU compatibility mode. Remove the '--gnu' flag."
#endif


#define BOOST_EXCEPTION_DISABLE
#define BOOST_COMPILER_CONFIG <weos/common/boost_armcc_5050169_compiler_config.hpp>
#include <boost/config.hpp>


namespace std
{
    typedef decltype(nullptr) nullptr_t;
} // namespace std

WEOS_BEGIN_NAMESPACE

using std::nullptr_t;

WEOS_END_NAMESPACE


#define WEOS_CONSTEXPR_FROM_CXX14
#define WEOS_FORCE_INLINE   __attribute__((always_inline))

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#ifdef __GNUC__

#define WEOS_CONSTEXPR_FROM_CXX14
#define WEOS_FORCE_INLINE   __attribute__((always_inline))

#else

#define WEOS_CONSTEXPR_FROM_CXX14
#define WEOS_FORCE_INLINE   inline

#endif // __GNUC__

#include <cstddef>


WEOS_BEGIN_NAMESPACE

using std::nullptr_t;

WEOS_END_NAMESPACE

#endif // __CC_ARM


// ----=====================================================================----
//     Scoped enums
// ----=====================================================================----

#define WEOS_SCOPED_ENUM_BEGIN(x) enum class x
#define WEOS_SCOPED_ENUM_END(x)


// ----=====================================================================----
// ----=====================================================================----

#if defined(WEOS_WRAP_CXX11)
    #include "cxx11/core.hpp"
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
    #include "keil_cmsis_rtos/core.hpp"
#else
    #error "Invalid native OS."
#endif

#endif // WEOS_CORE_HPP
