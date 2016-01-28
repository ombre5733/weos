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

#include <exception.hpp>

#include <stdexcept>

#include "../common/testutils.hpp"
#include "gtest/gtest.h"


TEST(exception_ptr, constructor)
{
    weos::exception_ptr ptr;
    ASSERT_TRUE(ptr == nullptr);
}

template <typename T>
T create(weos::true_type)
{
    return T();
}

template <typename T>
T create(weos::false_type)
{
    return T("");
}

template <typename T>
int throwTest()
{
    bool caught = false;
    try
    {
        throw create<T>(weos::integral_constant<bool, weos::is_default_constructible<T>::value>());
    }
    catch (...)
    {
        weos::exception_ptr ptr = weos::current_exception();
        ASSERT_TRUE(ptr != nullptr);
        try
        {
            weos::rethrow_exception(ptr);
        }
        catch (T&)
        {
            caught = true;
        }
        catch (...)
        {
        }
    }
    ASSERT_TRUE(caught);
    return 0;
}

template <typename... T>
void execute(T&&...) noexcept
{
}

template <class... T>
void throwTests()
{
    execute(throwTest<T>()...);
}

TEST(rethrow_exception, std_exception)
{
    throwTests<std::bad_alloc,
               std::logic_error,
               std::domain_error,
               std::invalid_argument,
               std::length_error,
               std::out_of_range,
               std::runtime_error,
               std::range_error,
               std::overflow_error,
               std::underflow_error>();
}
