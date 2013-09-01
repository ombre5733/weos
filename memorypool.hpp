#ifndef WEOS_MEMORYPOOL_HPP
#define WEOS_MEMORYPOOL_HPP

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

private:
    //! Pointer to the first free block.
    void* m_first;

    static void*& next(void* const p)
    {
        return *static_cast<void**>(p);
    }
};

} // namespace detail

template <typename TElement, unsigned TNumElem>
class memory_pool
{
public:
    typedef TElement element_type;

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

private:
    typename ::boost::aligned_storage<block_size, min_align>::type m_data;
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
