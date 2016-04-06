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

#ifndef WEOS_CMSIS_RTOS_MEMORYPOOL_HPP
#define WEOS_CMSIS_RTOS_MEMORYPOOL_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "_core.hpp"

#include "../memorypool.hpp"
#include "../atomic.hpp"


WEOS_BEGIN_NAMESPACE

//! A shared memory pool.
//! A shared_memory_pool is a thread-safe alternative to the memory_pool.
//! Like its non-threaded counterpart, it holds the memory for up to
//! (\p TNumElem) elements of type \p TElement internally and does not
//! allocate them on the heap.
template <typename TElement, std::size_t TNumElem>
class shared_memory_pool
{
public:
    //! The type of the elements in this pool.
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

    // The control block of a memory box. Defined as OS_BM in
    // ${CMSIS-RTOS}/SRC/rt_TypeDef.h.
    static_assert(osCMSIS_RTX <= ((4<<16) | 78), "Check the layout of OS_BM.");
    struct ControlBlock
    {
        void* free;
        void* end;
        std::uint32_t chunkSize;
    };

public:
    //! Constructs a shared memory pool.
    shared_memory_pool() noexcept
    {
        m_controlBlock.free = weos_detail::FreeList(
                                  &m_chunks[0], sizeof(chunk_type), TNumElem).first();
        m_controlBlock.end = &m_chunks[TNumElem];
        m_controlBlock.chunkSize = sizeof(chunk_type);
    }

    shared_memory_pool(const shared_memory_pool&) = delete;
    shared_memory_pool& operator=(const shared_memory_pool&) = delete;

    //! Returns the number of pool elements.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const noexcept
    {
        return TNumElem;
    }

    //! Checks if the memory pool is empty.
    //!
    //! Returns \p true, if the memory pool is empty.
    bool empty() const noexcept
    {
        // TODO: what memory order to use here?
        atomic_thread_fence(memory_order_seq_cst);
        return m_controlBlock.free == 0;
    }

    //! Allocates a chunk from the pool.
    //! Allocates one chunk from the memory pool and returns a pointer to it.
    //! If the pool is already empty, a null-pointer is returned.
    //!
    //! \note This method may be called in an interrupt context.
    //!
    //! \sa free()
    void* try_allocate() noexcept
    {
        return osPoolAlloc(static_cast<osPoolId>(
                               static_cast<void*>(&m_controlBlock)));
    }

    //! Frees a chunk of memory.
    //! Frees a \p chunk of memory which must have been allocated through
    //! this pool.
    //!
    //! \note This method may be called in an interrupt context.
    //!
    //! \sa try_allocate()
    void free(void* chunk) noexcept
    {
        osStatus ret = osPoolFree(static_cast<osPoolId>(
                                      static_cast<void*>(&m_controlBlock)),
                                  chunk);
        WEOS_ASSERT(ret == osOK);
    }

private:
    //! The pool's control block. Note: It is important that the control
    //! block is placed before the chunk array. osPoolFree() makes a boundary
    //! check of the chunk to be freed, which involves the control block.
    ControlBlock m_controlBlock;
    //! The memory chunks for the elements and the free-list pointers.
    chunk_type m_chunks[TNumElem];
};

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_MEMORYPOOL_HPP
