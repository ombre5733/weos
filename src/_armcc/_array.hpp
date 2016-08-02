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

#ifndef WEOS_ARMCC_ARRAY_HPP
#define WEOS_ARMCC_ARRAY_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../exception.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>


namespace std
{

template <typename TType, std::size_t TSize>
struct array
{
    typedef TType value_type;

    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    typedef value_type&       reference;
    typedef const value_type& const_reference;

    typedef value_type*       pointer;
    typedef const value_type* const_pointer;

    typedef value_type*                           iterator;
    typedef const value_type*                     const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // ---- Element access

    reference operator[](size_type index)
    {
        return _m_data[index];
    }

    constexpr
    const_reference operator[](size_type index) const
    {
        return _m_data[index];
    }

    reference at(size_type index)
    {
#ifdef WEOS_ENABLE_EXCEPTIONS
        if (index >= TSize)
            throw WEOS_EXCEPTION(std::out_of_range("array::at()"));
#endif // WEOS_ENABLE_EXCEPTIONS
        WEOS_ASSERT(index < TSize);

        return _m_data[index];
    }

    constexpr
    const_reference at(size_type index) const
    {
#ifdef WEOS_ENABLE_EXCEPTIONS
        return index < TSize ? _m_data[index] : throw WEOS_EXCEPTION(std::out_of_range("array::at()"));
#else
        return index < TSize ? _m_data[index] : asserting_at(index);
#endif // WEOS_ENABLE_EXCEPTIONS
    }

    reference front()
    {
        WEOS_ASSERT(TSize > 0);
        return _m_data[0];
    }

    constexpr
    const_reference front() const
    {
        return _m_data[0];
    }

    reference back()
    {
        WEOS_ASSERT(TSize > 0);
        return _m_data[TSize > 0 ? TSize - 1 : 0];
    }

    constexpr
    const_reference back() const
    {
        return _m_data[TSize > 0 ? TSize - 1 : 0];
    }

    value_type* data() noexcept
    {
        return _m_data;
    }

    const value_type* data() const noexcept
    {
        return _m_data;
    }

    // ---- Iterators

    iterator begin() noexcept
    {
        return iterator(_m_data);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(_m_data);
    }

    const_iterator cbegin() const noexcept
    {
        return const_iterator(_m_data);
    }

    iterator end() noexcept
    {
        return iterator(_m_data + TSize);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(_m_data + TSize);
    }

    const_iterator cend() const noexcept
    {
        return const_iterator(_m_data + TSize);
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(_m_data + TSize);
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(_m_data + TSize);
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(_m_data + TSize);
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(_m_data);
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(_m_data);
    }

    const_reverse_iterator crend() const noexcept
    {
        return reverse_iterator(_m_data);
    }

    // ---- Capacity

    constexpr
    bool empty() const noexcept
    {
        return TSize == 0;
    }

    constexpr
    size_type size() const noexcept
    {
        return TSize;
    }

    constexpr
    size_type max_size() const noexcept
    {
        return TSize;
    }

    // ---- Operations

    void fill(const value_type& value)
    {
        std::fill_n(_m_data, TSize, value);
    }

    void swap(array& other) // TODO: noexcept
    {
        std::swap_ranges(_m_data, _m_data + TSize, other._m_data);
    }



    // The array elements.
    value_type _m_data[TSize > 0 ? TSize : 1];

private:
    const_reference asserting_at(size_type index) const
    {
        WEOS_ASSERT(index < TSize);
        return _m_data[index];
    }
};

template <typename TType, std::size_t TSize>
inline
bool operator==(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return std::equal(x._m_data, x._m_data + TSize, y._m_data);
}

template <typename TType, std::size_t TSize>
inline
bool operator!=(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return !(x == y);
}

template <typename TType, std::size_t TSize>
inline
bool operator<(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return std::lexicographical_compare(x._m_data, x._m_data + TSize,
                                        y._m_data, y._m_data + TSize);
}

template <typename TType, std::size_t TSize>
inline
bool operator<=(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return !(y < x);
}

template <typename TType, std::size_t TSize>
inline
bool operator>(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return y < x;
}

template <typename TType, std::size_t TSize>
inline
bool operator>=(const array<TType, TSize>& x, const array<TType, TSize>& y)
{
    return !(x < y);
}

// Swaps the two arrays \p x and \p y.
template <typename TType, std::size_t TSize>
void swap(array<TType, TSize>& x, array<TType, TSize>& y)
{
    x.swap(y);
}

// TODO: tuple_size

} // namespace std

#endif  // WEOS_ARMCC_ARRAY_HPP
