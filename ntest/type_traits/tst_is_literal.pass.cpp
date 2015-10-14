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

struct Literal
{
    int elements[2];
};

struct NonLiteral
{
    virtual ~NonLiteral();
};

int main()
{
   // Scalar types
   static_assert( is_literal_type<char>::value, "");
   static_assert( is_literal_type<int>::value, "");
   static_assert( is_literal_type<long>::value, "");
   static_assert( is_literal_type<float>::value, "");
   static_assert( is_literal_type<double>::value, "");
   static_assert( is_literal_type<const int>::value, "");
   static_assert( is_literal_type<volatile int>::value, "");
   static_assert( is_literal_type<const volatile int>::value, "");

   // References
   static_assert( is_literal_type<int&>::value, "");
   static_assert( is_literal_type<const int&>::value, "");
   static_assert( is_literal_type<volatile int&>::value, "");
   static_assert( is_literal_type<const volatile int&>::value, "");
   static_assert( is_literal_type<int&&>::value, "");
   static_assert( is_literal_type<const int&&>::value, "");
   static_assert( is_literal_type<volatile int&&>::value, "");
   static_assert( is_literal_type<const volatile int&&>::value, "");
   static_assert( is_literal_type<NonLiteral&>::value, "");
   static_assert( is_literal_type<const NonLiteral&>::value, "");
   static_assert( is_literal_type<const volatile NonLiteral&>::value, "");
   static_assert( is_literal_type<NonLiteral&&>::value, "");

   // Arrays
   static_assert( is_literal_type<int[4]>::value, "");
   static_assert( is_literal_type<double[4]>::value, "");
   static_assert( is_literal_type<Literal[4]>::value, "");
   static_assert(!is_literal_type<NonLiteral[4]>::value, "");

   // Classes
   static_assert( is_literal_type<Literal>::value, "");
   static_assert(!is_literal_type<NonLiteral>::value, "");
}
