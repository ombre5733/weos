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

#ifndef WEOS_COMMON_APPLY_HPP
#define WEOS_COMMON_APPLY_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#if __cplusplus <= 201402L

#include "_invoke.hpp"
#include "../utility.hpp"

#include <cstddef>

namespace std
{

namespace weos_detail
{

template <typename TF, typename TArgsTuple, std::size_t... TIndices>
constexpr
auto doApply(TF&& f, TArgsTuple&& args, std::index_sequence<TIndices...>)
    -> decltype(std::invoke(std::declval<TF>(), std::declval<std::tuple_element_t<TIndices, TArgsTuple>>()...))
{
    return std::invoke(std::forward<TF>(f),
                       std::get<TIndices>(std::forward<TArgsTuple>(args))...);
}

} // namespace weos_detail

//! Executes the callable \p f with the arguments \p args given in a tuple.
template <typename TF, typename TArgsTuple>
constexpr
auto apply(TF&& f, TArgsTuple&& args)
    -> decltype(weos_detail::doApply(std::declval<TF>(), std::declval<TArgsTuple>(),
                                     std::make_index_sequence<std::tuple_size<TArgsTuple>::value>()))
{
    return weos_detail::doApply(
                std::forward<TF>(f), std::forward<TArgsTuple>(args),
                std::make_index_sequence<std::tuple_size<TArgsTuple>::value>());
}

} // namespace std

#endif // __cplusplus <= 201402L

#endif // WEOS_COMMON_APPLY_HPP
