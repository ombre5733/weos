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

#ifndef WEOS_COMMON_THREAD_DETAIL_HPP
#define WEOS_COMMON_THREAD_DETAIL_HPP

#include "../config.hpp"

#include "../functional.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TFunction, typename... TArgs>
class DecayedFunction
{
public:
    typedef typename ::WEOS_NAMESPACE::weos_detail::invoke_result_type<TFunction, TArgs...>::type result_type;

    explicit DecayedFunction(TFunction&& f, TArgs&&... args)
        : m_boundFunction(WEOS_NAMESPACE::move(f),
                          WEOS_NAMESPACE::move(args)...)
    {
    }

    DecayedFunction(DecayedFunction&& other)
        : m_boundFunction(WEOS_NAMESPACE::move(other.m_boundFunction))
    {
    }

    result_type operator()()
    {
        typedef typename weos_detail::make_tuple_indices<
                1 + sizeof...(TArgs), 1>::type indices_type;
        return invoke(indices_type());
    }

private:
    template <std::size_t... TIndices>
    result_type invoke(weos_detail::TupleIndices<TIndices...>)
    {
        return WEOS_NAMESPACE::invoke(
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<0>(m_boundFunction)),
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<TIndices>(m_boundFunction))...);
    }

    tuple<TFunction, TArgs...> m_boundFunction;
};

// 30.2.6
template <typename T>
typename decay<T>::type decay_copy(T&& v)
{
    return WEOS_NAMESPACE::forward<T>(v);
}

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_THREAD_DETAIL_HPP
