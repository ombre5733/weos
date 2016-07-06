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

#ifndef WEOS_MEMORYPOOL_HPP
#define WEOS_MEMORYPOOL_HPP

#include "_config.hpp"

#include "mutex.hpp"
#include "semaphore.hpp"
#include "type_traits.hpp"

#include <cstddef>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

class FreeList
{
public:
    FreeList(void* memory, std::size_t chunkSize,
             std::size_t numElements) noexcept
        : m_first(memory)
    {
        char* iter = static_cast<char*>(memory) + chunkSize * numElements;
        char* prev = 0;
        while (iter > static_cast<char*>(m_first))
        {
            iter -= chunkSize;
            next(iter) = prev;
            prev = iter;
        }
    }

    FreeList(const FreeList&) = delete;
    FreeList& operator=(const FreeList&) = delete;

    bool empty() const noexcept
    {
        return m_first == 0;
    }

    void* first() const noexcept
    {
        return m_first;
    }

    void* try_allocate() noexcept
    {
        if (m_first == 0)
            return 0;

        void* chunk = m_first;
        m_first = next(m_first);
        return chunk;
    }

    void free(void* chunk) noexcept
    {
        next(chunk) = m_first;
        m_first = chunk;
    }

private:
    //! Pointer to the first free block.
    void* m_first;

    //! Returns a reference to the next pointer.
    static void*& next(void* p)
    {
        return *static_cast<void**>(p);
    }
};

} // namespace weos_detail

//! A memory pool.
//! A memory_pool provides storage for (\p TNumElem) elements of
//! type \p TElement. The storage is allocated statically, i.e. the pool
//! does not acquire memory from the heap.
//!
//! The memory_pool is not thread-safe. If it is simultaneously accessed from
//! multiple threads, some kind of external synchronization (e.g. a mutex)
//! has to be used. The shared_memory_pool might be an alternative in this
//! case.
template <typename TElement, std::size_t TNumElem>
class memory_pool
{
public:
    //! The type of the elements stored in the pool.
    typedef TElement element_type;

private:
    static_assert(TNumElem > 0, "The number of elements must be non-zero.");

    // Every chunk has to be aligned such that it can contain either a
    // void* or an element_type.
    static const std::size_t chunk_align =
            alignment_of<void*>::value > alignment_of<element_type>::value
            ? alignment_of<void*>::value
            : alignment_of<element_type>::value;
    // The chunk size has to be large enough to store a void* or an element.
    static const std::size_t chunk_size =
            sizeof(void*) > sizeof(element_type)
            ? sizeof(void*)
            : sizeof(element_type);

    // One chunk must be large enough for a void* or an element_type and it
    // must be aligned to the stricter of both. Furthermore, the alignment
    // must be a multiple of the size. The aligned_storage<> takes care
    // of this.
    typedef typename aligned_storage<chunk_size, chunk_align>::type chunk_type;

public:
    //! Creates a memory pool.
    //! Creates a memory pool with statically allocated storage.
    memory_pool() noexcept
        : m_list(&m_chunks[0], sizeof(chunk_type), TNumElem)
    {
    }

    memory_pool(const memory_pool&) = delete;
    memory_pool& operator= (const memory_pool&) = delete;

    //! Returns the number of pool elements.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const noexcept
    {
        return TNumElem;
    }

    //! Checks if the memory pool is empty.
    //! Returns \p true, if the memory pool is empty.
    bool empty() const noexcept
    {
        return m_list.empty();
    }

    //! Allocates a chunk from the pool.
    //! Allocates one chunk from the memory pool and returns a pointer to it.
    //! If the pool is already empty, a null-pointer is returned.
    //!
    //! \sa free()
    void* try_allocate() noexcept
    {
        return m_list.try_allocate();
    }

    //! Frees a previously allocated chunk.
    //! Returns a \p chunk which must have been allocated via allocate() back
    //! to the pool.
    //!
    //! \sa allocate()
    void free(void* chunk) noexcept
    {
        m_list.free(chunk);
    }

private:
    //! The memory chunks for the elements and the free-list pointers.
    chunk_type m_chunks[TNumElem];

    //! The free-list.
    weos_detail::FreeList m_list;
};

WEOS_END_NAMESPACE


#if defined(WEOS_WRAP_CMSIS_RTOS)
    #include "_cmsis_rtos/memorypool.hpp"
#else
    #error "Invalid native OS."
#endif

#endif // WEOS_MEMORYPOOL_HPP
