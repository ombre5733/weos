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

#ifndef WEOS_TYPE_TRAITS_HPP
#define WEOS_TYPE_TRAITS_HPP

#include "_config.hpp"

#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include "_armcc/_type_traits.hpp"

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <type_traits>


WEOS_BEGIN_NAMESPACE

template <typename... T>
struct common_type
{
    typedef typename std::common_type<T...>::type type;
};

using std::add_const;
using std::add_cv;
using std::add_lvalue_reference;
using std::add_rvalue_reference;
using std::add_pointer;
using std::add_volatile;
using std::aligned_storage;
using std::alignment_of;
using std::conditional;
using std::extent;
using std::is_arithmetic;
using std::is_array;
using std::is_compound;
using std::is_const;
using std::is_floating_point;
using std::is_function;
using std::is_fundamental;
using std::is_integral;
using std::is_lvalue_reference;
using std::is_member_pointer;
using std::is_member_function_pointer;
using std::is_member_object_pointer;
using std::is_object;
using std::is_pointer;
using std::is_reference;
using std::is_rvalue_reference;
using std::is_same;
using std::is_scalar;
using std::is_void;
using std::is_volatile;
using std::remove_all_extents;
using std::remove_const;
using std::remove_cv;
using std::remove_extent;
using std::remove_reference;
using std::remove_volatile;
using std::result_of;

using std::decay;
using std::enable_if;

using std::integral_constant;
using std::false_type;
using std::true_type;

using std::is_abstract;
using std::is_base_of;
using std::is_class;
using std::is_constructible;
using std::is_convertible;
using std::is_default_constructible;
using std::is_empty;
using std::is_enum;
//using std::is_final;
using std::is_literal_type;
using std::is_nothrow_constructible;
using std::is_nothrow_copy_constructible;
using std::is_nothrow_default_constructible;
using std::is_union;
// TODO: using std::is_trivially_copyable;
template <typename T>
struct is_trivially_copyable : public std::is_pod<T> {};

WEOS_END_NAMESPACE

#endif // __CC_ARM

#endif // WEOS_TYPE_TRAITS_HPP
