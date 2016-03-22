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

#ifndef WEOS_COMMON_UTILITY_HPP
#define WEOS_COMMON_UTILITY_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include "../type_traits.hpp"
#include <cstddef>


WEOS_BEGIN_NAMESPACE

// "Creates" a reference type from T. Note that there is no definition for
// declval, it may only be used in an unevaluated context, e.g. a decltype().
template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept;

template <typename T>
inline
constexpr typename remove_reference<T>::type&& move(T&& t) noexcept
{
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template <typename T>
inline
constexpr T&& forward(typename remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);
}

template <typename T>
inline
constexpr T&& forward(typename remove_reference<T>::type&& t) noexcept
{
    return static_cast<T&&>(t);
}

// ----=====================================================================----
//     integer_sequence
// ----=====================================================================----

template <typename T, T... TIndices>
struct integer_sequence
{
    typedef T value_type;

    static constexpr std::size_t size() noexcept
    {
        return sizeof...(TIndices);
    }
};

namespace weos_detail
{

template <typename T, typename TSequence1, typename TSequence2>
struct AppendIntegerSequence;

template <typename T, T... TIndices1, T... TIndices2>
struct AppendIntegerSequence<T, integer_sequence<T, TIndices1...>, integer_sequence<T, TIndices2...>>
{
    using type = integer_sequence<T, TIndices1..., (sizeof...(TIndices1) + TIndices2)...>;
};

template <typename T, std::size_t N>
struct MakeIntegerSequence
{
    using type = typename AppendIntegerSequence<T,
                                                typename MakeIntegerSequence<T, N/2>::type,
                                                typename MakeIntegerSequence<T, N - N/2>::type>::type;
};

template <typename T>
struct MakeIntegerSequence<T, 0>
{
    using type = integer_sequence<T>;
};

template <typename T>
struct MakeIntegerSequence<T, 1>
{
    using type = integer_sequence<T, 0>;
};

template <typename T, T N>
struct MakeSafeIntegerSequence
{
    // Break compilation if N is negative. Otherwise, the compiler tries to
    // create lots and lots of template instances.
    static_assert(N >= 0, "The sequence length must be positive.");
    using type = typename MakeIntegerSequence<T, N>::type;
};

} // namespace weos_detail

template <std::size_t... TValues>
using index_sequence = integer_sequence<std::size_t, TValues...>;

//! Creates the integer sequence 0, 1, 2, ... N - 1 of type T.
template <typename T, T N>
using make_integer_sequence = typename weos_detail::MakeSafeIntegerSequence<T, N>::type;

//! Creates the integer sequence 0, 1, 2, ... N - 1 with type std::size_t.
template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <typename... TArgs>
using index_sequence_for = make_index_sequence<sizeof...(TArgs)>;

WEOS_END_NAMESPACE

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <utility>


WEOS_BEGIN_NAMESPACE

using std::declval;
using std::forward;
using std::move;

WEOS_END_NAMESPACE

#endif // __CC_ARM

#endif // WEOS_COMMON_UTILITY_HPP
