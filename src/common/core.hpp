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

#ifndef WEOS_COMMON_CORE_HPP
#define WEOS_COMMON_CORE_HPP

// -----------------------------------------------------------------------------
// C++11
// -----------------------------------------------------------------------------
#if defined(WEOS_USE_CXX11)

#include <cstddef>

#define WEOS_CONSTEXPR_OR_CONST   constexpr
#define WEOS_CONSTEXPR            constexpr
#define WEOS_NOEXCEPT             noexcept


WEOS_BEGIN_NAMESPACE

using std::nullptr_t;

WEOS_END_NAMESPACE

// -----------------------------------------------------------------------------
// Boost
// -----------------------------------------------------------------------------
#elif defined(WEOS_USE_BOOST)

// For armcc, we require that it is compiled with GNU compatibility. The other
// option would be to tell Boost, that armcc is an EDG frontend.
#ifdef __CC_ARM
    #ifndef __GNUC__
        #error "Must be compiled in GNU mode. Use the '--gnu' command line argument."
    #endif // __GNU__
#endif // __CC_ARM

#include <boost/config.hpp>
#include <boost/static_assert.hpp>

#define WEOS_CONSTEXPR_OR_CONST   BOOST_CONSTEXPR_OR_CONST
#define WEOS_CONSTEXPR            BOOST_CONSTEXPR
#define WEOS_NOEXCEPT             BOOST_NOEXCEPT

#define static_assert(cond, msg)   BOOST_STATIC_ASSERT_MSG(cond, msg)

WEOS_BEGIN_NAMESPACE

struct nullptr_t {};

WEOS_END_NAMESPACE

WEOS_CONSTEXPR_OR_CONST weos::nullptr_t nullptr = weos::nullptr_t();

// -----------------------------------------------------------------------------
// Unknown
// -----------------------------------------------------------------------------
#else
    #error "No core.hpp available."
#endif

#endif // WEOS_COMMON_CORE_HPP
