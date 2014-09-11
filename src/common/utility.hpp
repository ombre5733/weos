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

#ifndef WEOS_COMMON_UTILITY_HPP
#define WEOS_COMMON_UTILITY_HPP

// -----------------------------------------------------------------------------
// C++11
// -----------------------------------------------------------------------------
#if defined(WEOS_USE_CXX11)

#include <utility>

#define WEOS_COPYABLE_AND_MOVABLE(type)
#define WEOS_MOVABLE_BUT_NOT_COPYABLE(type)                                    \
    type(const type&) = delete;                                                \
    type& operator= (const type&) = delete;

#define WEOS_FWD_REF(type)   type&&
#define WEOS_RV_REF(type)    type&&


WEOS_BEGIN_NAMESPACE

using std::forward;
using std::move;

WEOS_END_NAMESPACE

// -----------------------------------------------------------------------------
// Boost
// -----------------------------------------------------------------------------
#elif defined(WEOS_USE_BOOST)

#include <boost/move/move.hpp>

#define WEOS_COPYABLE_AND_MOVABLE(type)                                        \
    BOOST_COPYABLE_AND_MOVABLE(type)
#define WEOS_MOVABLE_BUT_NOT_COPYABLE(type)                                    \
    BOOST_MOVABLE_BUT_NOT_COPYABLE(type)

#define WEOS_FWD_REF(type)   BOOST_FWD_REF(type)
#define WEOS_RV_REF(type)    BOOST_RV_REF(type)


WEOS_BEGIN_NAMESPACE

using boost::forward;
using boost::move;

WEOS_END_NAMESPACE

// -----------------------------------------------------------------------------
// Unknown
// -----------------------------------------------------------------------------
#else
    #error "No utility.hpp available."
#endif

#endif // WEOS_COMMON_UTILITY_HPP
