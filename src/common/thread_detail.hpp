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

#ifndef WEOS_COMMON_THREAD_DETAIL_HPP
#define WEOS_COMMON_THREAD_DETAIL_HPP

#include "../memorypool.hpp"

#include <boost/move/move.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace weos
{
namespace detail
{

// ----=====================================================================----
//     invokeCallable
// ----=====================================================================----

// --------------------------------------------------------------------
//     Case 1: Member function pointer together with object
// --------------------------------------------------------------------
template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(),
               BOOST_FWD_REF(A0) a0)
{
    (boost::forward<A0>(a0).*f)(
        );
}

template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)() const,
               BOOST_FWD_REF(A0) a0)
{
    (boost::forward<A0>(a0).*f)(
        );
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    (boost::forward<A0>(a0).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

// --------------------------------------------------------------------
//     Case 2: Member function pointer together with pointer
// --------------------------------------------------------------------
template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(),
               BOOST_FWD_REF(A0) a0)
{
    ((*boost::forward<A0>(a0)).*f)(
        );
}

template <typename Return, typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)() const,
               BOOST_FWD_REF(A0) a0)
{
    ((*boost::forward<A0>(a0)).*f)(
        );
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3),
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

template <typename Return, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type
invokeCallable(Return (F::*f)(A1, A2, A3) const,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    ((*boost::forward<A0>(a0)).*f)(
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}

// --------------------------------------------------------------------
//     Case 3: Function pointer
// --------------------------------------------------------------------
template <typename F>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f)
{
    boost::forward<F>(f)(
        );
}

template <typename F,
          typename A0>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0));
}

template <typename F,
          typename A0,
          typename A1>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1));
}

template <typename F,
          typename A0,
          typename A1,
          typename A2>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1),
        boost::forward<A2>(a2));
}

template <typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename boost::enable_if_c<
    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type
invokeCallable(BOOST_FWD_REF(F) f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
{
    boost::forward<F>(f)(
        boost::forward<A0>(a0),
        boost::forward<A1>(a1),
        boost::forward<A2>(a2),
        boost::forward<A3>(a3));
}



// ----=====================================================================----
//     ThreadData
// ----=====================================================================----

//! Data which is shared between the threaded function and the thread handle.
class ThreadDataBase
{
public:
    //! Creates the shared thread data with a reference count of 1.
    ThreadDataBase();
    //! Destroys the shared data.
    virtual ~ThreadDataBase() {}

    //! Decreases the reference counter by one. If the reference counter reaches
    //! zero, this ThreadData is returned to the pool.
    void deref();
    //! Increases the reference counter by one.
    void ref();

    //! Invokes the callable in the new thread.
    virtual void invoke() = 0;

    //! Allocates a ThreadData object from the global pool and throws and
    //! exception if none is available.
    static ThreadDataBase* allocate();

    //! This semaphore is increased by the threaded function when it's
    //! execution finishes. It is needed to implement thread::join().
    semaphore m_finished;

    struct atomic_int
    {
        mutex mtx;
        int value;
    };
    atomic_int m_referenceCount;


    //! The system-specific thread id.
    weos::detail::native_thread_traits::thread_id_type m_threadId;

private:
    ThreadDataBase(const ThreadDataBase&);
    const ThreadDataBase& operator= (const ThreadDataBase&);
};

// The largest possible ThreadData object.
struct LargestThreadData : public ThreadDataBase
{
    virtual void invoke() {}
    boost::aligned_storage<WEOS_MAX_THREAD_ARGUMENT_SIZE>::type m_data;
};

struct null_type {};

template <typename F,
          typename A0 = null_type, typename A1 = null_type,
          typename A2 = null_type, typename A3 = null_type>
struct ThreadData : public ThreadDataBase
{
    ThreadData(F f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2,
               BOOST_FWD_REF(A3) a3)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1)),
          m_a2(boost::forward<A2>(a2)),
          m_a3(boost::forward<A3>(a3))
    {
        BOOST_STATIC_ASSERT(sizeof(ThreadData) <= sizeof(LargestThreadData));
        BOOST_STATIC_ASSERT(boost::alignment_of<ThreadData>::value
                            <= boost::alignment_of<LargestThreadData>::value);
    }

    virtual void invoke()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1),
                       boost::move(m_a2),
                       boost::move(m_a3));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
    typename boost::decay<A2>::type m_a2;
    typename boost::decay<A3>::type m_a3;
};

template <typename F, typename A0, typename A1, typename A2>
struct ThreadData<F, A0, A1, A2, null_type>
        : public ThreadDataBase
{
    ThreadData(F f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1)),
          m_a2(boost::forward<A2>(a2))
    {
        BOOST_STATIC_ASSERT(sizeof(ThreadData) <= sizeof(LargestThreadData));
        BOOST_STATIC_ASSERT(boost::alignment_of<ThreadData>::value
                            <= boost::alignment_of<LargestThreadData>::value);
    }

    virtual void invoke()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1),
                       boost::move(m_a2));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
    typename boost::decay<A2>::type m_a2;
};

template <typename F, typename A0, typename A1>
struct ThreadData<F, A0, A1, null_type, null_type>
        : public ThreadDataBase
{
    ThreadData(F f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1))
    {
        BOOST_STATIC_ASSERT(sizeof(ThreadData) <= sizeof(LargestThreadData));
        BOOST_STATIC_ASSERT(boost::alignment_of<ThreadData>::value
                            <= boost::alignment_of<LargestThreadData>::value);
    }

    virtual void invoke()
    {
        invokeCallable(m_f,
                       boost::move(m_a0),
                       boost::move(m_a1));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
};

template <typename F, typename A0>
struct ThreadData<F, A0, null_type, null_type, null_type>
        : public ThreadDataBase
{
    ThreadData(F f,
               BOOST_FWD_REF(A0) a0)
        : m_f(f),
          m_a0(boost::forward<A0>(a0))
    {
        BOOST_STATIC_ASSERT(sizeof(ThreadData) <= sizeof(LargestThreadData));
        BOOST_STATIC_ASSERT(boost::alignment_of<ThreadData>::value
                            <= boost::alignment_of<LargestThreadData>::value);
    }

    virtual void invoke()
    {
        invokeCallable(m_f,
                       boost::move(m_a0));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
};

template <typename F>
struct ThreadData<F, null_type, null_type, null_type, null_type>
        : public ThreadDataBase
{
    explicit ThreadData(BOOST_FWD_REF(F) f)
        : m_f(boost::forward<F>(f))
    {
        BOOST_STATIC_ASSERT(sizeof(ThreadData) <= sizeof(LargestThreadData));
        BOOST_STATIC_ASSERT(boost::alignment_of<ThreadData>::value
                            <= boost::alignment_of<LargestThreadData>::value);
    }

    virtual void invoke()
    {
        m_f();
    }

    typename boost::decay<F>::type m_f;
};

} // namespace detail
} // namespace weos

#endif // WEOS_COMMON_THREAD_DETAIL_HPP
