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

#include <boost/integer/static_min_max.hpp>
#include <boost/math/common_factor_ct.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility.hpp>

namespace weos
{

namespace detail
{

class free_list : boost::noncopyable
{
public:
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

        char* iter = memBlock;
        while (iter != last)
        {
            char* follow = iter + chunkSize;
            next(iter) = follow;
            iter = follow;
        }
    }

    bool empty() const BOOST_NOEXCEPT
    {
        return m_first == 0;
    }

    void* allocate()
    {
        void* chunk = m_first;
        m_first = next(m_first);
        return chunk;
    }

    void free(void* const chunk)
    {
        next(chunk) = m_first;
        m_first = chunk;
    }

private:
    //! Pointer to the first free block.
    void* m_first;

    static void*& next(void* const p)
    {
        return *static_cast<void**>(p);
    }
};

} // namespace detail

template <typename TElement, unsigned TNumElem, typename TMutex = null_mutex>
class memory_pool
#ifndef WEOS_DOXYGEN_RUN
        : public TMutex
#endif
{
public:
    typedef TElement element_type;
    typedef TMutex mutex_type;

private:
    // A chunk has to be aligned such that it can contain a void* or an element.
    static const std::size_t min_align =
        ::boost::math::static_lcm< ::boost::alignment_of<void*>::value,
                                   ::boost::alignment_of<element_type>::value>::value;
    // The chunk size has to be large enough to sotre a void* or an element.
    // Further it must be a multiple of the alignment.
    static const std::size_t chunk_size =
        ::boost::math::static_lcm<
            ::boost::static_unsigned_max<sizeof(void*), sizeof(element_type)>::value,
            min_align>::value;
    // The memory block must be able to hold TNumElem elements.
    static const std::size_t block_size = chunk_size * TNumElem;

public:
    memory_pool()
        : m_freeList(m_data.address(), chunk_size, block_size)
    {
    }

    bool empty() const
    {
        unique_lock<mutex_type> lock(*this);
        return m_freeList.empty();
    }

    void* allocate()
    {
        unique_lock<mutex_type> lock(*this);
        if (empty())
            return 0;
        else
            return m_freeList.allocate();
    }

    void* try_allocate()
    {
    }

    template <typename RepT, typename PeriodT>
    void* try_allocate_for(const chrono::duration<RepT, PeriodT>& d)
    {
    }

    void free(void* const chunk)
    {
        unique_lock<mutex_type> lock(*this);
        m_freeList.free(chunk);
    }

private:
    typename ::boost::aligned_storage<block_size, min_align>::type m_data;
    detail::free_list m_freeList;
};

} // namespace weos

#endif // WEOS_MEMORYPOOL_HPP




//! An object pool with static (compile-time) storage.
//! The StaticObjectPool is a memory pool for up to \p TNumElem objects of
//! type \p TType. It does not allocate memory from the heap but uses an
//! internal array.
template <typename TType, unsigned TNumElem>
class StaticObjectPool : protected boost::simple_segregated_storage<std::size_t>
{
public:
    typedef TType element_type;

private:
    typedef std::size_t size_type;

    // The chunk size has to be a multiple of sizeof(void*) and big enough
    // to hold an element.
    BOOST_STATIC_CONSTANT(size_type, chunk_size =
        (::boost::math::static_lcm<sizeof(void*),
                                   sizeof(element_type)>::value));
    // The memory block must be able to hold TNumElem elements.
    BOOST_STATIC_CONSTANT(size_type, block_size =
        (chunk_size * TNumElem));
    // For the simple_segregated_storage we need to have a chunk aligned such
    // that it can be cast to a void*. When returning a chunk-pointer, to
    // the user, the chunk needs to have the alignment of the element_type.
    BOOST_STATIC_CONSTANT(size_type, min_align =
        (::boost::math::static_lcm< ::boost::alignment_of<void*>::value,
                                    ::boost::alignment_of<element_type>::value>::value));

    typedef boost::simple_segregated_storage<std::size_t> storage_t;
    storage_t& storage() { return *this; }
    const storage_t& storage() const { return *this; }

public:
    //! Creates an object pool.
    StaticObjectPool()
    {
        // simple_segregated_storage requires that
        // - chunk_size >= sizeof(void*)
        // - chunk_size = sizeof(void*) * i, for some integer i
        // - block_size >= chunk_size
        // - Block is properly aligned for an array of object of
        //   size chunk_size and array of void*
        storage().add_block(&m_data, sizeof(m_data), chunk_size);
    }

    ~StaticObjectPool()
    {
        //! \todo 1. order the free list (insert an order() function into
        //!          the storage
        //!       2. traverse the ordered free list - if the 'next'-pointer
        //!          does not point to the following chunk, the chunk is
        //!          occupied by a live object whose destructor needs to be
        //!          called
    }

    //! Checks if the pool is empty.
    //! Returns \p true, if the pool is empty and no more object can be
    //! allocated.
    bool empty() const
    {
        //! \todo: osl::lock_guard<osl::mutex> locker(&m_mutex);
        return storage().empty();
    }

    //! Allocates memory for an object.
    //! Allocates memory for one object and returns a pointer to the allocated
    //! space. The object is not initialized, i.e. the caller has to invoke the
    //! constructor using a placement-new. The method returns a pointer to
    //! the allocated memory or a null-pointer if no more memory was available.
    element_type* malloc()
    {
        if (empty())
            return 0;
        else
            return static_cast<element_type*>(storage().malloc());
    }

    //! Frees a previously allocated storage.
    //! Frees the memory space \p element which has been previously allocated
    //! via this object pool. This function does not destroy the element and
    //! the caller is responsible for calling the destructor, thus.
    void free(element_type* const element)
    {
        storage().free(element);
    }

    //! Allocates and constructs an object.
    //! Allocates memory for an object and calls its constructor. The method
    //! returns a pointer to the newly created object or a null-pointer if no
    //! memory was available.
    element_type* construct()
    {
        void* mem = this->malloc();
        element_type* element = new (mem) element_type;
        return element;
    }

    template <class T1>
    element_type* construct(BOOST_FWD_REF(T1) x1)
    {
        void* mem = this->malloc();
        element_type* element = new (mem) element_type(
                                    boost::forward<T1>(x1));
        return element;
    }

    template <class T1, class T2>
    element_type* construct(BOOST_FWD_REF(T1) x1, BOOST_FWD_REF(T2) x2)
    {
        void* mem = this->malloc();
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
        storage().free(element);
    }

private:
    typename ::boost::aligned_storage<block_size, min_align>::type m_data;
};
