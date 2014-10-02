/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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

#ifndef WEOS_COMMON_SCOPEGUARD_HPP
#define WEOS_COMMON_SCOPEGUARD_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


WEOS_BEGIN_NAMESPACE

//! \brief The base class for scope guards.
//!
//! The ScopeGuardBase is the base class of all scope guards.
class ScopeGuardBase
{
public:
    //! \brief Dismiss the scope guard.
    //!
    //! Dismisses the scope guard, i.e. the clean-up action won't be called
    //! on scope exit.
    void dismiss() const WEOS_NOEXCEPT
    {
        m_dismissed = true;
    }

protected:
    //! Creates a scope guard.
    ScopeGuardBase()
        : m_dismissed(false)
    {
    }

    //! This flag will be set, if the guard has been dismissed.
    mutable bool m_dismissed;

private:
    // ---- Hidden methods.
    ScopeGuardBase& operator= (const ScopeGuardBase&);
};

//! \brief A scope guard.
//!
//! The ScopeGuard is type which shall be used to hold the result returned by
//! makeScopeGuard().
typedef const ScopeGuardBase& ScopeGuard;


// Helper macros for the generation of anonymous variables.
#define WEOS_CONCATENATE_HELPER(a, b)   a ## b
#define WEOS_CONCATENATE(a, b)          WEOS_CONCATENATE_HELPER(a, b)
#define WEOS_ANONYMOUS_VARIABLE(name)   WEOS_CONCATENATE(name, __LINE__)


//! \brief Creates an anonymous scope guard.
//!
//! Use this macro to create an anonymous scope guard. The macro is equivalent
//! to the C++11 code
//! \code
//! auto anonymous = makeScopeGuard(__VA_ARGS__);
//! \endcode
//! where \p anonymous is an automatically generated unique name.
#define WEOS_ON_SCOPE_EXIT(...)                                                \
    WEOS_NAMESPACE::ScopeGuard WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) =    \
    WEOS_NAMESPACE::makeScopeGuard(__VA_ARGS__);                               \
    (void)WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_);


WEOS_END_NAMESPACE

#endif // WEOS_COMMON_SCOPEGUARD_HPP
