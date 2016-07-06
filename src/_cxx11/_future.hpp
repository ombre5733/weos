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

#ifndef WEOS_CXX11_FUTURE_HPP
#define WEOS_CXX11_FUTURE_HPP

#include "_thread_detail.hpp"

#include <exception>
#include <future>


WEOS_BEGIN_NAMESPACE

// ----=====================================================================----
//     future utilities
// ----=====================================================================----

template <typename T>
std::future<T> make_exceptional_future(std::exception_ptr exc)
{
    std::promise<T> promise;
    promise.set_exception(exc);
    return promise.get_future();
}

template <typename T, typename TException>
std::future<T> make_exceptional_future(TException&& exc)
{
    std::promise<T> promise;
    promise.set_exception(std::make_exception_ptr(std::forward<TException>(exc)));
    return promise.get_future();
}

// ----=====================================================================----
//     async()
// ----=====================================================================----

template <typename TFunction, typename... TArgs>
inline
auto async(std::launch launchPolicy, const thread_attributes& attrs,
           TFunction&& f, TArgs&&... args)
    -> decltype(std::async(std::declval<TFunction>(), std::declval<TArgs>()...))
{
    return std::async(launchPolicy, std::forward<TFunction>(f),
                      std::forward<TArgs>(args)...);
}

template <typename TFunction, typename... TArgs>
auto async(const thread_attributes& attrs, TFunction&& f, TArgs&&... args)
    -> decltype(std::async(std::declval<TFunction>(), std::declval<TArgs>()...))
{
    return std::async(std::forward<TFunction>(f),
                      std::forward<TArgs>(args)...);
}

WEOS_END_NAMESPACE

#endif // WEOS_CXX11_FUTURE_HPP
