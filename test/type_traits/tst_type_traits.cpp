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

#include <type_traits.hpp>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"

using namespace weos;

TEST(type_traits, enable_if)
{
    static_assert(is_same<enable_if<true>::type, void>::value, "");
    static_assert(is_same<enable_if<true, int>::type, int>::value, "");
    // TODO: make sure that enable_if<false> has no type member
}

namespace is_abstract_
{
struct A {};
struct B { virtual void operator()() = 0; };
struct C : B {};
struct D : B { virtual void operator()() {} };
}

TEST(type_traits, is_abstract)
{
    using namespace is_abstract_;
    static_assert(!is_abstract<A>::value, "");
    static_assert( is_abstract<B>::value, "");
    static_assert( is_abstract<C>::value, "");
    static_assert(!is_abstract<D>::value, "");
}

namespace is_base_of_
{
struct A {};
struct B : A {};
struct C : private A {};
}

TEST(type_traits, is_base_of)
{
    using namespace is_base_of_;
    static_assert( is_base_of<A, B>::value, "");
    static_assert( is_base_of<A, C>::value, "");
    static_assert( is_base_of<A, A>::value, "");
    static_assert(!is_base_of<B, A>::value, "");
    static_assert(!is_base_of<C, A>::value, "");
}

namespace is_class_
{
struct A {};
struct B;
}

TEST(type_traits, is_class)
{
    using namespace is_class_;
    static_assert(!is_class<int>::value, "");
    static_assert( is_class<A>::value, "");
    static_assert( is_class<B>::value, "");
}

namespace is_constructible_
{
    struct A {};
    struct B { B(int); };
    struct C { virtual void f() = 0; };
    struct D { D(B); };
    struct E { explicit E(int); };
    struct F { F(E); };
}

TEST(type_traits, is_constructible)
{
    using namespace is_constructible_;
    static_assert( is_constructible<int>::value, "");
    static_assert( is_constructible<A>::value, "");
    static_assert(!is_constructible<A, int>::value, "");
    static_assert(!is_constructible<B>::value, "");
    static_assert( is_constructible<B, int>::value, "");

    // Void and abstract types are not constructible.
    static_assert(!is_constructible<void>::value, "");
    static_assert(!is_constructible<C>::value, "");

    // Arrays with known bounds are constructible if the element type is so.
    static_assert( is_constructible<int[4]>::value, "");
    static_assert( is_constructible<A[4]>::value, "");
    static_assert(!is_constructible<B[4]>::value, "");
    static_assert(!is_constructible<int[]>::value, "");
    static_assert(!is_constructible<A[]>::value, "");
    static_assert(!is_constructible<B[]>::value, "");

    // References are not default-constructible.
    static_assert(!is_constructible<int&>::value, "");
    static_assert(!is_constructible<int&&>::value, "");

    // References can be created from an implicitly convertible type.
    static_assert( is_constructible<int&, int&>::value, "");
    static_assert( is_constructible<int&&, int&&>::value, "");
    static_assert(!is_constructible<int&, int>::value, "");
    static_assert( is_constructible<int&&, int>::value, "");
    static_assert( is_constructible<const int&, int>::value, "");
    static_assert(!is_constructible<B&, int>::value, "");
    static_assert( is_constructible<B&&, int>::value, "");
    static_assert( is_constructible<const B&, int>::value, "");

    // Function types are not constructible.
    static_assert(!is_constructible<void()>::value, "");
    static_assert(!is_constructible<int()>::value, "");
    static_assert(!is_constructible<int(int)>::value, "");

    // Implicit and explicit conversion.
    static_assert( is_constructible<D, int>::value, "");
    static_assert(!is_constructible<F, int>::value, "");
    static_assert( is_constructible<F, E>::value, "");
}

TEST(type_traits, is_default_constructible)
{
    using namespace is_constructible_;
    static_assert( is_default_constructible<int>::value, "");
    static_assert( is_default_constructible<A>::value, "");
    static_assert(!is_default_constructible<B>::value, "");

    // Void and abstract types are not constructible.
    static_assert(!is_default_constructible<void>::value, "");
    static_assert(!is_default_constructible<C>::value, "");

    // Arrays with known bounds are constructible if the element type is so.
    static_assert( is_default_constructible<int[4]>::value, "");
    static_assert( is_default_constructible<A[4]>::value, "");
    static_assert(!is_default_constructible<B[4]>::value, "");
    static_assert(!is_default_constructible<int[]>::value, "");
    static_assert(!is_default_constructible<A[]>::value, "");
    static_assert(!is_default_constructible<B[]>::value, "");

    // References are not default-constructible.
    static_assert(!is_default_constructible<int&>::value, "");
    static_assert(!is_default_constructible<int&&>::value, "");

    // Function types are not constructible.
    static_assert(!is_default_constructible<void()>::value, "");
    static_assert(!is_default_constructible<int()>::value, "");
    static_assert(!is_default_constructible<int(int)>::value, "");

    static_assert(!is_default_constructible<void(&)()>::value, "");
    static_assert(!is_default_constructible<int(&)()>::value, "");
    static_assert(!is_default_constructible<int(&)(int)>::value, "");

    static_assert( is_default_constructible<void(*)()>::value, "");
    static_assert( is_default_constructible<int(*)()>::value, "");
    static_assert( is_default_constructible<int(*)(int)>::value, "");
}

namespace is_empty_
{
struct A {};
struct B : A {};
struct C { char c; };
}

TEST(type_traits, is_empty)
{
    using namespace is_empty_;
    static_assert( is_empty<A>::value, "");
    static_assert( is_empty<B>::value, "");
    static_assert(!is_empty<C>::value, "");
}

namespace is_enum_
{
struct A {};
enum B {};
enum class C {};
enum class D : char {};
}

TEST(type_traits, is_enum)
{
    using namespace is_enum_;
    static_assert(!is_enum<A>::value, "");
    static_assert( is_enum<B>::value, "");
    static_assert( is_enum<C>::value, "");
    static_assert( is_enum<D>::value, "");
}

//namespace is_final_
//{
//struct A {};
//struct B final {};
//}
//
//TEST(type_traits, is_final)
//{
//    using namespace is_final_;
//    static_assert(!is_final<A>::value, "");
//    static_assert( is_final<B>::value, "");
//}

namespace is_nothrow_constructible_
{
struct A { A() noexcept; };
struct B { B(); };
struct C { virtual void f() = 0; };
}

TEST(type_traits, is_nothrow_constructible)
{
    using namespace is_nothrow_constructible_;
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

// is_nothrow_copy_constructible

TEST(type_traits, is_same)
{
    static_assert( is_same<int, int>::value, "");
    static_assert(!is_same<int&, int>::value, "");
}

namespace is_union_
{
struct S {};
union U {};
}

TEST(type_traits, is_union)
{
    using namespace is_union_;
    static_assert(!is_union<S>::value, "");
    static_assert( is_union<U>::value, "");
}

// is_trivially_copyable
