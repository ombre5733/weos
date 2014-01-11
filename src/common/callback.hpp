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

#ifndef WEOS_COMMON_CALLBACK_HPP
#define WEOS_COMMON_CALLBACK_HPP

#include "../config.hpp"
#include "invokecallable.hpp"

#include <boost/move/move.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/decay.hpp>

namespace weos
{
namespace detail
{

// A null-type which acts as sentinel in the simulated variadic templates.
struct null_type {};

} // namespace detail

// ----=====================================================================----
//     callback
// ----=====================================================================----

//! The base class for callbacks.
//! The callback_base is the base class from which all callbacks derive.
class callback_base
{
public:
    virtual ~callback_base() {}
    virtual void operator() () = 0;
};

//! A callback.
//! A callback is a function pointer or a pointer to a member function whose
//! arguments are bound.
template <typename F,
          typename A0 = detail::null_type,
          typename A1 = detail::null_type,
          typename A2 = detail::null_type,
          typename A3 = detail::null_type>
struct callback : public callback_base
{
    callback(F f,
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
    }

    virtual void operator() ()
    {
        detail::invokeCallable(m_f,
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

template <typename F,
          typename A0,
          typename A1,
          typename A2>
struct callback<F, A0, A1, A2, detail::null_type>
        : public callback_base
{
    callback(F f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1,
               BOOST_FWD_REF(A2) a2)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1)),
          m_a2(boost::forward<A2>(a2))
    {
    }

    virtual void operator() ()
    {
        detail::invokeCallable(m_f,
                               boost::move(m_a0),
                               boost::move(m_a1),
                               boost::move(m_a2));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
    typename boost::decay<A2>::type m_a2;
};

template <typename F,
          typename A0,
          typename A1>
struct callback<F, A0, A1, detail::null_type, detail::null_type>
        : public callback_base
{
    callback(F f,
               BOOST_FWD_REF(A0) a0,
               BOOST_FWD_REF(A1) a1)
        : m_f(f),
          m_a0(boost::forward<A0>(a0)),
          m_a1(boost::forward<A1>(a1))
    {
    }

    virtual void operator() ()
    {
        detail::invokeCallable(m_f,
                               boost::move(m_a0),
                               boost::move(m_a1));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
    typename boost::decay<A1>::type m_a1;
};

template <typename F,
          typename A0>
struct callback<F, A0, detail::null_type, detail::null_type, detail::null_type>
        : public callback_base
{
    callback(F f,
               BOOST_FWD_REF(A0) a0)
        : m_f(f),
          m_a0(boost::forward<A0>(a0))
    {
    }

    virtual void operator() ()
    {
        detail::invokeCallable(m_f,
                               boost::move(m_a0));
    }

    typename boost::decay<F>::type m_f;
    typename boost::decay<A0>::type m_a0;
};

template <typename F>
struct callback<F, detail::null_type, detail::null_type, detail::null_type,
                detail::null_type>
        : public callback_base
{
    explicit callback(BOOST_FWD_REF(F) f)
        : m_f(boost::forward<F>(f))
    {
    }

    virtual void operator() ()
    {
        m_f();
    }

    typename boost::decay<F>::type m_f;
};

// ----=====================================================================----
//     callback_wrapper
// ----=====================================================================----

namespace detail
{

// A helper type which derives from callback_base and includes an additional
// storage of SizeT bytes with maximum alignment.
template <std::size_t SizeT>
struct callback_size_helper : public callback_base
{
    virtual void operator() () {}
    typename boost::aligned_storage<SizeT>::type m_storage;
};

} // namespace detail

//! A wrapper for a callback.
//! The callback_wrapper wraps a callback. Its sole purpose is to hide the
//! concrete type of the callback, which is tedious to write if the compiler
//! does not support the automatic deduction of a variable's type (i.e. it
//! does not support the \p auto keyword).
//!
//! A callback_wrapper is similar to a <tt>std::function<void()></tt>. However,
//! it never allocates memory from the heap but has an internal storage large
//! enough to store a callback whose arguments fit into \p SizeT bytes.
template <std::size_t SizeT>
struct callback_wrapper
{
private:
    typedef typename boost::aligned_storage<
        sizeof(detail::callback_size_helper<SizeT>)>::type storage_type;

public:
    callback_wrapper()
        : m_callback(0)
    {
    }

    ~callback_wrapper()
    {
        reset();
    }

    inline
    void operator() ()
    {
        if (m_callback)
            (*m_callback)();
        else
        {
            //! \todo ::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }
    }

    void reset()
    {
        if (m_callback)
        {
            m_callback->~callback_base();
            m_callback = 0;
        }
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2,
              typename A3>
    inline
    callback<F, A0, A1, A2, A3>* emplace(F f,
                                         BOOST_FWD_REF(A0) a0,
                                         BOOST_FWD_REF(A1) a1,
                                         BOOST_FWD_REF(A2) a2,
                                         BOOST_FWD_REF(A3) a3)
    {
        typedef callback<F, A0, A1, A2, A3> callback_type;

        BOOST_STATIC_ASSERT(sizeof(callback_type) <= sizeof(storage_type));
        BOOST_STATIC_ASSERT(boost::alignment_of<callback_type>::value
                            <= boost::alignment_of<storage_type>::value);

        reset();
        callback_type* temp = new (m_storage.address()) callback_type(
                                  f,
                                  boost::forward<A0>(a0),
                                  boost::forward<A1>(a1),
                                  boost::forward<A2>(a2),
                                  boost::forward<A3>(a3));
        m_callback = temp;
        return temp;
    }

    template <typename F,
              typename A0,
              typename A1,
              typename A2>
    inline
    callback<F, A0, A1, A2>* emplace(F f,
                                     BOOST_FWD_REF(A0) a0,
                                     BOOST_FWD_REF(A1) a1,
                                     BOOST_FWD_REF(A2) a2)
    {
        typedef callback<F, A0, A1, A2> callback_type;

        BOOST_STATIC_ASSERT(sizeof(callback_type) <= sizeof(storage_type));
        BOOST_STATIC_ASSERT(boost::alignment_of<callback_type>::value
                            <= boost::alignment_of<storage_type>::value);

        reset();
        callback_type* temp = new (m_storage.address()) callback_type(
                                  f,
                                  boost::forward<A0>(a0),
                                  boost::forward<A1>(a1),
                                  boost::forward<A2>(a2));
        m_callback = temp;
        return temp;
    }

    template <typename F,
              typename A0,
              typename A1>
    inline
    callback<F, A0, A1>* emplace(F f,
                                 BOOST_FWD_REF(A0) a0,
                                 BOOST_FWD_REF(A1) a1)
    {
        typedef callback<F, A0, A1> callback_type;

        BOOST_STATIC_ASSERT(sizeof(callback_type) <= sizeof(storage_type));
        BOOST_STATIC_ASSERT(boost::alignment_of<callback_type>::value
                            <= boost::alignment_of<storage_type>::value);

        reset();
        callback_type* temp = new (m_storage.address()) callback_type(
                                  f,
                                  boost::forward<A0>(a0),
                                  boost::forward<A1>(a1));
        m_callback = temp;
        return temp;
    }

    template <typename F,
              typename A0>
    inline
    callback<F, A0>* emplace(F f,
                             BOOST_FWD_REF(A0) a0)
    {
        typedef callback<F, A0> callback_type;

        BOOST_STATIC_ASSERT(sizeof(callback_type) <= sizeof(storage_type));
        BOOST_STATIC_ASSERT(boost::alignment_of<callback_type>::value
                            <= boost::alignment_of<storage_type>::value);

        reset();
        callback_type* temp = new (m_storage.address()) callback_type(
                                  f,
                                  boost::forward<A0>(a0));
        m_callback = temp;
        return temp;
    }

    template <typename F>
    inline
    callback<F>* emplace(BOOST_FWD_REF(F) f)
    {
        typedef callback<F> callback_type;

        BOOST_STATIC_ASSERT(sizeof(callback_type) <= sizeof(storage_type));
        BOOST_STATIC_ASSERT(boost::alignment_of<callback_type>::value
                            <= boost::alignment_of<storage_type>::value);

        reset();
        callback_type* temp = new (m_storage.address()) callback_type(
                                  boost::forward<F>(f));
        m_callback = temp;
        return temp;
    }

private:
    callback_base* m_callback;
    storage_type m_storage;
};

//! \todo make_callback is missing
/*
template <typename F, typename ...Args>
callback<F, Args...> make_callback(F&& f, Args&&... args)
{
    return callback<F, Args...>(std::forward<F>(f),
                                std::forward<Args>(args)...);
}
*/

} // namespace weos

#endif // WEOS_COMMON_CALLBACK_HPP
