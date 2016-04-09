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

#ifndef WEOS_OBJECTPOOL_HPP
#define WEOS_OBJECTPOOL_HPP

#inlcude "memory.hpp"
#include "memorypool.hpp"


WEOS_BEGIN_NAMESPACE

//! An object pool with static (compile-time) storage.
//!
//! The object_pool is a memory pool for (\p TNumElem) objects of
//! type \p TElement. The memory is allocated statically (internally in the
//! object), i.e. the pool does not allocate memory from the heap.
//! In addition to the memory_pool, the object_pool constructs and destroys
//! the allocated objects. When allocating from this pool, the element's
//! constructor is invoked using a placement new. Upon destruction, the
//! element's destructor is called before the memory is returned back to the
//! pool.
template <typename TElement, std::size_t TNumElem>
class object_pool
{
    typedef memory_pool<TElement, TNumElem> memory_pool_t;

    struct Deleter
    {
        typedef void* pointer;

        Deleter(memory_pool_t& pool)
            : m_pool(pool)
        {
        }

        void operator()(pointer p) noexcept
        {
            m_pool.free(p);
        }

    private:
        memory_pool_t& m_pool;
    };

public:
    //! The type of the elements which can be allocated via this pool.
    typedef TElement element_type;

    object_pool() = default;

    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;

    ~object_pool()
    {
        //! \todo 1. order the free list (insert an order() function into
        //!          the storage
        //!       2. traverse the ordered free list - if the 'next'-pointer
        //!          does not point to the following chunk, the chunk is
        //!          occupied by a live object whose destructor needs to be
        //!          called
    }

    //! Returns the pool's capacity.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const noexcept
    {
        return TNumElem;
    }

    //! Checks if the pool is empty.
    //! Returns \p true, if the pool is empty and no more object can be
    //! allocated.
    bool empty() const noexcept
    {
        return m_memoryPool.empty();
    }

    //! Allocates and constructs an object.
    //! Allocates memory for an object and calls its constructor. The method
    //! returns a pointer to the newly created object or a null-pointer if no
    //! memory was available.
    template <typename... TArgs>
    element_type* try_construct(TArgs&&... args)
    {
        unique_ptr<void, Deleter> mem(m_memoryPool.try_allocate(),
                                      Deleter(m_memoryPool));
        if (!mem)
            return 0;
        new (mem.get()) element_type(std::forward<TArgs>(args)...);
        return static_cast<element_type*>(mem.release());
    }

    //! Destroys an element.
    //! Destroys the \p element whose memory must have been allocated via
    //! this object pool and whose constructor must have been called.
    void destroy(element_type* element) noexcept
    {
        element->~element_type();
        m_memoryPool.free(element);
    }

private:
    //! The pool from which the memory for the elements is allocated.
    memory_pool_t m_memoryPool;
};

//! A shared object pool.
//!
//! The shared_object_pool is a pool for the creation of elements of type
//! \p TElement. The necessary memory for up to (\p TNumElem) elements is
//! held internally, i.e. no memory is allocated dynamically.
//!
//! As the shared_object_pool uses a shared_memory_pool internally, it is
//! thread-safe.
template <typename TElement, std::size_t TNumElem>
class shared_object_pool
{
    typedef shared_memory_pool<TElement, TNumElem> memory_pool_t;

    struct Deleter
    {
        typedef void* pointer;

        Deleter(memory_pool_t& pool)
            : m_pool(pool)
        {
        }

        void operator()(pointer p) noexcept
        {
            m_pool.free(p);
        }

    private:
        memory_pool_t& m_pool;
    };

public:
    //! The type of the elements which can be allocated via this pool.
    typedef TElement element_type;

    shared_object_pool() = default;

    shared_object_pool(const shared_object_pool&) = delete;
    shared_object_pool& operator=(const shared_object_pool&) = delete;

    ~shared_object_pool()
    {
        //! \todo Sort and delete the objects
    }

    //! Returns the pool's capacity.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const noexcept
    {
        return TNumElem;
    }

    //! Checks if the pool is empty.
    //! Returns \p true if the pool is empty.
    bool empty() const
    {
        return m_memoryPool.empty();
    }

    //! Returns the number of available elements.
    std::size_t size() const
    {
        return m_memoryPool.size();
    }

    //! Constructs an object.
    //! Allocates memory and constructs an element in it. The method
    //! returns a pointer to the newly constructed object. If the pool is empty,
    //! the calling thread is blocked until an element has been returned.
    template <typename... TArgs>
    element_type* construct(TArgs&&... args)
    {
        unique_ptr<void, Deleter> mem(m_memoryPool.allocate(),
                                      Deleter(m_memoryPool));
        new (mem.get()) element_type(std::forward<TArgs>(args)...);
        return static_cast<element_type*>(mem.release());
    }

    //! Tries to construct an object.
    //! Tries to allocate memory and constructs an element in it. Then
    //! a pointer to the newly constructed element is returned. If no memory
    //! is available in the pool, a null-pointer is returned.
    template <typename... TArgs>
    element_type* try_construct(TArgs&&... args)
    {
        unique_ptr<void, Deleter> mem(m_memoryPool.try_allocate(),
                                      Deleter(m_memoryPool));
        if (!mem)
            return 0;
        new (mem.get()) element_type(std::forward<TArgs>(args)...);
        return static_cast<element_type*>(mem.release());
    }

    //! Tries to construct an object with timeout.
    //! Tries to allocate memory and constructs an element in it. Then
    //! a pointer to the newly constructed element is returned. If no memory
    //! is available in the pool, the calling thread is blocked until either
    //! a memory block becomes available or the timeout duration \p d
    //! expires. In the latter case, a null-pointer is returned.
    template <typename RepT, typename PeriodT, typename... TArgs>
    element_type* try_construct_for(const chrono::duration<RepT, PeriodT>& d,
                                    TArgs... args)
    {
        unique_ptr<void, Deleter> mem(m_memoryPool.try_allocate(),
                                      Deleter(m_memoryPool));
        if (!mem)
            return 0;
        new (mem.get()) element_type(std::forward<TArgs>(args)...);
        return static_cast<element_type*>(mem.release());
    }

    //! Destroys an element.
    //! Destroys the \p element whose memory must have been allocated via
    //! this object pool. The destructor of \p element is called before
    //! its memory is returned.
    //!
    //! \sa construct()
    void destroy(element_type* element) noexcept
    {
        element->~element_type();
        m_memoryPool.free(element);
    }

private:
    //! The pool from which the memory for the elements is allocated.
    memory_pool_t m_memoryPool;
};

WEOS_END_NAMESPACE

#endif // WEOS_OBJECTPOOL_HPP
