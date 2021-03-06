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

#ifndef WEOS_ARMCC_MEMORY_HPP
#define WEOS_ARMCC_MEMORY_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"

#include <boost/version.hpp>
#if BOOST_VERSION > 105500
    #include <boost/core/addressof.hpp>
#else
    #include "string.h"
    #include <boost/utility/addressof.hpp>
#endif
#include <boost/container/scoped_allocator.hpp>


namespace std
{

// ----=====================================================================----
//     default_delete
// ----=====================================================================----

//! The default deleter.
//!
//! default_delete is the default deleter used by unique_ptr. It calles
//! <tt>delete</tt> in order to delete an object.
template <typename T>
struct default_delete
{
    constexpr
    default_delete() noexcept = default;

    template <typename U>
    default_delete(const default_delete<U>&,
                   typename enable_if<is_convertible<U*, T*>::value>::type* = 0) noexcept
    {
    }

    void operator() (T* ptr) const
    {
        delete ptr;
    }
};

// TODO: add a default_delete specialization for arrays

// ----=====================================================================----
//     unique_ptr
// ----=====================================================================----

namespace weos_detail
{

// Checks if T has a member named 'pointer', i.e. if T::pointer exists.
template <typename T>
class has_pointer_member
{
    struct two
    {
        char a;
        char b;
    };

    template <typename U>
    static char test(typename U::pointer* = 0);

    template <typename U>
    static two test(...);

public:
    static const bool value = sizeof(test<T>(0)) == 1;
};

// Default case when TDeleter::pointer exists.
template <typename TType, typename TDeleter,
          bool THasPointerField = has_pointer_member<TDeleter>::value>
struct deleter_pointer_or_fallback
{
    typedef typename TDeleter::pointer type;
};

// Fallback when TDeleter::pointer does no exist.
template <typename TType, typename TDeleter>
struct deleter_pointer_or_fallback<TType, TDeleter, false>
{
    typedef TType* type;
};

} // namespace weos_detail

//! A unique pointer.
template <typename TPointee, typename TDeleter = default_delete<TPointee> >
class unique_ptr
{
public:
    typedef TPointee element_type;
    typedef TDeleter deleter_type;
    typedef typename weos_detail::deleter_pointer_or_fallback<
                element_type,
                typename remove_reference<deleter_type>::type>::type
            pointer;

    //! Creates a unique pointer equivalent to nullptr.
    constexpr
    unique_ptr() noexcept
        : m_pointerDeleter(pointer(), deleter_type())
    {
    }

    //! Creates a unique pointer equivalent to nullptr.
    constexpr
    unique_ptr(nullptr_t) noexcept
        : m_pointerDeleter(pointer(), deleter_type())
    {
    }

    //! Creates a unique pointer which owns the given \p ptr.
    explicit
    unique_ptr(pointer ptr) noexcept
        : m_pointerDeleter(ptr, deleter_type())
    {
    }

    // 20.8.1.2.1
    // Type of the deleter:
    // If TDeleter is a non-reference type A --> const A&
    // If TDeleter is an lvalue reference type A& --> A&
    // If TDeleter is an lvalue reference type const A& --> const A&
    unique_ptr(pointer ptr,
               typename conditional<
                   is_reference<TDeleter>::value,
                   TDeleter,
                   typename add_lvalue_reference<
                       typename add_const<TDeleter>::type
                   >::type
               >::type deleter)
        : m_pointerDeleter(ptr, deleter)
    {
    }

    // 20.8.1.2.1
    // Type of the deleter:
    // If TDeleter is a non-reference type A --> A&&
    // If TDeleter is an lvalue reference type A& --> A&&
    // If TDeleter is an lvalue reference type const A& --> const A&&
    unique_ptr(pointer ptr,
               typename add_rvalue_reference<
                   typename remove_reference<TDeleter>::type
               >::type deleter)
        : m_pointerDeleter(ptr, std::move(deleter))
    {
    }

    //! Move construction.
    //! Creates a unique pointer by moving from the \p other pointer.
    unique_ptr(unique_ptr&& other) noexcept
        :  m_pointerDeleter(other.release(),
                            std::forward<TDeleter>(other.get_deleter()))
    {
    }

    // TODO: add missing move constructor which converts deleter_type's

    //! Destroys the pointer and the managed packet.
    ~unique_ptr()
    {
        reset();
    }

    //! Assigns a nullptr.
    //! Resets this pointer to a nullptr.
    unique_ptr& operator=(nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    //! Move assignment.
    //! Moves the \p other pointer to this pointer.
    unique_ptr& operator=(unique_ptr&& other) noexcept
    {
        reset(other.release());
        std::get<1>(m_pointerDeleter)
                = std::forward<TDeleter>(other.get_deleter());
        return *this;
    }

    // TODO: move assignment which converts deleter_type's

    //! Returns a plain pointer to the owned object.
    //! Returns a plain pointer to the owned object without giving up the
    //! ownership.
    pointer get() const noexcept
    {
        return std::get<0>(m_pointerDeleter);
    }

    //! Returns the associated deleter.
    deleter_type& get_deleter() noexcept
    {
        return std::get<1>(m_pointerDeleter);
    }

    //! Returns the associated deleter.
    const deleter_type& get_deleter() const noexcept
    {
        return std::get<1>(m_pointerDeleter);
    }

    //! Releases the ownership.
    //! Transfers the ownership of the stored object to the caller.
    pointer release() noexcept
    {
        pointer temp = std::get<0>(m_pointerDeleter);
        std::get<0>(m_pointerDeleter) = pointer();
        return temp;
    }

    //! Destroys the owned object and takes the ownership of the given \p ptr.
    void reset(pointer ptr = pointer()) noexcept
    {
        // Watch out for self-assignment.
        if (std::get<0>(m_pointerDeleter) != ptr)
        {
            // Note: The order of these expressions is important.
            // See 20.8.1.2.5.
            pointer temp = std::get<0>(m_pointerDeleter);
            std::get<0>(m_pointerDeleter) = ptr;
            if (temp)
                std::get<1>(m_pointerDeleter)(temp);
        }
    }

    //! Swaps this unique_ptr with the \p other unique_ptr.
    void swap(unique_ptr& other) noexcept
    {
        using std::swap;
        swap(m_pointerDeleter, other.m_pointerDeleter);
    }

    //! Returns a reference to the owned object.
    typename add_lvalue_reference<element_type>::type operator*() const
    {
        return *std::get<0>(m_pointerDeleter);
    }

    //! Accesses the owned object.
    pointer operator->() const noexcept
    {
        return std::get<0>(m_pointerDeleter);
    }

    //! Checks if the smart pointer owns an object.
    //! Returns \p true, if this smart pointer owns an object.
    explicit
    operator bool() const noexcept
    {
        return std::get<0>(m_pointerDeleter) != nullptr;
    }

private:
    //! A pointer to the owned object.
    tuple<pointer, deleter_type> m_pointerDeleter;

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
};

//! Swaps two unique_ptrs \p a and \p b.
template <typename TPointee, typename TDeleter>
inline
void swap(unique_ptr<TPointee, TDeleter>& a, unique_ptr<TPointee, TDeleter>& b)
{
    a.swap(b);
}

// TODO: add unique_ptr for arrays

template <typename T1, typename D1, typename T2, typename D2>
inline
bool operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
    return x.get() == y.get();
}

template <typename T1, typename D1>
inline
bool operator==(const unique_ptr<T1, D1>& x, nullptr_t) noexcept
{
    return !x;
}

template <typename T1, typename D1>
inline
bool operator==(nullptr_t, const unique_ptr<T1, D1>& x) noexcept
{
    return !x;
}

template <class T, class D>
inline
bool operator!=(const unique_ptr<T, D>& x, nullptr_t) noexcept
{
    return static_cast<bool>(x);
}

template <class T, class D>
inline
bool operator!=(nullptr_t, const unique_ptr<T, D>& x) noexcept
{
    return static_cast<bool>(x);
}


// TODO: add other operators

// TODO: Must not participate in overload resolution for arrays.
template <typename TType, typename... TArgs>
unique_ptr<TType> make_unique(TArgs&&... args )
{
    return unique_ptr<TType>(new TType(std::forward<TArgs>(args)...));
}

// ----=====================================================================----
//     allocator
// ----=====================================================================----

using boost::addressof;

using boost::container::allocator_traits;
using boost::container::allocator_arg;
using boost::container::uses_allocator;

struct allocator_arg_t {};

// ----=====================================================================----
//     align
// ----=====================================================================----

void* align(std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space);

} // namespace std

#endif // WEOS_ARMCC_MEMORY_HPP
