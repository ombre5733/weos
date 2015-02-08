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

#ifndef WEOS_SCOPEGUARD_IMPL_CXX11_HPP
#define WEOS_SCOPEGUARD_IMPL_CXX11_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../functional.hpp"
#include "../utility.hpp"
#include "../type_traits.hpp"


WEOS_BEGIN_NAMESPACE

namespace detail
{

//! \brief A scope guard for bind() results.
//!
//! This class is a scope guard, which calls a bind() expression when it
//! goes out of scope.
template <typename TBindResult>
class ScopeGuardImpl : public ScopeGuardBase
{
public:
    //! \brief Creates a scope guard.
    //!
    //! Creates a scope guard by binding the given \p callable and the
    //! arguments \p args as if <tt>bind(callable, args)</tt> was called.
    // Note: This constructor must not take part in overload resolution, if
    // the argument is a scope guard. Otherwise, the copy constructor would
    // be bypassed and the compiler tries to create a scope guard by binding
    // another scope guard.
    template <typename TCallable, typename... TArgs,
              typename _ = typename WEOS_NAMESPACE::enable_if<
                               !WEOS_NAMESPACE::is_same<TCallable, ScopeGuardImpl>::value
                           >::type>
    ScopeGuardImpl(TCallable&& callable, TArgs&&... args)
        : m_bindResult(WEOS_NAMESPACE::bind(WEOS_NAMESPACE::forward<TCallable>(callable),
                                            WEOS_NAMESPACE::forward<TArgs>(args)...))
    {
    }

    //! \brief Copy-constructs a scope guard.
    //!
    //! Creates a scope guard by copying the \p other guard.
    ScopeGuardImpl(const ScopeGuardImpl& other)
        : m_bindResult(other.m_bindResult)
    {
    }

    //! \brief Destroys the scope guard.
    //!
    //! Destroys the scope guard and calls the bound expression, if the guard
    //! has not been dismissed.
    ~ScopeGuardImpl()
    {
        if (!this->m_dismissed)
            m_bindResult();
    }

private:
    //! The result of the bind expression.
    TBindResult m_bindResult;
};

} // namespace detail


//! \brief Creates a scope guard.
//!
//! Creates a scope guard which will call the callable \p f with the arguments
//! \p args when it goes out of scope. In the following example, a scope
//! guard is used to call fclose(), no matter how the scope is left. The file
//! is closed even if an exception is thrown.
//! \code
//! FILE* handle = fopen("some_file.txt", "rb");
//! if (!handle)
//!     return;
//! // This guard calls   fclose(handle)   when it goes out of scope.
//! ScopeGuard myGuard = makeScopeGuard(fclose, handle);
//! // Operate on the file and let the guard close it.
//! \endcode
template <typename TCallable, typename... TArgs>
auto makeScopeGuard(TCallable&& f, TArgs&&... args)
    -> detail::ScopeGuardImpl<
           decltype(WEOS_NAMESPACE::bind(WEOS_NAMESPACE::forward<TCallable>(f),
                                         WEOS_NAMESPACE::forward<TArgs>(args)...))>
{
    typedef decltype(std::bind(WEOS_NAMESPACE::forward<TCallable>(f),
                               WEOS_NAMESPACE::forward<TArgs>(args)...)) type;
    return detail::ScopeGuardImpl<type>(WEOS_NAMESPACE::forward<TCallable>(f),
                                        WEOS_NAMESPACE::forward<TArgs>(args)...);
}

WEOS_END_NAMESPACE

#endif // WEOS_SCOPEGUARD_IMPL_CXX11_HPP
