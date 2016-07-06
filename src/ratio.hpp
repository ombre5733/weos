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

#ifndef WEOS_RATIO_HPP
#define WEOS_RATIO_HPP

#include "_config.hpp"


#ifdef __CC_ARM
#include "_armcc/_ratio.hpp"
#else
#include <ratio>
#endif // __CC_ARM


// TODO:CLEAN
WEOS_BEGIN_NAMESPACE

using std::ratio;
using std::ratio_add;
using std::ratio_subtract;
using std::ratio_multiply;
using std::ratio_divide;

using std::ratio_equal;
using std::ratio_not_equal;
using std::ratio_less;
using std::ratio_less_equal;
using std::ratio_greater;
using std::ratio_greater_equal;

using std::atto;
using std::femto;
using std::pico;
using std::nano;
using std::micro;
using std::milli;
using std::centi;
using std::deci;
using std::deca;
using std::hecto;
using std::kilo;
using std::mega;
using std::giga;
using std::tera;
using std::peta;
using std::exa;

WEOS_END_NAMESPACE

#endif // WEOS_RATIO_HPP
