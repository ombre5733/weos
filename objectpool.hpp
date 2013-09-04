#ifndef WEOS_OBJECTPOOL_HPP
#define WEOS_OBJECTPOOL_HPP

#include "memorypool.hpp"

namespace weos
{

//! An object pool with static (compile-time) storage.
//! The StaticObjectPool is a memory pool for up to \p TNumElem objects of
//! type \p TType. It does not allocate memory from the heap but uses an
//! internal array.
template <typename TType, unsigned TNumElem, typename TMutex = null_mutex>
class object_pool
{
public:
    typedef TType element_type;
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
    //! constructor using a placement-new. The method returns a pointer to
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

} // namespace weos

#endif // WEOS_OBJECTPOOL_HPP
