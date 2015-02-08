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

#ifndef WEOS_COMMON_RATIO_HPP
#define WEOS_COMMON_RATIO_HPP

#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include <boost/ratio.hpp>
#define WEOS_IMPL_NAMESPACE   boost

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <ratio>
#define WEOS_IMPL_NAMESPACE   std

#endif // __CC_ARM


WEOS_BEGIN_NAMESPACE

using WEOS_IMPL_NAMESPACE::ratio;
using WEOS_IMPL_NAMESPACE::ratio_add;
using WEOS_IMPL_NAMESPACE::ratio_subtract;
using WEOS_IMPL_NAMESPACE::ratio_multiply;
using WEOS_IMPL_NAMESPACE::ratio_divide;

using WEOS_IMPL_NAMESPACE::ratio_equal;
using WEOS_IMPL_NAMESPACE::ratio_not_equal;
using WEOS_IMPL_NAMESPACE::ratio_less;
using WEOS_IMPL_NAMESPACE::ratio_less_equal;
using WEOS_IMPL_NAMESPACE::ratio_greater;
using WEOS_IMPL_NAMESPACE::ratio_greater_equal;

using WEOS_IMPL_NAMESPACE::atto;
using WEOS_IMPL_NAMESPACE::femto;
using WEOS_IMPL_NAMESPACE::pico;
using WEOS_IMPL_NAMESPACE::nano;
using WEOS_IMPL_NAMESPACE::micro;
using WEOS_IMPL_NAMESPACE::milli;
using WEOS_IMPL_NAMESPACE::centi;
using WEOS_IMPL_NAMESPACE::deci;
using WEOS_IMPL_NAMESPACE::deca;
using WEOS_IMPL_NAMESPACE::hecto;
using WEOS_IMPL_NAMESPACE::kilo;
using WEOS_IMPL_NAMESPACE::mega;
using WEOS_IMPL_NAMESPACE::giga;
using WEOS_IMPL_NAMESPACE::tera;
using WEOS_IMPL_NAMESPACE::peta;
using WEOS_IMPL_NAMESPACE::exa;

WEOS_END_NAMESPACE

#undef WEOS_IMPL_NAMESPACE

#endif // WEOS_COMMON_RATIO_HPP
