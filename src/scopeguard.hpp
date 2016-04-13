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

#ifndef WEOS_SCOPEGUARD_HPP
#define WEOS_SCOPEGUARD_HPP

#include "_config.hpp"

#include "exception.hpp"
#include "type_traits.hpp"
#include "utility.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TCallable>
class ScopeGuard
{
public:
    template <typename T,
              typename _ = typename std::enable_if<!std::is_same<typename std::decay<T>::type,
                                                   ScopeGuard>::value>::type>
    explicit ScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(callable),
          m_dismissed(false)
    {
    }

    ScopeGuard(ScopeGuard&& other) // TODO: noexcept
        : m_callable(std::move(other.m_callable)),
          m_dismissed(other.m_dismissed)
    {
        other.m_dismissed = true;
    }

    ~ScopeGuard()
    {
        if (!m_dismissed)
        {
            m_callable();
        }
    }

    ScopeGuard(const ScopeGuard& other) = delete;
    ScopeGuard& operator=(const ScopeGuard& other) = delete;

    void dismiss() noexcept
    {
        m_dismissed = true;
    }

private:
    TCallable m_callable;
    bool m_dismissed;
};

template <typename TCallable, bool TExecuteOnException>
class ExceptionScopeGuard
{
public:
    template <typename T,
              typename _ = typename std::enable_if<!std::is_same<typename std::decay<T>::type,
                                                   ExceptionScopeGuard>::value>::type>
    explicit ExceptionScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(std::forward<T>(callable)),
          m_numExceptions(std::uncaught_exceptions())
    {
    }

    ExceptionScopeGuard(ExceptionScopeGuard&& other) // TODO: noexcept
        : m_callable(std::move(other.m_callable)),
          m_numExceptions(other.m_numExceptions)
    {
    }

    ~ExceptionScopeGuard() noexcept(TExecuteOnException)
    {
        if (TExecuteOnException
            == (std::uncaught_exceptions() > m_numExceptions))
        {
            m_callable();
        }
    }

    ExceptionScopeGuard(const ExceptionScopeGuard& other) = delete;
    ExceptionScopeGuard& operator=(const ExceptionScopeGuard& other) = delete;

private:
    TCallable m_callable;
    int m_numExceptions;
};

} // namespace weos_detail

struct OnScopeExit {};
struct OnScopeFailure {};
struct OnScopeSuccess {};

template <typename TCallable>
weos_detail::ScopeGuard<typename std::decay<TCallable>::type> operator+(
        OnScopeExit, TCallable&& callable)
{
    return weos_detail::ScopeGuard<typename std::decay<TCallable>::type>(
                std::forward<TCallable>(callable));
}

template <typename TCallable>
weos_detail::ExceptionScopeGuard<typename std::decay<TCallable>::type, true> operator+(
        OnScopeFailure, TCallable&& callable)
{
    return weos_detail::ExceptionScopeGuard<typename std::decay<TCallable>::type, true>(
                std::forward<TCallable>(callable));
}

template <typename TCallable>
weos_detail::ExceptionScopeGuard<typename std::decay<TCallable>::type, false> operator+(
        OnScopeSuccess, TCallable&& callable)
{
    return weos_detail::ExceptionScopeGuard<typename std::decay<TCallable>::type, false>(
                std::forward<TCallable>(callable));
}

// Helper macros for the generation of anonymous variables.
#define WEOS_CONCATENATE_HELPER(a, b)   a ## b
#define WEOS_CONCATENATE(a, b)          WEOS_CONCATENATE_HELPER(a, b)
#define WEOS_ANONYMOUS_VARIABLE(name)   WEOS_CONCATENATE(name, __LINE__)


// Usage:
// WEOS_SCOPE_EXIT {
//     some code here
// };
#define WEOS_SCOPE_EXIT                                                        \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) =                          \
        ::WEOS_NAMESPACE::OnScopeExit() + [&]() noexcept

#define WEOS_SCOPE_FAILURE                                                     \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) =                          \
        ::WEOS_NAMESPACE::OnScopeFailure() + [&]() noexcept

#define WEOS_SCOPE_SUCCESS                                                     \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) =                          \
        ::WEOS_NAMESPACE::OnScopeSuccess() + [&]()

WEOS_END_NAMESPACE


#endif // WEOS_SCOPEGUARD_HPP
