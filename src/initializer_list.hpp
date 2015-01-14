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

#ifndef WEOS_INITIALIZER_LIST_HPP
#define WEOS_INITIALIZER_LIST_HPP

#include "config.hpp"

#include "_core.hpp"


#ifdef __CC_ARM
// -----------------------------------------------------------------------------
// ARMCC
// -----------------------------------------------------------------------------

#include <cstddef>

// Note: The following code fragment is intentionally placed in namespace std.
// Otherwise, the ARMCC compiler won't generate initializer_list's.
namespace std
{

template <typename TType>
class initializer_list
{
public:
    typedef TType value_type;
    typedef const TType& reference;
    typedef const TType& const_reference;
    typedef std::size_t size_type;
    typedef const TType* iterator;
    typedef const TType* const_iterator;

    inline constexpr initializer_list() noexcept
        : m_begin(0),
          m_size(0)
    {
    }

    inline constexpr size_type size() const noexcept
    {
        return m_size;
    }

    inline constexpr const TType* begin() const noexcept
    {
        return m_begin;
    }

    inline constexpr const TType* end() const noexcept
    {
        return m_begin + m_size;
    }

private:
    const TType* m_begin;
    size_type m_size;

    // This constructor is used by ARMCC to create an initializer_list.
    inline constexpr initializer_list(const TType* begin, size_type size) noexcept
        : m_begin(begin),
          m_size(size)
    {
    }
};

template <typename TType>
constexpr const TType* begin(initializer_list<TType> list) noexcept
{
    return list.begin();
}

template <typename TType>
constexpr const TType* end(initializer_list<TType> list) noexcept
{
    return list.end();
}

} // namespace std

#else
// -----------------------------------------------------------------------------
// C++11 conforming STL
// -----------------------------------------------------------------------------

#include <initializer_list>

#endif // __CC_ARM

WEOS_BEGIN_NAMESPACE

using std::begin;
using std::initializer_list;
using std::end;

WEOS_END_NAMESPACE

#endif // WEOS_INITIALIZER_LIST_HPP
