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

#ifndef WEOS_COMMON_INDEX_SEQUENCE_HPP
#define WEOS_COMMON_INDEX_SEQUENCE_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../type_traits.hpp"
#include <cstddef>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <std::size_t... TIndices>
struct IndexSequence
{
};

template <std::size_t TStart, std::size_t TEnd, typename TTemporary>
struct makeIndexSequenceImpl;

template <std::size_t TStart, std::size_t TEnd, std::size_t... TIndices>
struct makeIndexSequenceImpl<TStart, TEnd, IndexSequence<TIndices...>>
{
    using type = typename makeIndexSequenceImpl<
                     TStart + 1,
                     TEnd,
                     IndexSequence<TIndices..., TStart>>::type;
};

// Specialization for TStart == TEnd.
template <std::size_t TStart, std::size_t... TIndices>
struct makeIndexSequenceImpl<TStart, TStart, IndexSequence<TIndices...>>
{
    using type = IndexSequence<TIndices...>;
};

template <std::size_t TEnd, std::size_t TStart = 0>
struct makeIndexSequence
{
    static_assert(TStart <= TEnd, "makeIndexSequence: invalid range");
    using type = typename makeIndexSequenceImpl<TStart, TEnd, IndexSequence<>>::type;
};

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_INDEX_SEQUENCE_HPP
