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

#ifndef WEOS_CXX11_MEMORYPOOL_HPP
#define WEOS_CXX11_MEMORYPOOL_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../memorypool.hpp"


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

    //! Constructs a shared memory pool.
    shared_memory_pool()
        : m_numElements(TNumElem)
    {
    }

    //! Returns the number of pool elements.
    //! Returns the number of elements for which the pool provides memory.
    std::size_t capacity() const noexcept
    {
        return TNumElem;
    }

    //! Checks if the memory pool is empty.
    //! Returns \p true, if the memory pool is empty.
    bool empty() const
    {
        lock_guard<mutex> lock(m_mutex);
        return m_memoryPool.empty();
    }

    //! Allocates a chunk of memory.
    //! Allocates a chunk of memory and returns a pointer to it. The calling
    //! thread is blocked until a chunk is available.
    //!
    //! \sa free(), try_allocate(), try_allocate_for()
    void* allocate()
    {
        m_numElements.wait();
        lock_guard<mutex> lock(m_mutex);
        void* element = m_memoryPool.try_allocate();
        WEOS_ASSERT(element);
        return element;
    }

    //! Tries to allocate a chunk of memory.
    //! Tries to allocate a chunk of memory and returns a pointer to it. If
    //! no memory is available, a null-pointer is returned.
    //!
    //! \sa allocate(), free(), try_allocate_for()
    void* try_allocate()
    {
        if (m_numElements.try_wait())
        {
            lock_guard<mutex> lock(m_mutex);
            void* element = m_memoryPool.try_allocate();
            WEOS_ASSERT(element);
            return element;
        }
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
        {
            lock_guard<mutex> lock(m_mutex);
            void* element = m_memoryPool.try_allocate();
            WEOS_ASSERT(element);
            return element;
        }
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
        lock_guard<mutex> lock(m_mutex);
        m_memoryPool.free(chunk);
        m_numElements.post();
    }

private:
    typedef memory_pool<TElement, TNumElem> pool_t;
    //! The pool from which the memory for the element is allocated.
    pool_t m_memoryPool;
    //! A mutex to protect the pool.
    mutable mutex m_mutex;
    //! The number of available elements.
    semaphore m_numElements;
};

WEOS_END_NAMESPACE

#endif // WEOS_CXX11_MEMORYPOOL_HPP
