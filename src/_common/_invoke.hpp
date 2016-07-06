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

#ifndef WEOS_COMMON_INVOKE_HPP
#define WEOS_COMMON_INVOKE_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

// Invokes a Callable. See: http://en.cppreference.com/w/cpp/concept/Callable

// Pointer to member function + object of/reference to matching class.
template <typename F, typename A0, typename... An>
inline
auto invoke(F&& f, A0&& a0, An&&... an)
    -> decltype((std::forward<A0>(a0).*f)(std::forward<An>(an)...))
{
    return (std::forward<A0>(a0).*f)(std::forward<An>(an)...);
}

// Pointer to member function + pointer to matching class.
template <typename F, typename A0, typename... An>
inline
auto invoke(F&& f, A0&& a0, An&&... an)
    -> decltype(((*std::forward<A0>(a0)).*f)(std::forward<An>(an)...))
{
    return ((*std::forward<A0>(a0)).*f)(std::forward<An>(an)...);
}

// Pointer to data member + object of/reference to matching class.
template <typename F, typename A0>
inline
auto invoke(F&& f, A0&& a0)
    -> decltype(std::forward<A0>(a0).*f)
{
    return std::forward<A0>(a0).*f;
}

// Pointer to data member + pointer to matching class.
template <typename F, typename A0>
inline
auto invoke(F&& f, A0&& a0)
    -> decltype((*std::forward<A0>(a0)).*f)
{
    return (*std::forward<A0>(a0)).*f;
}

// Function object
template <typename F, typename... An>
inline
auto invoke(F&& f, An&&... an)
    -> decltype(std::forward<F>(f)(std::forward<An>(an)...))
{
    return std::forward<F>(f)(std::forward<An>(an)...);
}

template <typename F, typename... An>
struct invoke_result_type
{
    using type = decltype(invoke(std::declval<F>(), std::declval<An>()...));
};

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_INVOKE_HPP
