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

#include <weos/type_traits.hpp>

using namespace weos;

struct A { A() noexcept; };
struct B { B(); };
struct C { virtual void f() = 0; };

int main()
{
    static_assert( is_nothrow_constructible<int>::value, "");
    static_assert( is_nothrow_constructible<A>::value, "");
    static_assert(!is_nothrow_constructible<B>::value, "");

    // Void and abstract types are not nothrow constructible.
    static_assert(!is_nothrow_constructible<void>::value, "");
    static_assert(!is_nothrow_constructible<C>::value, "");

    // Arrays with known bounds are nothrow constructible if the element
    // type is so.
    static_assert( is_nothrow_constructible<int[4]>::value, "");
    static_assert( is_nothrow_constructible<A[4]>::value, "");
    static_assert(!is_nothrow_constructible<B[4]>::value, "");
    static_assert(!is_nothrow_constructible<int[]>::value, "");
    static_assert(!is_nothrow_constructible<A[]>::value, "");
    static_assert(!is_nothrow_constructible<B[]>::value, "");

    // References are not nothrow default-constructible.
    static_assert(!is_nothrow_constructible<int&>::value, "");
    static_assert(!is_nothrow_constructible<int&&>::value, "");
    static_assert( is_nothrow_constructible<int&, int&>::value, "");
    static_assert( is_nothrow_constructible<int&&, int&&>::value, "");
}
