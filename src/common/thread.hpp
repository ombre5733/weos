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

#ifndef WEOS_COMMON_THREAD_HPP
#define WEOS_COMMON_THREAD_HPP

#include "thread_detail.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/decay.hpp>

namespace weos
{

//! A thread handle.
class thread
{
public:
    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    class id
    {
    public:
        id() BOOST_NOEXCEPT
            : m_id(0)
        {
        }

        explicit id(weos::detail::native_thread_traits::thread_id_type _id) BOOST_NOEXCEPT
            : m_id(_id)
        {
        }

    private:
        friend bool operator== (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator!= (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator< (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator<= (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator> (id lhs, id rhs) BOOST_NOEXCEPT;
        friend bool operator>= (id lhs, id rhs) BOOST_NOEXCEPT;

        weos::detail::native_thread_traits::thread_id_type m_id;
    };

    //! Thread attributes.
    class attributes
    {
    public:
        //! An enumeration of thread priorities.
        enum Priority
        {
            Idle = osPriorityIdle,
            Low = osPriorityLow,
            BelowNormal = osPriorityBelowNormal,
            Normal = osPriorityNormal,
            AboveNormal = osPriorityAboveNormal,
            High = osPriorityHigh,
            Realtime = osPriorityRealtime,
            Error = osPriorityError
        };

        //! Creates default thread attributes.
        attributes()
            : m_priority(Normal),
              m_customStackSize(0),
              m_customStack(0)
        {
        }

        //! Provides a custom stack.
        //! Makes the thread use the memory pointed to by \p stack whose size
        //! in bytes is passed in \p stackSize rather than the default stack.
        //!
        //! The default is a null-pointer for the stack and zero for its size.
        void setCustomStack(void* stack, std::uint32_t stackSize)
        {
            m_customStack = stack;
            m_customStackSize = stackSize;
        }

        //! Sets the priority.
        //! Sets the thread priority to \p priority.
        //!
        //! The default value is Priority::Normal.
        void setPriority(Priority priority)
        {
            m_priority = priority;
        }

    private:
        //! The thread's priority.
        Priority m_priority;
        //! The size of the custom stack.
        std::size_t m_customStackSize;
        //! A pointer to the custom stack.
        void* m_customStack;

        friend class thread;
    };

    //! Creates a thread handle without a thread.
    //! Creates a thread handle which is not associated with any thread. The
    //! new thread handle is not joinable.
    thread() BOOST_NOEXCEPT
        : m_data(0)
    {
    }

    template <typename F>
    thread(BOOST_FWD_REF(F) f,
           typename boost::enable_if_c<
               !boost::is_same<
                    typename boost::decay<F>::type, thread>::value
           >::type* dummy = 0)
        : m_data(detail::ThreadDataBase::allocate())
    {
        new (m_data) detail::ThreadData<
                typename boost::remove_reference<F>::type>(
                    boost::forward<F>(f));

        attributes attrs;
        invokeWithDefaultStack(attrs);
    }

    template <typename F,
              typename A0>
    thread(BOOST_FWD_REF(F) f,
           BOOST_FWD_REF(A0) a0,
           typename boost::enable_if_c<
               !boost::is_same<typename boost::remove_reference<F>::type,
                               attributes>::value>::type* dummy = 0)
        : m_data(detail::ThreadDataBase::allocate())
    {
        new (m_data) detail::ThreadData<
                typename boost::remove_reference<F>::type,
                A0>(
                    boost::forward<F>(f),
                    boost::forward<A0>(a0));

        attributes attrs;
        invokeWithDefaultStack(attrs);
    }

    template <typename F,
              typename A0,
              typename A1>
    thread(BOOST_FWD_REF(F) f,
           BOOST_FWD_REF(A0) a0,
           BOOST_FWD_REF(A1) a1)
        : m_data(detail::ThreadDataBase::allocate())
    {
        new (m_data) detail::ThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1>(
                    boost::forward<F>(f),
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1));

        attributes attrs;
        invokeWithDefaultStack(attrs);
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2>
    thread(BOOST_FWD_REF(F) f,
           BOOST_FWD_REF(A0) a0,
           BOOST_FWD_REF(A1) a1,
           BOOST_FWD_REF(A2) a2)
        : m_data(detail::ThreadDataBase::allocate())
    {
        new (m_data) detail::ThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1, A2>(
                    boost::forward<F>(f),
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1),
                    boost::forward<A2>(a2));

        attributes attrs;
        invokeWithDefaultStack(attrs);
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2,
              typename A3>
    thread(BOOST_FWD_REF(F) f,
           BOOST_FWD_REF(A0) a0,
           BOOST_FWD_REF(A1) a1,
           BOOST_FWD_REF(A2) a2,
           BOOST_FWD_REF(A3) a3)
        : m_data(detail::ThreadDataBase::allocate())
    {
        new (m_data) detail::ThreadData<
                typename boost::remove_reference<F>::type,
                A0, A1, A2, A3>(
                    boost::forward<F>(f),
                    boost::forward<A0>(a0),
                    boost::forward<A1>(a1),
                    boost::forward<A2>(a2),
                    boost::forward<A3>(a3));

        attributes attrs;
        invokeWithDefaultStack(attrs);
    }
# if 0
    //! Creates a thread.
    //! Starts the function \p fun with the argument \p arg in a new thread.
    //! The thread attributes are passed in \p attrs.
    thread(const attributes& attrs, void (*fun)(void*), void* arg)
        : m_data(0)
    {
        if (attrs.m_customStack || attrs.m_customStackSize)
            invokeWithCustomStack(attrs, fun, arg);
        else
            invokeWithDefaultStack(attrs);
    }
#endif
    //! Destroys the thread handle.
    //! Destroys this thread handle.
    //! \note If the thread handle is still associated with a joinable thread,
    //! its destruction will call std::terminate(). It is mandatory to either
    //! call join() or detach().
    ~thread()
    {
        if (joinable())
            std::terminate();
    }

    //! Separates the executing thread from this thread handle.
    void detach()
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }
        m_data->deref();
        m_data = 0;
    }

    //! Returns the id of the thread.
    id get_id() const BOOST_NOEXCEPT
    {
        if (m_data)
            return id(m_data->m_threadId);
        else
            return id();
    }

    //! Blocks until the associated thread has been finished.
    //! Blocks the calling thread until the thread which is associated with
    //! this thread handle has been finished.
    void join()
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }
        m_data->m_finished.wait();

        // The thread data is not needed any longer.
        m_data->deref();
        m_data = 0;
    }

    //! Checks if the thread is joinable.
    //! Returns \p true, if the thread is joinable.
    //! \note If a thread is joinable, either join() or detach() must be
    //! called before the destructor is executed.
    inline
    bool joinable() BOOST_NOEXCEPT
    {
        return m_data != 0;
    }

    //! Clears a set of signals.
    //! Clears the signals which are specified by the \p mask.
    inline
    void clear_signals(std::uint32_t mask)
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }

        detail::native_thread_traits::clear_signals(m_data->m_threadId, mask);
    }

    //! Sets a set of signals.
    //! Sets the signals which are specified by the \p mask.
    inline
    void set_signals(std::uint32_t mask)
    {
        if (!joinable())
        {
            ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }

        detail::native_thread_traits::set_signals(m_data->m_threadId, mask);
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    inline
    static unsigned hardware_concurrency() BOOST_NOEXCEPT
    {
        return 1;
    }

protected:
    //! Invokes the function \p fun with the argument \p arg. The thread
    //! attributes are passed in \p attrs. This method may only be called
    //! if \p attrs contains a valid custom stack configuration.
    void invokeWithCustomStack(const attributes& attrs,
                               void (*fun)(void*), void* arg);

    //! Invokes the function \p fun with the argument \p arg. The thread
    //! attributes are passed in \p attrs. This method may only be called
    //! if \p attrs does not contain a custom stack configuration.
    void invokeWithDefaultStack(const attributes& attrs);

private:
    //! The thread-data which is shared by this class and the invoker
    //! function.
    detail::ThreadDataBase* m_data;

    thread(const thread&);
    const thread& operator= (const thread&);
};

//! A thread with a custom stack.
//! The custom_stack_thread is a convenience class for creating a thread with a
//! custom stack. The memory for the stack is held statically by the object.
//! Its size in bytes is determined at compile-time by the template
//! parameter \p TStackSize.
template <std::size_t TStackSize>
class custom_stack_thread : public thread
{
    BOOST_STATIC_ASSERT(TStackSize >= weos::detail::native_thread_traits::minimum_custom_stack_size);

public:
    //! Creates a thread with a custom stack.
    //! Starts the function \p fun with the argument \p arg in a new thread.
    custom_stack_thread(void (*fun)(void*), void* arg)
    {
        thread::attributes attrs;
        attrs.setCustomStack(m_stack.address(), TStackSize);
        this->invokeWithCustomStack(attrs, fun, arg);
    }

    //! Creates a thread with a custom stack and a custom priority.
    //! Runs the function \p fun with the argument \p arg in a new thread,
    //! which has a custom stack and a user-defined priority of \p priority.
    custom_stack_thread(thread::attributes::Priority priority,
                        void (*fun)(void*), void* arg)
    {
        thread::attributes attrs;
        attrs.setCustomStack(m_stack.address(), TStackSize);
        attrs.setPriority(priority);
        this->invokeWithCustomStack(attrs, fun, arg);
    }

private:
    //! The custom stack.
    typename ::boost::aligned_storage<TStackSize>::type m_stack;
};

//! Compares two thread ids for equality.
//! Returns \p true, if \p lhs and \p rhs are equal.
inline
bool operator== (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id == rhs.m_id;
}

//! Compares two thread ids for inequality.
//! Returns \p true, if \p lhs and \p rhs are not equal.
inline
bool operator!= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id != rhs.m_id;
}

//! Less-than comparison for thread ids.
//! Returns \p true, if \p lhs is less than \p rhs.
inline
bool operator< (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id < rhs.m_id;
}

//! Less-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is less than or equal to \p rhs.
inline
bool operator<= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id <= rhs.m_id;
}

//! Greater-than comparison for thread ids.
//! Returns \p true, if \p lhs is greater than \p rhs.
inline
bool operator> (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id > rhs.m_id;
}

//! Greater-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is greater than or equal to \p rhs.
inline
bool operator>= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id >= rhs.m_id;
}

} // namespace weos

#endif // WEOS_COMMON_THREAD_HPP
