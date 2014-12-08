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

#ifndef WEOS_COMMON_THREAD_DETAIL_HPP
#define WEOS_COMMON_THREAD_DETAIL_HPP

#include "../config.hpp"

#include "../atomic.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

namespace detail
{

// ----=====================================================================----
//     ThreadSharedData
// ----=====================================================================----

//! Data which is shared between the threaded function and the thread handle.
struct ThreadSharedData
{
    //! Creates the shared thread data with a reference count of 1.
    ThreadSharedData();

    //! Decreases the reference counter by one. If the reference counter reaches
    //! zero, this ThreadData is destructed and returned to the pool.
    void deref();

    //! Increases the reference counter by one.
    void ref();

    //! Allocates a ThreadData object from the global pool. An exception is
    //! thrown if the pool is empty.
    static ThreadSharedData* allocate();



    //! The bound function which will be called in the new thread.
    function<void()> m_threadedFunction;

    //! This semaphore is increased by the threaded function when it's
    //! execution finishes. It is needed to implement thread::join().
    semaphore m_finished;

    atomic_int m_referenceCount;

    //! This semaphore is increased by the thread creator when it has
    //! initialized the shared data, i.e. it is abused for sending a signal.
    semaphore m_initializationDone;

    //! The system-specific thread id.
    native_thread_traits::thread_id_type m_threadId;

private:
    ThreadSharedData(const ThreadSharedData&);
    const ThreadSharedData& operator= (const ThreadSharedData&);
};

} // namespace detail

//! A thread handle.
class thread : private detail::thread_base
{
    WEOS_MOVABLE_BUT_NOT_COPYABLE(thread)

    // An internal helper type for the enable_if machinery. Using a void pointer
    // is no good idea as it would match a user-supplied void pointer.
    struct _guard_type;

public:
    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that it is inexpensive to pass copies around.
    typedef detail::native_thread_traits::id id;

    //! The thread attributes.
    typedef detail::native_thread_traits::attributes attributes;


    //! Creates a thread handle without a thread.
    //! Creates a thread handle which is not associated with any thread. The
    //! new thread handle is not joinable.
    thread() WEOS_NOEXCEPT
        : m_data(0)
    {
    }

    // -------------------------------------------------------------------------
    // Constructors without attributes
    // -------------------------------------------------------------------------

    template <typename F>
    explicit
    thread(WEOS_FWD_REF(F) f,
           typename enable_if<
               !is_same<typename decay<F>::type, thread>::value,
               _guard_type
           >::type* dummy = 0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f));

        invoke(attributes());
    }

    template <typename F,
              typename A0>
    thread(WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           typename enable_if<
               !is_same<typename decay<F>::type, attributes>::value,
               _guard_type
           >::type* dummy = 0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0));

        invoke(attributes());
    }

    template <typename F,
              typename A0,
              typename A1>
    thread(WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1,
           typename enable_if<
               !is_same<typename decay<F>::type, attributes>::value,
               _guard_type
           >::type* dummy = 0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1));

        invoke(attributes());
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2>
    thread(WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1,
           WEOS_FWD_REF(A2) a2,
           typename enable_if<
               !is_same<typename decay<F>::type, attributes>::value,
               _guard_type
           >::type* dummy = 0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1),
                                                forward<A2>(a2));

        invoke(attributes());
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2,
              typename A3>
    thread(WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1,
           WEOS_FWD_REF(A2) a2,
           WEOS_FWD_REF(A3) a3,
           typename enable_if<
               !is_same<typename decay<F>::type, attributes>::value,
               _guard_type
           >::type* dummy = 0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1),
                                                forward<A2>(a2),
                                                forward<A3>(a3));

        invoke(attributes());
    }

    // -------------------------------------------------------------------------
    // Constructors with attributes
    // -------------------------------------------------------------------------

    template <typename F>
    thread(const attributes& attrs,
           WEOS_FWD_REF(F) f)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f));

        invoke(attrs);
    }

    template <typename F,
              typename A0>
    thread(const attributes& attrs,
           WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0));

        invoke(attrs);
    }

    template <typename F,
              typename A0,
              typename A1>
    thread(const attributes& attrs,
           WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1));

        invoke(attrs);
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2>
    thread(const attributes& attrs,
           WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1,
           WEOS_FWD_REF(A2) a2)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1),
                                                forward<A2>(a2));

        invoke(attrs);
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2,
              typename A3>
    thread(const attributes& attrs,
           WEOS_FWD_REF(F) f,
           WEOS_FWD_REF(A0) a0,
           WEOS_FWD_REF(A1) a1,
           WEOS_FWD_REF(A2) a2,
           WEOS_FWD_REF(A3) a3)
        : m_data(detail::ThreadSharedData::allocate())
    {
        m_data->m_threadedFunction = bind<void>(forward<F>(f),
                                                forward<A0>(a0),
                                                forward<A1>(a1),
                                                forward<A2>(a2),
                                                forward<A3>(a3));

        invoke(attrs);
    }

    //! Move constructor.
    //! Constructs a thread by moving from the \p other thread.
    thread(WEOS_RV_REF(thread) other)
        : m_data(other.m_data)
    {
        other.m_data = 0;
    }

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

    //! Move assignment.
    //! Move-assigns the \p other thread to this thread.
    thread& operator= (WEOS_RV_REF(thread) other)
    {
        if (this != &other)
        {
            m_data = other.m_data;
            other.m_data = 0;
        }
        return *this;
    }

    //! Separates the executing thread from this thread handle.
    void detach()
    {
        if (!joinable())
            WEOS_THROW_SYSTEM_ERROR(errc::operation_not_permitted,
                                    "thread::detach: thread is not joinable");

        m_data->deref();
        m_data = 0;
    }

    //! Returns the id of the thread.
    id get_id() const WEOS_NOEXCEPT
    {
        if (m_data)
            return id(m_data->m_threadId);
        else
            return id();
    }

    //! Blocks until the associated thread has been finished.
    //! Blocks the calling thread until the thread which is associated with
    //! this thread handle has been finished.
    void join();

    //! Checks if the thread is joinable.
    //! Returns \p true, if the thread is joinable.
    //! \note If a thread is joinable, either join() or detach() must be
    //! called before the destructor is executed.
    inline
    bool joinable() WEOS_NOEXCEPT
    {
        return m_data != 0;
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    inline
    static unsigned hardware_concurrency() WEOS_NOEXCEPT
    {
        return 1;
    }

    // -------------------------------------------------------------------------
    // Signal management
    // -------------------------------------------------------------------------

    //! Represents a set of signal flags.
    typedef detail::native_thread_traits::signal_set signal_set;

    //! Returns the number of signals in a set.
    inline
    static int signals_count() WEOS_NOEXCEPT
    {
        return detail::native_thread_traits::signals_count;
    }

    //! Returns a signal set with all flags being set.
    inline
    static signal_set all_signals() WEOS_NOEXCEPT
    {
        return detail::native_thread_traits::all_signals;
    }

    //! Clears a set of signals.
    //! Clears the signals which are specified by the \p flags.
    void clear_signals(signal_set flags);

    //! Sets a set of signals.
    //! Sets the signals which are specified by the \p flags.
    void set_signals(signal_set flags);

protected:
    //! Invokes the function which is stored in the shared data in a new
    //! thread which is created with the attributes \p attrs.
    void invoke(const attributes& attrs);

private:
    //! The thread-data which is shared by this class and the invoker
    //! function.
    detail::ThreadSharedData* m_data;
};

//! Compares two thread ids for equality.
//! Returns \p true, if \p lhs and \p rhs are equal.
inline
bool operator== (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id == rhs.m_id;
}

//! Compares two thread ids for inequality.
//! Returns \p true, if \p lhs and \p rhs are not equal.
inline
bool operator!= (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id != rhs.m_id;
}

//! Less-than comparison for thread ids.
//! Returns \p true, if \p lhs is less than \p rhs.
inline
bool operator< (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id < rhs.m_id;
}

//! Less-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is less than or equal to \p rhs.
inline
bool operator<= (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id <= rhs.m_id;
}

//! Greater-than comparison for thread ids.
//! Returns \p true, if \p lhs is greater than \p rhs.
inline
bool operator> (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id > rhs.m_id;
}

//! Greater-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is greater than or equal to \p rhs.
inline
bool operator>= (thread::id lhs, thread::id rhs) WEOS_NOEXCEPT
{
    return lhs.m_id >= rhs.m_id;
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_THREAD_DETAIL_HPP
