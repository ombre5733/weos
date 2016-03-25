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

#ifndef WEOS_ARMCC_RATIO_HPP
#define WEOS_ARMCC_RATIO_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include <boost/ratio.hpp>

namespace std
{

using boost::ratio;
using boost::ratio_add;
using boost::ratio_subtract;
using boost::ratio_multiply;
using boost::ratio_divide;

using boost::ratio_equal;
using boost::ratio_not_equal;
using boost::ratio_less;
using boost::ratio_less_equal;
using boost::ratio_greater;
using boost::ratio_greater_equal;

using boost::atto;
using boost::femto;
using boost::pico;
using boost::nano;
using boost::micro;
using boost::milli;
using boost::centi;
using boost::deci;
using boost::deca;
using boost::hecto;
using boost::kilo;
using boost::mega;
using boost::giga;
using boost::tera;
using boost::peta;
using boost::exa;

} // namespace std

#endif // WEOS_ARMCC_RATIO_HPP
