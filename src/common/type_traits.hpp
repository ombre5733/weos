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

#ifndef WEOS_COMMON_TYPETRAITS_HPP
#define WEOS_COMMON_TYPETRAITS_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include <boost/type_traits.hpp>
#define WEOS_IMPL_NAMESPACE   boost


WEOS_BEGIN_NAMESPACE

template <bool TCondition, typename TType = void>
struct enable_if
{
};

template <typename TType>
struct enable_if<true, TType>
{
    typedef TType type;
};

WEOS_END_NAMESPACE

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <type_traits>
#define WEOS_IMPL_NAMESPACE   std


WEOS_BEGIN_NAMESPACE

using std::enable_if;

WEOS_END_NAMESPACE

#endif // __CC_ARM


WEOS_BEGIN_NAMESPACE

using WEOS_IMPL_NAMESPACE::add_const;
using WEOS_IMPL_NAMESPACE::add_lvalue_reference;
using WEOS_IMPL_NAMESPACE::add_rvalue_reference;
using WEOS_IMPL_NAMESPACE::aligned_storage;
using WEOS_IMPL_NAMESPACE::alignment_of;
using WEOS_IMPL_NAMESPACE::common_type;
using WEOS_IMPL_NAMESPACE::conditional;
using WEOS_IMPL_NAMESPACE::decay;
using WEOS_IMPL_NAMESPACE::false_type;
using WEOS_IMPL_NAMESPACE::integral_constant;
using WEOS_IMPL_NAMESPACE::is_arithmetic;
using WEOS_IMPL_NAMESPACE::is_convertible;
using WEOS_IMPL_NAMESPACE::is_member_function_pointer;
using WEOS_IMPL_NAMESPACE::is_pointer;
using WEOS_IMPL_NAMESPACE::is_same;
using WEOS_IMPL_NAMESPACE::remove_reference;
using WEOS_IMPL_NAMESPACE::true_type;

WEOS_END_NAMESPACE

#undef WEOS_IMPL_NAMESPACE

#endif // WEOS_COMMON_TYPETRAITS_HPP
