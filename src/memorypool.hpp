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

#ifndef WEOS_MEMORYPOOL_HPP
#define WEOS_MEMORYPOOL_HPP

#include "mutex.hpp"
#include "semaphore.hpp"

#include <boost/integer/static_min_max.hpp>
#include <boost/math/common_factor_ct.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

namespace weos
{

namespace detail
{

//! A free list.
//! The free_list keeps a list of memory chunks which can be allocated by
//! a memory pool.
class free_list
{
public:
    //! Creates a free list.
    //! Creates a free list in the memory pointed to by \p memBlock. The size
    //! of one chunk is \p chunkSize and the size of the memory block is
    //! passed in \p memSize.
    //! The memory \p memBlock must be suitable aligned to hold a <tt>void*</tt>
    //! and the \p chunkSize must be a multiple of
    //! <tt>sizeof(void*)</tt> to ensure correct alignment.
    free_list(void* memBlock, std::size_t chunkSize, std::size_t memSize)
        : m_first(memBlock)
    {
        // Make memSize a multiple of chunkSize.
        memSize = (memSize / chunkSize) * chunkSize;
        if (memSize == 0)
        {
            m_first = 0;
            return;
        }

        // Compute the location of the last chunk and terminate it with a
        // null-pointer.
        char* last = static_cast<char*>(memBlock) + memSize - chunkSize;
        next(last) = 0;

        char* iter = static_cast<char*>(memBlock);
        while (iter != last)
        {
            char* follow = iter + chunkSize;
            next(iter) = follow;
            iter = follow;
        }
    }

    //! Checks if the free list is empty.
    //! Returns \p true if the free list is empty and no further chunk can
    //! be allocated.
    bool empty() const BOOST_NOEXCEPT
    {
        return m_first == 0;
    }

    //! Allocates a memory chunk.
    //! Returns the first available memory chunk from the list.
    //!
    //! \warning This method must not be called, if the list is empty.
    void* allocate()
    {
        void* chunk = m_first;
        m_first = next(m_first);
        return chunk;
    }

    //! Returns a memory \p chunk back to the list.
    void free(void* const chunk)
    {
        next(chunk) = m_first;
        m_first = chunk;
    }

private:
    //! Pointer to the first free block.
    void* m_first;

    //! Returns a reference to the next pointer.
    static void*& next(void* const p)
    {
        return *static_cast<void**>(p);
    }

    // Hidden copy constructor and assignment operator.
    free_list(const free_list&);
    const free_list& operator= (const free_list&);
};

} // namespace detail

//! A memory pool.
//! A memory_pool provides storage for (\p TNumElem) elements of
//! type \p TElement. The storage is allocated statically, i.e. the pool
//! does not acquire memory from the heap.
template <typename TElement, std::size_t TNumElem, typename TMutex = null_mutex>
class memory_pool : private TMutex
{
public:
    //! The type of the elements stored in the pool.
    typedef TElement element_type;
    //! The type of the mutex which protects the internal data from concurrent
    //! modifications.
    typedef TMutex mutex_type;
    //! The type of the pool.
    typedef memory_pool<TElement, TNumElem, TMutex> pool_t;

private:
    // A chunk has to be aligned such that it can contain a void* or an element.
    static const std::size_t min_align =
        ::boost::math::static_lcm< ::boost::alignment_of<void*>::value,
                                   ::boost::alignment_of<element_type>::value>::value;
    // The chunk size has to be large enough to store a void* or an element.
    // Further it must be a multiple of the alignment.
    static const std::size_t chunk_size =
        ::boost::math::static_lcm<
            ::boost::static_unsigned_max<sizeof(void*), sizeof(element_type)>::value,
            min_align>::value;
    // The memory block must be able to hold TNumElem elements.
    static const std::size_t block_size = chunk_size * TNumElem;

public:
    //! Creates a memory pool.
    //! Creates a memory pool with statically allocated storage.
    memory_pool()
        : m_freeList(m_data.address(), chunk_size, block_size)
    {
    }

    //! Returns the number of pool elements.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const
    {
        return TNumElem;
    }

    //! Checks if the memory pool is empty.
    //! Returns \p true, if the memory pool is empty.
    bool empty() const
    {
        lock_guard<mutex_type> lock(*const_cast<pool_t*>(this));
        return m_freeList.empty();
    }

    //! Allocates a chunk from the pool.
    //! Allocates one chunk from the memory pool and returns a pointer to it.
    //! If the pool is already empty, a null-pointer is returned.
    //!
    //! \sa free()
    void* try_allocate()
    {
        lock_guard<mutex_type> lock(*this);
        if (m_freeList.empty())
            return 0;
        else
            return m_freeList.allocate();
    }

    //! Frees a previously allocated chunk.
    //! Returns a \p chunk which must have been allocated via allocate() back
    //! to the pool.
    //!
    //! \sa allocate()
    void free(void* const chunk)
    {
        lock_guard<mutex_type> lock(*this);
        m_freeList.free(chunk);
    }

private:
    //! The aligned data block for the memory chunks.
    typename ::boost::aligned_storage<block_size, min_align>::type m_data;
    //! A list of free (i.e. not allocated) memory chunks.
    detail::free_list m_freeList;
};

//! A counting memory pool.
//! A counting memory pool is an extension to memory_pool. Internally it holds
//! memory for (\p TNumElem) elements of type \p TElement. In addition, it
//! keeps track of the number of elements available in the pool and can thus
//! block the calling thread until an element becomes available.
//!
//! The counting_memory_pool is always thread safe. Multiple threads can
//! concurrently use it to allocate and free memory chunks.
template <typename TElement, std::size_t TNumElem>
class counting_memory_pool
{
public:
    //! The type of the elements in this pool.
    typedef TElement element_type;

    //! Constructs a counting memory pool.
    counting_memory_pool()
        : m_numElements(TNumElem)
    {
    }

    //! Returns the number of pool elements.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const
    {
        return TNumElem;
    }

    //! Checks if the pool is empty.
    bool empty() const
    {
        return m_memoryPool.empty();
    }

    //! Returns the number of available elements.
    std::size_t size() const
    {
        return m_numElements.value();
    }

    //! Allocates a chunk of memory.
    //! Allocates a chunk of memory and returns a pointer to it. The calling
    //! thread is blocked until a chunk is available.
    //!
    //! \sa free(), try_allocate(), try_allocate_for()
    void* allocate()
    {
        m_numElements.wait();
        return m_memoryPool.try_allocate();
    }

    //! Tries to allocate a chunk of memory.
    //! Tries to allocate a chunk of memory and returns a pointer to it. If
    //! no memory is available, a null-pointer is returned.
    //!
    //! \sa allocate(), free(), try_allocate_for()
    void* try_allocate()
    {
        if (m_numElements.try_wait())
            return m_memoryPool.try_allocate();
        else
            return 0;
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
        if (m_numElements.try_wait_for(d))
            return m_memoryPool.try_allocate();
        else
            return 0;
    }

    //! Frees a chunk of memory.
    //! Frees a \p chunk of memory which must have been allocated through
    //! this pool.
    //!
    //! \sa allocate(), try_allocate(), try_allocate_for()
    void free(void* const chunk)
    {
        m_memoryPool.free(chunk);
        m_numElements.post();
    }

private:
    typedef memory_pool<TElement, TNumElem, mutex> pool_t;
    //! The pool from which the memory for the element is allocated.
    pool_t m_memoryPool;
    //! The number of available elements.
    semaphore m_numElements;
};

} // namespace weos

#endif // WEOS_MEMORYPOOL_HPP
