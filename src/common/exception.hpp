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

#ifndef WEOS_COMMON_EXCEPTION_HPP
#define WEOS_COMMON_EXCEPTION_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#define WEOS_THROW_EXCEPTION(exc)                                              \
    throw WEOS_NAMESPACE::enable_current_exception(exc)

// ----=====================================================================----
//     exception
// ----=====================================================================----

class exception
{
public:
    virtual ~exception() throw() = 0;

protected:
    exception()
        : m_errorInfoList()
    {
    }

private:
    mutable void* m_errorInfoList;
};

inline
exception::~exception() throw()
{
}

namespace detail_exception
{
void cloneErrorInfoList(const exception* src, exception* dest)
{
}

void cloneErrorInfoList(const void* /*src*/, void* /*dest*/)
{
}

} // namespace detail_exception


// ----=====================================================================----
//
// ----=====================================================================----

#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include "exception_impl_armcc.hpp"

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <exception>


WEOS_BEGIN_NAMESPACE

template <typename TType>
inline
TType&& enable_current_exception(TType&& exc)
{
    return exc;
}

using std::current_exception;
using std::exception_ptr;
using std::rethrow_exception;

WEOS_END_NAMESPACE

#endif // __CC_ARM

#endif // WEOS_COMMON_EXCEPTION_HPP
