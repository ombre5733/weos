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

#ifndef WEOS_COMMON_INTRUSIVE_PTR_HPP
#define WEOS_COMMON_INTRUSIVE_PTR_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include <algorithm> // for std::swap


WEOS_BEGIN_NAMESPACE

struct keep_reference_count_t {};

constexpr keep_reference_count_t keep_reference_count = keep_reference_count_t();

//! A smart pointer for intrusive reference counting.
//!
//! Two more functions are required in combination with the intrusive_ptr<T>:
//!
//! void intrusive_ptr_add_ref(T* t);
//! void intrusive_ptr_release_ref(T* t);
template <typename TType>
class intrusive_ptr
{
public:
    typedef TType element_type;
    typedef TType* pointer;

    //! Creates an intrusive pointer equivalent to nullptr.
    constexpr intrusive_ptr() noexcept
        : m_pointer()
    {
    }

    //! Creates an intrusive pointer equivalent to nullptr.
    constexpr intrusive_ptr(nullptr_t) noexcept
        : m_pointer()
    {
    }

    //! Creates an intrusive pointer to an object given by \p ptr.
    explicit intrusive_ptr(pointer ptr)
        : m_pointer(ptr)
    {
        if (m_pointer)
            intrusive_ptr_add_ref(m_pointer);
    }

    //! Creates an intrusive pointer, which manages the object given
    //! by \p ptr. The reference count of that object is not increased.
    constexpr intrusive_ptr(pointer ptr, keep_reference_count_t) noexcept
        : m_pointer(ptr)
    {
    }

    //! Copy-constructs an intrusive pointer from the \p other pointer.
    intrusive_ptr(const intrusive_ptr& other)
        : m_pointer(other.m_pointer)
    {
        if (m_pointer)
            intrusive_ptr_add_ref(m_pointer);
    }

    //! Move-constructs an intrusive pointer by moving from the \p other
    //! pointer.
    intrusive_ptr(intrusive_ptr&& other)
        : m_pointer(other.m_pointer)
    {
        other.m_pointer = pointer();
    }

    //! Destroys the intrusive pointer.
    //! The reference count of the managed object is decreased by one.
    ~intrusive_ptr()
    {
        reset();
    }

    //! Copy-assigns the \p other pointer to this pointer.
    intrusive_ptr& operator=(const intrusive_ptr& other)
    {
        if (this != &other)
        {
            if (m_pointer)
                intrusive_ptr_release_ref(m_pointer);
            m_pointer = other.m_pointer;
            if (m_pointer)
                intrusive_ptr_add_ref(m_pointer);
        }

        return *this;
    }

    //! Move-assigns the \p other pointer to this pointer.
    intrusive_ptr& operator=(intrusive_ptr&& other)
    {
        if (this != &other)
        {
            if (m_pointer)
                intrusive_ptr_release_ref(m_pointer);

            m_pointer = other.m_pointer;
            other.m_pointer = pointer();
        }

        return *this;
    }

    //! Resets this pointer.
    intrusive_ptr& operator=(nullptr_t)
    {
        if (m_pointer)
            intrusive_ptr_release_ref(m_pointer);
        m_pointer = pointer();

        return *this;
    }

    //! Releases the ownership.
    //! Transfers the ownership of the stored object to the caller.
    pointer release() noexcept
    {
        pointer temp = m_pointer;
        m_pointer = pointer();
        return temp;
    }

    //! Decreases the reference count of the owned object and destroys it,
    //! if the reference counter reaches zero. Then the ownership of the
    //! given \p ptr is taken.
    void reset(pointer ptr = pointer()) /*noexcept?*/
    {
        if (m_pointer)
            intrusive_ptr_release_ref(m_pointer);
        m_pointer = ptr;
        if (m_pointer)
            intrusive_ptr_add_ref(m_pointer);
    }

    //! Returns a reference to the managed object.
    typename add_lvalue_reference<element_type>::type operator*() const /*noexcept?*/
    {
        WEOS_ASSERT(m_pointer);
        return *m_pointer;
    }

    //! Accesses the owned object.
    pointer operator->() const noexcept
    {
        WEOS_ASSERT(m_pointer);
        return m_pointer;
    }

    //! Returns a pointer to the managed object.
    pointer get() const noexcept
    {
        return m_pointer;
    }

    //! Returns \p true, if the managed pointer is not a nullptr.
    explicit operator bool() const noexcept
    {
        return m_pointer;
    }

    //! Swaps this intrusive pointer with the \p other pointer.
    void swap(intrusive_ptr& other) noexcept
    {
        using std::swap;
        swap(m_pointer, other.m_pointer);
    }

    bool operator==(const intrusive_ptr& other) const noexcept
    {
        return m_pointer == other.m_pointer;
    }

    bool operator!=(const intrusive_ptr& other) const noexcept
    {
        return !(m_pointer == other.m_pointer);
    }

private:
    //! A pointer to the managed object.
    pointer m_pointer;
};

//! Swaps two intrusive pointers \p a and \p b.
template <typename TType>
void swap(intrusive_ptr<TType>& a, intrusive_ptr<TType>& b) noexcept
{
    a.swap(b);
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_INTRUSIVE_PTR_HPP
