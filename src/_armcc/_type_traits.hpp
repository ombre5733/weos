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

#ifndef WEOS_ARMCC_TYPE_TRAITS_HPP
#define WEOS_ARMCC_TYPE_TRAITS_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include <boost/type_traits.hpp>
#include <boost/utility/result_of.hpp>


namespace std
{

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
template <typename... T>
struct common_type
{
    typedef typename boost::common_type<T...>::type type;
};
#else
template <typename T1, typename T2 = void, typename T3 = void>
struct common_type
{
    typedef typename boost::common_type<T1, T2, T3>::type type;
};
#endif

using boost::add_const;
using boost::add_cv;
using boost::add_lvalue_reference;
using boost::add_rvalue_reference;
using boost::add_pointer;
using boost::add_volatile;
using boost::aligned_storage;
using boost::alignment_of;
using boost::conditional;
using boost::decay;
using boost::extent;
using boost::is_arithmetic;
using boost::is_array;
using boost::is_compound;
using boost::is_const;
using boost::is_floating_point;
using boost::is_function;
using boost::is_fundamental;
using boost::is_integral;
using boost::is_lvalue_reference;
using boost::is_member_pointer;
using boost::is_member_function_pointer;
using boost::is_member_object_pointer;
using boost::is_object;
using boost::is_pointer;
using boost::is_reference;
using boost::is_rvalue_reference;
using boost::is_same;
using boost::is_scalar;
using boost::is_void;
using boost::is_volatile;
using boost::remove_all_extents;
using boost::remove_const;
using boost::remove_cv;
using boost::remove_extent;
using boost::remove_reference;
using boost::remove_volatile;
using boost::result_of;

// ---- enable_if --------------------------------------------------------------

template <bool TCondition, typename TType = void>
struct enable_if
{
};

template <typename TType>
struct enable_if<true, TType>
{
    typedef TType type;
};

// ---- integral_constant ------------------------------------------------------

using boost::integral_constant;
using boost::false_type;
using boost::true_type;

// ---- is_abstract ------------------------------------------------------------

template <typename TType>
struct is_abstract : public integral_constant<bool, __is_abstract(TType)> {};

// ---- is_base_of -------------------------------------------------------------

template <typename TBase, typename TDerived>
struct is_base_of : public integral_constant<bool, __is_base_of(TBase, TDerived)> {};

// ---- is_class ---------------------------------------------------------------

template <typename TType>
struct is_class : public integral_constant<bool, __is_class(TType)> {};

// ---- is_constructible -------------------------------------------------------

template <typename TType, typename... TArgs>
struct is_constructible : public integral_constant<bool, __is_constructible(TType, TArgs...)> {};

// ---- is_convertible ---------------------------------------------------------

template <typename TFrom, typename TTo>
struct is_convertible : public integral_constant<bool, __is_convertible_to(TFrom, TTo) &&
                                                       !is_abstract<TTo>::value> {};

// ---- is_default_constructible -----------------------------------------------

template <typename TType>
struct is_default_constructible : public is_constructible<TType> {};

// ---- is_empty ---------------------------------------------------------------

template <typename TType>
struct is_empty : public integral_constant<bool, __is_empty(TType)> {};

// ---- is_enum ----------------------------------------------------------------

template <typename TType>
struct is_enum : public integral_constant<bool, __is_enum(TType)> {};

// ---- is_final ---------------------------------------------------------------

template <typename TType>
struct is_final : public integral_constant<bool, __is_final(TType)> {};

// ---- is_literal_type --------------------------------------------------------

template <typename TType>
struct is_literal_type
        : public integral_constant<bool, __is_literal_type(TType)> {};

// ---- is_nothrow_constructible -----------------------------------------------

template <typename TType, typename... TArgs>
struct is_nothrow_constructible
        : public integral_constant<bool, __is_nothrow_constructible(TType, TArgs...)> {};

// ---- is_nothrow_copy_constructible ------------------------------------------

template <typename TType>
struct is_nothrow_copy_constructible
    : public is_nothrow_constructible<
                 TType,
                 typename add_lvalue_reference<typename add_const<TType>::type>::type> {};

// ---- is_nothrow_default_constructible ---------------------------------------

template <typename TType>
struct is_nothrow_default_constructible : public is_nothrow_constructible<TType> {};

// ---- is_union ---------------------------------------------------------------

template <typename TType>
struct is_union : public integral_constant<bool, __is_union(TType)> {};

// ---- is_trivially_copyable --------------------------------------------------

template <typename TType>
struct is_trivially_copyable : public integral_constant<bool, __is_trivially_copyable(TType)> {};

} // namespace std

#endif // WEOS_ARMCC_TYPE_TRAITS_HPP
