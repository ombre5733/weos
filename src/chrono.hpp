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

#ifndef WEOS_CHRONO_HPP
#define WEOS_CHRONO_HPP

#include "_config.hpp"

#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include "_common/_chrono.hpp"

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <chrono>

WEOS_BEGIN_NAMESPACE

namespace chrono
{

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::duration_values;
using std::chrono::time_point;
using std::chrono::time_point_cast;
using std::chrono::treat_as_floating_point;

using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;

} // namespace chrono

WEOS_END_NAMESPACE

#endif // __CC_ARM

// ----=====================================================================----
//     System clock
// ----=====================================================================----

#if defined(WEOS_WRAP_CMSIS_RTOS)
    #include "_cmsis_rtos/_chrono_clocks.hpp"
#else
    #error "Invalid native OS."
#endif

#endif // WEOS_CHRONO_HPP
