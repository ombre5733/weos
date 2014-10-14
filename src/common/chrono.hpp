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

#ifndef WEOS_COMMON_CHRONO_HPP
#define WEOS_COMMON_CHRONO_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


// -----------------------------------------------------------------------------
// C++11
// -----------------------------------------------------------------------------
#if defined(WEOS_USE_CXX11)

#include <chrono>
#define WEOS_IMPL_NAMESPACE   std::chrono
#define WEOS_NEED_IMPORT      1


// -----------------------------------------------------------------------------
// Boost
// -----------------------------------------------------------------------------
#elif defined(WEOS_USE_BOOST)

#ifdef __CC_ARM

    #include "duration.hpp"
    #include "timepoint.hpp"
    #define WEOS_NEED_IMPORT      0

#else

    #include <boost/chrono.hpp>
    #define WEOS_IMPL_NAMESPACE   boost::chrono
    #define WEOS_NEED_IMPORT      1

#endif // __CC_ARM


// -----------------------------------------------------------------------------
// Unknown
// -----------------------------------------------------------------------------
#else
    #error "No chrono.hpp available."
#endif

#if WEOS_NEED_IMPORT

    WEOS_BEGIN_NAMESPACE

    namespace chrono
    {

    using WEOS_IMPL_NAMESPACE::duration;
    using WEOS_IMPL_NAMESPACE::duration_cast;
    using WEOS_IMPL_NAMESPACE::duration_values;
    using WEOS_IMPL_NAMESPACE::time_point;
    using WEOS_IMPL_NAMESPACE::treat_as_floating_point;

    using WEOS_IMPL_NAMESPACE::nanoseconds;
    using WEOS_IMPL_NAMESPACE::microseconds;
    using WEOS_IMPL_NAMESPACE::milliseconds;
    using WEOS_IMPL_NAMESPACE::seconds;
    using WEOS_IMPL_NAMESPACE::minutes;
    using WEOS_IMPL_NAMESPACE::hours;

    } // namespace chrono

WEOS_END_NAMESPACE

#endif // WEOS_NEED_IMPORT


#undef WEOS_IMPL_NAMESPACE
#undef WEOS_NEED_IMPORT


#endif // WEOS_COMMON_CHRONO_HPP
