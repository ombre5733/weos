/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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

#if __ARMCC_VERSION < 5050000 // Format is Mmmbbbb
    #error "Only armcc 5.05 and greater is supported."
#endif

#if __cplusplus < 201103L
    #error "Must be compiled in C++11 mode. Use the '--cpp11' command line argument. Remove the '--gnu' flag."
#endif

#define BOOST_EXCEPTION_DISABLE

#include <boost/config.hpp>

#if __ARMCC_VERSION / 10000 == 505

WEOS_BEGIN_NAMESPACE

struct nullptr_t
{
    bool operator==(nullptr_t) const noexcept
    {
        return true;
    }

    bool operator!=(nullptr_t) const noexcept
    {
        return false;
    }

    template <typename T>
    bool operator==(T* t) const noexcept
    {
        return !t;
    }

    template <typename T>
    bool operator!=(T* t) const noexcept
    {
        return t;
    }

    template <typename T>
    operator T*() const noexcept
    {
        return 0;
    }

    explicit operator bool() const noexcept
    {
        return false;
    }
};

WEOS_END_NAMESPACE

#define nullptr WEOS_NAMESPACE::nullptr_t()

#else

WEOS_BEGIN_NAMESPACE

typedef decltype(nullptr) nullptr_t;

WEOS_END_NAMESPACE

#endif // ARMCC 5.05

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <cstddef>


WEOS_BEGIN_NAMESPACE

using std::nullptr_t;

WEOS_END_NAMESPACE

#endif // __CC_ARM


#if defined(WEOS_WRAP_CXX11)
    #include "cxx11/core.hpp"
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
    #include "keil_cmsis_rtos/core.hpp"
#elif defined(WEOS_WRAP_KEIL_RL_RTX)
    #include "keil_rl_rtx/core.hpp"
#elif defined(WEOS_WRAP_OSAL)
    #include "osal/core.hpp"
#else
    #error "Invalid native OS."
#endif

#endif // WEOS_CORE_HPP
