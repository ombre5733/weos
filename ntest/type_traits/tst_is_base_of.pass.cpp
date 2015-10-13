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

#include <weos/type_traits.hpp>

using namespace weos;

struct A {};
struct B {};
struct C : A {};
struct D : private A {};
struct E : A, B {};
struct F : virtual C, virtual D {};

int main()
{
    static_assert( is_base_of<A, A>::value, "");
    static_assert( is_base_of<A, C>::value, "");
    static_assert( is_base_of<A, D>::value, "");
    static_assert( is_base_of<A, E>::value, "");
    static_assert( is_base_of<B, E>::value, "");
    static_assert( is_base_of<A, F>::value, "");
    static_assert( is_base_of<C, F>::value, "");
    static_assert( is_base_of<D, F>::value, "");

    static_assert(!is_base_of<C, A>::value, "");
    static_assert(!is_base_of<D, A>::value, "");
    static_assert(!is_base_of<E, A>::value, "");
    static_assert(!is_base_of<E, B>::value, "");
    static_assert(!is_base_of<F, A>::value, "");
    static_assert(!is_base_of<F, C>::value, "");
    static_assert(!is_base_of<F, D>::value, "");
}
