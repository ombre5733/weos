/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
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

#include "memorypool.hpp"

#include <boost/move/move.hpp>

namespace weos
{

//! An object pool with static (compile-time) storage.
//! The object_pool is a memory pool for (\p TNumElem) objects of
//! type \p TElement. The memory is allocated statically (internally in the
//! object), i.e. the pool does not access the heap.
template <typename TElement, unsigned TNumElem, typename TMutex = null_mutex>
class object_pool
{
public:
    typedef TElement element_type;
    typedef TMutex mutex_type;

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
    std::size_t capacity() const
    {
        return TNumElem;
    }

    //! Checks if the pool is empty.
    //! Returns \p true, if the pool is empty and no more object can be
    //! allocated.
    bool empty() const
    {
        return m_memoryPool.empty();
    }

    //! Allocates memory for an object.
    //! Allocates memory for one object and returns a pointer to the allocated
    //! space. The object is not initialized, i.e. the caller has to invoke the
    //! constructor using placement-new. The method returns a pointer to
    //! the allocated memory or a null-pointer if no more memory was available.
    element_type* allocate()
    {
        return static_cast<element_type*>(m_memoryPool.allocate());
    }

    //! Frees a previously allocated storage.
    //! Frees the memory space \p element which has been previously allocated
    //! via this object pool. This function does not destroy the element and
    //! the caller is responsible for calling the destructor, thus.
    void free(element_type* const element)
    {
        m_memoryPool.free(element);
    }

    //! Allocates and constructs an object.
    //! Allocates memory for an object and calls its constructor. The method
    //! returns a pointer to the newly created object or a null-pointer if no
    //! memory was available.
    element_type* construct()
    {
        void* mem = this->allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type;
        return element;
    }

    template <class T1>
    element_type* construct(BOOST_FWD_REF(T1) x1)
    {
        void* mem = this->allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1));
        return element;
    }

    template <class T1, class T2>
    element_type* construct(BOOST_FWD_REF(T1) x1, BOOST_FWD_REF(T2) x2)
    {
        void* mem = this->allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1),
                                    boost::forward<T2>(x2));
        return element;
    }

    //! Destroys an element.
    //! Destroys the \p element whose memory must have been allocated via
    //! this object pool and whose constructor must have been called.
    void destroy(element_type* const element)
    {
        element->~element_type();
        this->free(element);
    }

private:
    typedef memory_pool<TElement, TNumElem, TMutex> pool_t;
    pool_t m_memoryPool;
};

//! The counting_memory_pool is always thread safe. Multiple threads can
//! concurrently use it for the creation and destruction of elements.
template <typename TElement, unsigned TNumElem>
class counting_object_pool
{
public:
    typedef TElement element_type;

    ~counting_object_pool()
    {
        //! \todo Sort and delete the objects
    }

    //! Returns the pool's capacity.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const
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

    //! Allocates an element from the pool.
    //! Allocates an element from the pool and returns a pointer to it. The
    //! calling thread is blocked until an element is available.
    //!
    //! \sa free(), try_allocate(), try_allocate_for()
    element_type* allocate()
    {
        return static_cast<element_type*>(m_memoryPool.allocate());
    }

    //! Tries to allocate a chunk of memory.
    //! Tries to allocate a chunk of memory and returns a pointer to it. If
    //! no memory is available, a null-pointer is returned.
    //!
    //! \sa allocate(), free(), try_allocate_for()
    void* try_allocate()
    {
        return m_memoryPool.try_allocate();
    }

    //! Tries to allocate a chunk of memory with timeout.
    //! Tries to allocate a chunk of memory and returns a pointer to it.
    //! If no memory is available, the method blocks for a duration up to
    //! \p d and returns a null-pointer then.
    //!
    //! \sa allocate(), free(), try_allocate()
    template <typename RepT, typename PeriodT>
    void* try_allocate_for(const chrono::duration<RepT, PeriodT>& d)
    {
        return m_memoryPool.try_allocate_for(d);
    }

    //! Frees a chunk of memory.
    //! Frees a \p chunk of memory which must have been allocated through
    //! this pool.
    //!
    //! \sa allocate(), try_allocate(), try_allocate_for()
    void free(void* const chunk)
    {
        m_memoryPool.free(chunk);
    }

    //! Constructs an object.
    //! Allocates memory for an object and calls its constructor. The method
    //! returns a pointer to the newly created object. If the pool is empty,
    //! the calling thread is blocked until an element has been returned.
    element_type* construct()
    {
        void* mem = this->allocate();
        element_type* element = new (mem) element_type;
        return element;
    }

    template <class T1>
    element_type* construct(BOOST_FWD_REF(T1) x1)
    {
        void* mem = this->allocate();
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1));
        return element;
    }

    template <class T1, class T2>
    element_type* construct(BOOST_FWD_REF(T1) x1, BOOST_FWD_REF(T2) x2)
    {
        void* mem = this->allocate();
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1),
                                    boost::forward<T2>(x2));
        return element;
    }

    //! Tries to construct an object.
    //! Tries to allocate memory for an object, calls its constructor and
    //! returns a pointer to the new object. If no memory is available in the
    //! pool, a null-pointer is returned.
    element_type* try_construct()
    {
        void* mem = this->try_allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type;
        return element;
    }

    template <class T1>
    element_type* try_construct(BOOST_FWD_REF(T1) x1)
    {
        void* mem = this->try_allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1));
        return element;
    }

    template <class T1, class T2>
    element_type* try_construct(BOOST_FWD_REF(T1) x1, BOOST_FWD_REF(T2) x2)
    {
        void* mem = this->try_allocate();
        if (!mem)
            return 0;
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1),
                                    boost::forward<T2>(x2));
        return element;
    }

    //! Destroys an element.
    //! Destroys the \p element whose memory must have been allocated via
    //! this object pool and whose constructor must have been called.
    //!
    //! \sa construct()
    void destroy(element_type* const element)
    {
        element->~element_type();
        this->free(element);
    }

private:
    typedef counting_memory_pool<TElement, TNumElem> pool_t;
    pool_t m_memoryPool;
};

} // namespace weos

#endif // WEOS_OBJECTPOOL_HPP
