
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

#ifndef WEOS_COMMON_FUNCTIONAL_HPP
#define WEOS_COMMON_FUNCTIONAL_HPP

#include "../config.hpp"

#include "../utility.hpp"

#include <boost/utility/enable_if.hpp>
#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>

#include <cstdlib>

WEOS_BEGIN_NAMESPACE

namespace placeholders
{

template <int TIndex>
struct placeholder
{
};

extern const placeholder<1> _1;
extern const placeholder<2> _2;
extern const placeholder<3> _3;
extern const placeholder<4> _4;
} // namespace placeholders

template <typename T>
struct is_placeholder : boost::integral_constant<int, 0>
{
};

template <int TIndex>
struct is_placeholder<placeholders::placeholder<TIndex> >
        : boost::integral_constant<int, TIndex>
{
};

namespace detail
{

struct unspecified_type {};

template <typename TResult, typename TF>
struct result_traits
{
    typedef TResult type;
};

template <typename TF>
struct result_traits<unspecified_type, TF>
{
    typedef typename TF::result_type type;
};

// ====================================================================
// argument_tuple<>
// ====================================================================

struct argument_tuple_null_type;

template <typename A0 = argument_tuple_null_type,
          typename A1 = argument_tuple_null_type,
          typename A2 = argument_tuple_null_type,
          typename A3 = argument_tuple_null_type>
struct argument_tuple
{
    static const std::size_t size = 4;

    explicit argument_tuple(const A0& a0,
                            const A1& a1,
                            const A2& a2,
                            const A3& a3)
        : m_a0(a0),
          m_a1(a1),
          m_a2(a2),
          m_a3(a3)
    {
    }

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    explicit argument_tuple(BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
        : m_a0(boost::forward<T0>(t0)),
          m_a1(boost::forward<T1>(t1)),
          m_a2(boost::forward<T2>(t2)),
          m_a3(boost::forward<T3>(t3))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0),
          m_a1(other.m_a1),
          m_a2(other.m_a2),
          m_a3(other.m_a3)
    {
    }

    // Move constructor
    argument_tuple(BOOST_RV_REF(argument_tuple) other)
        : m_a0(boost::forward<A0>(other.m_a0)),
          m_a1(boost::forward<A1>(other.m_a1)),
          m_a2(boost::forward<A2>(other.m_a2)),
          m_a3(boost::forward<A3>(other.m_a3))
    {
    }

    // Accessors
    A0& get(boost::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(boost::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(boost::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(boost::integral_constant<std::size_t, 1>) const { return m_a1; }

    A2& get(boost::integral_constant<std::size_t, 2>) { return m_a2; }
    const A2& get(boost::integral_constant<std::size_t, 2>) const { return m_a2; }

    A3& get(boost::integral_constant<std::size_t, 3>) { return m_a3; }
    const A3& get(boost::integral_constant<std::size_t, 3>) const { return m_a3; }

private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;

    BOOST_COPYABLE_AND_MOVABLE(argument_tuple)
};

template <typename A0,
          typename A1,
          typename A2>
struct argument_tuple<A0,
                      A1,
                      A2,
                      argument_tuple_null_type>
{
    static const std::size_t size = 3;

    explicit argument_tuple(const A0& a0,
                            const A1& a1,
                            const A2& a2)
        : m_a0(a0),
          m_a1(a1),
          m_a2(a2)
    {
    }

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2>
    explicit argument_tuple(BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
        : m_a0(boost::forward<T0>(t0)),
          m_a1(boost::forward<T1>(t1)),
          m_a2(boost::forward<T2>(t2))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0),
          m_a1(other.m_a1),
          m_a2(other.m_a2)
    {
    }

    // Move constructor
    argument_tuple(BOOST_RV_REF(argument_tuple) other)
        : m_a0(boost::forward<A0>(other.m_a0)),
          m_a1(boost::forward<A1>(other.m_a1)),
          m_a2(boost::forward<A2>(other.m_a2))
    {
    }

    // Accessors
    A0& get(boost::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(boost::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(boost::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(boost::integral_constant<std::size_t, 1>) const { return m_a1; }

    A2& get(boost::integral_constant<std::size_t, 2>) { return m_a2; }
    const A2& get(boost::integral_constant<std::size_t, 2>) const { return m_a2; }

private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;

    BOOST_COPYABLE_AND_MOVABLE(argument_tuple)
};

template <typename A0,
          typename A1>
struct argument_tuple<A0,
                      A1,
                      argument_tuple_null_type,
                      argument_tuple_null_type>
{
    static const std::size_t size = 2;

    explicit argument_tuple(const A0& a0,
                            const A1& a1)
        : m_a0(a0),
          m_a1(a1)
    {
    }

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1>
    explicit argument_tuple(BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
        : m_a0(boost::forward<T0>(t0)),
          m_a1(boost::forward<T1>(t1))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0),
          m_a1(other.m_a1)
    {
    }

    // Move constructor
    argument_tuple(BOOST_RV_REF(argument_tuple) other)
        : m_a0(boost::forward<A0>(other.m_a0)),
          m_a1(boost::forward<A1>(other.m_a1))
    {
    }

    // Accessors
    A0& get(boost::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(boost::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(boost::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(boost::integral_constant<std::size_t, 1>) const { return m_a1; }

private:
    A0 m_a0;
    A1 m_a1;

    BOOST_COPYABLE_AND_MOVABLE(argument_tuple)
};

template <typename A0>
struct argument_tuple<A0,
                      argument_tuple_null_type,
                      argument_tuple_null_type,
                      argument_tuple_null_type>
{
    static const std::size_t size = 1;

    explicit argument_tuple(const A0& a0)
        : m_a0(a0)
    {
    }

    // Constructor with perfect forwarding
    template <typename T0>
    explicit argument_tuple(BOOST_FWD_REF(T0) t0)
        : m_a0(boost::forward<T0>(t0))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0)
    {
    }

    // Move constructor
    argument_tuple(BOOST_RV_REF(argument_tuple) other)
        : m_a0(boost::forward<A0>(other.m_a0))
    {
    }

    // Accessors
    A0& get(boost::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(boost::integral_constant<std::size_t, 0>) const { return m_a0; }

private:
    A0 m_a0;

    BOOST_COPYABLE_AND_MOVABLE(argument_tuple)
};

template <>
struct argument_tuple<argument_tuple_null_type,
                      argument_tuple_null_type,
                      argument_tuple_null_type,
                      argument_tuple_null_type>
{
    static const std::size_t size = 0;

private:
};


// --------------------------------------------------------------------
//     Get the type at index \p TIndex.
// --------------------------------------------------------------------
template <std::size_t TIndex, typename T>
struct argument_tuple_element;

// Recursive case
template <std::size_t TIndex,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_element<TIndex, argument_tuple<A0, A1, A2, A3> >
        : argument_tuple_element<TIndex - 1, argument_tuple<A1, A2, A3> >
{
};

// End of recursion
template <typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_element<0, argument_tuple<A0, A1, A2, A3> >
{
    typedef A0 type;
};

template <std::size_t TIndex,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_element<TIndex, const argument_tuple<A0, A1, A2, A3> >
{
    typedef argument_tuple<A0, A1, A2, A3> tuple_type;
    typedef typename boost::add_const<
        typename argument_tuple_element<TIndex, tuple_type>::type>::type type;
};

// --------------------------------------------------------------------
//     Get the element at \p TIndex.
// --------------------------------------------------------------------
template <std::size_t TIndex,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename argument_tuple_element<TIndex, argument_tuple<A0, A1, A2, A3> >::type&
    get(argument_tuple<A0, A1, A2, A3>& t)
{
    return t.get(boost::integral_constant<std::size_t, TIndex>());
}

template <std::size_t TIndex,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
const typename argument_tuple_element<TIndex, argument_tuple<A0, A1, A2, A3> >::type&
    get(const argument_tuple<A0, A1, A2, A3>& t)
{
    return t.get(boost::integral_constant<std::size_t, TIndex>());
}

// --------------------------------------------------------------------
//     Get the size.
// --------------------------------------------------------------------
template <typename T>
struct argument_tuple_size;

template <typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_size<argument_tuple<A0, A1, A2, A3> >
        : boost::integral_constant<
              std::size_t,
              argument_tuple<A0, A1, A2, A3>::size>
{
};

template <typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_size<const argument_tuple<A0, A1, A2, A3> >
        : boost::integral_constant<
              std::size_t,
              argument_tuple<A0, A1, A2, A3>::size>
{
};

inline
argument_tuple<> forward_as_argument_tuple()
{
    return argument_tuple<>();
}

template <typename A0>
inline
argument_tuple<BOOST_FWD_REF(A0)>
forward_as_argument_tuple(BOOST_FWD_REF(A0) a0)
{
    return argument_tuple<BOOST_FWD_REF(A0)>(
            boost::forward<A0>(a0));
}

template <typename A0,
          typename A1>
inline
argument_tuple<BOOST_FWD_REF(A0),
               BOOST_FWD_REF(A1)>
forward_as_argument_tuple(BOOST_FWD_REF(A0) a0,
                          BOOST_FWD_REF(A1) a1)
{
    return argument_tuple<BOOST_FWD_REF(A0),
                          BOOST_FWD_REF(A1)>(
            boost::forward<A0>(a0),
            boost::forward<A1>(a1));
}

template <typename A0,
          typename A1,
          typename A2>
inline
argument_tuple<BOOST_FWD_REF(A0),
               BOOST_FWD_REF(A1),
               BOOST_FWD_REF(A2)>
forward_as_argument_tuple(BOOST_FWD_REF(A0) a0,
                          BOOST_FWD_REF(A1) a1,
                          BOOST_FWD_REF(A2) a2)
{
    return argument_tuple<BOOST_FWD_REF(A0),
                          BOOST_FWD_REF(A1),
                          BOOST_FWD_REF(A2)>(
            boost::forward<A0>(a0),
            boost::forward<A1>(a1),
            boost::forward<A2>(a2));
}

template <typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
argument_tuple<BOOST_FWD_REF(A0),
               BOOST_FWD_REF(A1),
               BOOST_FWD_REF(A2),
               BOOST_FWD_REF(A3)>
forward_as_argument_tuple(BOOST_FWD_REF(A0) a0,
                          BOOST_FWD_REF(A1) a1,
                          BOOST_FWD_REF(A2) a2,
                          BOOST_FWD_REF(A3) a3)
{
    return argument_tuple<BOOST_FWD_REF(A0),
                          BOOST_FWD_REF(A1),
                          BOOST_FWD_REF(A2),
                          BOOST_FWD_REF(A3)>(
            boost::forward<A0>(a0),
            boost::forward<A1>(a1),
            boost::forward<A2>(a2),
            boost::forward<A3>(a3));
}

struct placeholder_out_of_bounds;

template <int TIndex, typename TArguments,
          bool TValid = (TIndex < argument_tuple_size<TArguments>::value)>
struct placeholder_bounds_checker
{
    typedef typename argument_tuple_element<TIndex, TArguments>::type type;
};

template <int TIndex, typename TArguments>
struct placeholder_bounds_checker<TIndex, TArguments, false>
{
    typedef placeholder_out_of_bounds type;
};

template <typename TBound, typename TUnbound>
struct unpacked_argument_type
{
    typedef TBound type;
};

template <int TIndex, typename TUnbound>
struct unpacked_argument_type<placeholders::placeholder<TIndex>, TUnbound>
{
    typedef typename placeholder_bounds_checker<TIndex - 1, TUnbound>::type temp_type;
    typedef typename boost::add_rvalue_reference<temp_type>::type type;
};

template <typename TBound>
struct unpack_argument
{
    template <typename TType, typename TUnbound>
    BOOST_FWD_REF(TType) operator() (BOOST_FWD_REF(TType) bound,
                                     TUnbound& unbound) const
    {
        return boost::forward<TType>(bound);
    }
};

template <int TIndex>
struct unpack_argument<placeholders::placeholder<TIndex> >
{
    typedef placeholders::placeholder<TIndex> bound_type;

    template <typename TUnbound>
    typename unpacked_argument_type<bound_type, TUnbound>::type operator() (
        const bound_type& /*bound*/, TUnbound& unbound) const
    {
        return boost::forward<typename unpacked_argument_type<
                                  bound_type, TUnbound>::type>(
                    get<TIndex - 1>(unbound));
    }
};

template <typename TResult, typename TSignature>
struct BindResult;

template <typename TResult, typename F>
struct BindResult<TResult, F()>
{
    typedef TResult result_type;

    // Constructor with perfect forwarding
    explicit BindResult(const F& f)
        : m_functor(f)
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(BOOST_RV_REF(BindResult) other)
        : m_functor(boost::move(other.m_functor)),
          m_arguments(boost::move(other.m_arguments))
    {
    }

    result_type operator() ()
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    result_type operator() () const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor();
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor();
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor();
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor();
    }

    BOOST_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0>
struct BindResult<TResult, F(A0)>
{
    typedef TResult result_type;

    // Constructor with perfect forwarding
    template <typename T0>
    explicit BindResult(const F& f,
                        BOOST_FWD_REF(T0) t0)
        : m_functor(f),
          m_arguments(boost::forward<T0>(t0))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(BOOST_RV_REF(BindResult) other)
        : m_functor(boost::move(other.m_functor)),
          m_arguments(boost::move(other.m_arguments))
    {
    }

    result_type operator() ()
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    result_type operator() () const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    // Invoke member function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    // Invoke member function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    BOOST_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1>
struct BindResult<TResult, F(A0, A1)>
{
    typedef TResult result_type;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1>
    explicit BindResult(const F& f,
                        BOOST_FWD_REF(T0) t0,
                        BOOST_FWD_REF(T1) t1)
        : m_functor(f),
          m_arguments(boost::forward<T0>(t0),
                      boost::forward<T1>(t1))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(BOOST_RV_REF(BindResult) other)
        : m_functor(boost::move(other.m_functor)),
          m_arguments(boost::move(other.m_arguments))
    {
    }

    result_type operator() ()
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    result_type operator() () const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    BOOST_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1,
          typename A2>
struct BindResult<TResult, F(A0, A1, A2)>
{
    typedef TResult result_type;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2>
    explicit BindResult(const F& f,
                        BOOST_FWD_REF(T0) t0,
                        BOOST_FWD_REF(T1) t1,
                        BOOST_FWD_REF(T2) t2)
        : m_functor(f),
          m_arguments(boost::forward<T0>(t0),
                      boost::forward<T1>(t1),
                      boost::forward<T2>(t2))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(BOOST_RV_REF(BindResult) other)
        : m_functor(boost::move(other.m_functor)),
          m_arguments(boost::move(other.m_arguments))
    {
    }

    result_type operator() ()
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    result_type operator() () const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1, A2> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args));
    }

    BOOST_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
struct BindResult<TResult, F(A0, A1, A2, A3)>
{
    typedef TResult result_type;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    explicit BindResult(const F& f,
                        BOOST_FWD_REF(T0) t0,
                        BOOST_FWD_REF(T1) t1,
                        BOOST_FWD_REF(T2) t2,
                        BOOST_FWD_REF(T3) t3)
        : m_functor(f),
          m_arguments(boost::forward<T0>(t0),
                      boost::forward<T1>(t1),
                      boost::forward<T2>(t2),
                      boost::forward<T3>(t3))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(BOOST_RV_REF(BindResult) other)
        : m_functor(boost::move(other.m_functor)),
          m_arguments(boost::move(other.m_arguments))
    {
    }

    result_type operator() ()
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    result_type operator() () const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple());
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (BOOST_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (BOOST_FWD_REF(T0) t0,
                            BOOST_FWD_REF(T1) t1,
                            BOOST_FWD_REF(T2) t2,
                            BOOST_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1, A2, A3> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && !boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor(
            unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    // Invoke member function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            BOOST_FWD_REF(TUnbound) unbound_args,
            typename boost::enable_if_c<
                !boost::is_same<TReturn, void>::value
                && boost::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<2, arguments_type>::type>()(
                    get<2>(m_arguments), unbound_args),
            unpack_argument<
                typename argument_tuple_element<3, arguments_type>::type>()(
                    get<3>(m_arguments), unbound_args));
    }

    BOOST_COPYABLE_AND_MOVABLE(BindResult)
};


struct bind_helper_null_type;

template <typename TResult,
          typename TFunctor,
          typename A0 = bind_helper_null_type,
          typename A1 = bind_helper_null_type,
          typename A2 = bind_helper_null_type,
          typename A3 = bind_helper_null_type>
struct bind_helper
{
    typedef typename boost::decay<TFunctor>::type functor_type;
    typedef BindResult<TResult,
                       functor_type(typename boost::decay<A0>::type,
                                    typename boost::decay<A1>::type,
                                    typename boost::decay<A2>::type,
                                    typename boost::decay<A3>::type)> type;
};

template <typename TResult,
          typename TFunctor,
          typename A0,
          typename A1,
          typename A2>
struct bind_helper<TResult, TFunctor,
                   A0,
                   A1,
                   A2,
                   bind_helper_null_type>
{
    typedef typename boost::decay<TFunctor>::type functor_type;
    typedef BindResult<TResult,
                       functor_type(typename boost::decay<A0>::type,
                                    typename boost::decay<A1>::type,
                                    typename boost::decay<A2>::type)> type;
};

template <typename TResult,
          typename TFunctor,
          typename A0,
          typename A1>
struct bind_helper<TResult, TFunctor,
                   A0,
                   A1,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    typedef typename boost::decay<TFunctor>::type functor_type;
    typedef BindResult<TResult,
                       functor_type(typename boost::decay<A0>::type,
                                    typename boost::decay<A1>::type)> type;
};

template <typename TResult,
          typename TFunctor,
          typename A0>
struct bind_helper<TResult, TFunctor,
                   A0,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    typedef typename boost::decay<TFunctor>::type functor_type;
    typedef BindResult<TResult,
                       functor_type(typename boost::decay<A0>::type)> type;
};

template <typename TResult,
          typename TFunctor>
struct bind_helper<TResult, TFunctor,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    typedef typename boost::decay<TFunctor>::type functor_type;
    typedef BindResult<TResult,
                       functor_type()> type;
};

} // namespace detail

// ====================================================================
// bind<>
// ====================================================================

template <typename TResult>
inline
typename detail::bind_helper<TResult,
                             TResult (*) ()>::type
bind(TResult (*f) ())
{
    typedef TResult (*fun_t) ();
    typedef typename detail::bind_helper<TResult, fun_t>::type bound_type;
    return bound_type(f);
}

template <typename TResult,
          typename T0,
          typename A0>
inline
typename detail::bind_helper<TResult,
                             TResult (*) (T0)>::type
bind(TResult (*f) (T0),
     BOOST_FWD_REF(A0) a0)
{
    typedef TResult (*fun_t) (T0);
    typedef typename detail::bind_helper<TResult, fun_t,
                                         A0>::type bound_type;
    return bound_type(f,
                      boost::forward<A0>(a0));
}

template <typename TResult,
          typename T0,
          typename T1,
          typename A0,
          typename A1>
inline
typename detail::bind_helper<TResult,
                             TResult (*) (T0, T1)>::type
bind(TResult (*f) (T0, T1),
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1)
{
    typedef TResult (*fun_t) (T0, T1);
    typedef typename detail::bind_helper<TResult, fun_t,
                                         A0,
                                         A1>::type bound_type;
    return bound_type(f,
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1));
}

template <typename TResult,
          typename T0,
          typename T1,
          typename T2,
          typename A0,
          typename A1,
          typename A2>
inline
typename detail::bind_helper<TResult,
                             TResult (*) (T0, T1, T2)>::type
bind(TResult (*f) (T0, T1, T2),
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1,
     BOOST_FWD_REF(A2) a2)
{
    typedef TResult (*fun_t) (T0, T1, T2);
    typedef typename detail::bind_helper<TResult, fun_t,
                                         A0,
                                         A1,
                                         A2>::type bound_type;
    return bound_type(f,
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1),
                      boost::forward<A2>(a2));
}

template <typename TResult,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename detail::bind_helper<TResult,
                             TResult (*) (T0, T1, T2, T3)>::type
bind(TResult (*f) (T0, T1, T2, T3),
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1,
     BOOST_FWD_REF(A2) a2,
     BOOST_FWD_REF(A3) a3)
{
    typedef TResult (*fun_t) (T0, T1, T2, T3);
    typedef typename detail::bind_helper<TResult, fun_t,
                                         A0,
                                         A1,
                                         A2,
                                         A3>::type bound_type;
    return bound_type(f,
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1),
                      boost::forward<A2>(a2),
                      boost::forward<A3>(a3));
}

template <typename TResult,
          typename TFunctor>
inline
typename detail::bind_helper<TResult,
                             TFunctor>::type
bind(BOOST_FWD_REF(TFunctor) f)
{
    typedef typename detail::bind_helper<TResult,
                                         TFunctor>::type bound_type;
    return bound_type(boost::forward<TFunctor>(f));
}

template <typename TResult,
          typename TFunctor,
          typename A0>
inline
typename detail::bind_helper<TResult,
                             TFunctor,
                             A0>::type
bind(BOOST_FWD_REF(TFunctor) f,
     BOOST_FWD_REF(A0) a0)
{
    typedef typename detail::bind_helper<TResult,
                                         TFunctor,
                                         A0>::type bound_type;
    return bound_type(boost::forward<TFunctor>(f),
                      boost::forward<A0>(a0));
}

template <typename TResult,
          typename TFunctor,
          typename A0,
          typename A1>
inline
typename detail::bind_helper<TResult,
                             TFunctor,
                             A0,
                             A1>::type
bind(BOOST_FWD_REF(TFunctor) f,
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1)
{
    typedef typename detail::bind_helper<TResult,
                                         TFunctor,
                                         A0,
                                         A1>::type bound_type;
    return bound_type(boost::forward<TFunctor>(f),
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1));
}

template <typename TResult,
          typename TFunctor,
          typename A0,
          typename A1,
          typename A2>
inline
typename detail::bind_helper<TResult,
                             TFunctor,
                             A0,
                             A1,
                             A2>::type
bind(BOOST_FWD_REF(TFunctor) f,
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1,
     BOOST_FWD_REF(A2) a2)
{
    typedef typename detail::bind_helper<TResult,
                                         TFunctor,
                                         A0,
                                         A1,
                                         A2>::type bound_type;
    return bound_type(boost::forward<TFunctor>(f),
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1),
                      boost::forward<A2>(a2));
}

template <typename TResult,
          typename TFunctor,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename detail::bind_helper<TResult,
                             TFunctor,
                             A0,
                             A1,
                             A2,
                             A3>::type
bind(BOOST_FWD_REF(TFunctor) f,
     BOOST_FWD_REF(A0) a0,
     BOOST_FWD_REF(A1) a1,
     BOOST_FWD_REF(A2) a2,
     BOOST_FWD_REF(A3) a3)
{
    typedef typename detail::bind_helper<TResult,
                                         TFunctor,
                                         A0,
                                         A1,
                                         A2,
                                         A3>::type bound_type;
    return bound_type(boost::forward<TFunctor>(f),
                      boost::forward<A0>(a0),
                      boost::forward<A1>(a1),
                      boost::forward<A2>(a2),
                      boost::forward<A3>(a3));
}

// ====================================================================
// function<>
// ====================================================================

namespace detail
{

enum AdapterTask
{
    AdapterTask_Clone,
    AdapterTask_CloneInto,
    AdapterTask_Destroy
};

// An adapter which allows to erase the type of a bind expression.
template <typename TBindResult>
struct BindAdapter
{
    static void* manage(AdapterTask task, void* self, const void* other)
    {
        switch (task)
        {
        case AdapterTask_Clone:
            self = std::malloc(sizeof(TBindResult)); //! \todo Hold in unique_ptr until copy construction is done
            new (self) TBindResult(*static_cast<const TBindResult*>(other));
            return self;
        case AdapterTask_CloneInto:
            new (self) TBindResult(*static_cast<const TBindResult*>(other));
            return self;
        case AdapterTask_Destroy:
            static_cast<TBindResult*>(self)->~TBindResult();
            return 0;
        }
        return 0;
    }

    static typename TBindResult::result_type invoke(
            void* bindExpression)
    {
        return (*static_cast<TBindResult*>(bindExpression))();
    }

    template <typename T0>
    static typename TBindResult::result_type invoke(
            void* bindExpression,
            T0 t0)
    {
        return (*static_cast<TBindResult*>(bindExpression))(
                    boost::forward<T0>(t0));
    }

    template <typename T0,
              typename T1>
    static typename TBindResult::result_type invoke(
            void* bindExpression,
            T0 t0,
            T1 t1)
    {
        return (*static_cast<TBindResult*>(bindExpression))(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1));
    }

    template <typename T0,
              typename T1,
              typename T2>
    static typename TBindResult::result_type invoke(
            void* bindExpression,
            T0 t0,
            T1 t1,
            T2 t2)
    {
        return (*static_cast<TBindResult*>(bindExpression))(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    static typename TBindResult::result_type invoke(
            void* bindExpression,
            T0 t0,
            T1 t1,
            T2 t2,
            T3 t3)
    {
        return (*static_cast<TBindResult*>(bindExpression))(
                    boost::forward<T0>(t0),
                    boost::forward<T1>(t1),
                    boost::forward<T2>(t2),
                    boost::forward<T3>(t3));
    }

};

} // namespace detail

template <typename TSignature>
class function;

template <typename TResult>
class function<TResult()>
{
public:
    typedef TResult result_type;

    function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(const function& other)
        : m_invoker(0)
    {
        *this = other;
    }

    template <typename TSignature>
    function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~function()
    {
        release();
    }

    function& operator= (const function& other)
    {
        if (this != &other)
        {
            release();
            if (other.m_invoker)
            {
                m_manager = other.m_manager;
                m_storage = m_manager(detail::AdapterTask_Clone, 0, other.m_storage);
                m_invoker = other.m_invoker;
            }
        }
        return *this;
    }

    template <typename TSignature>
    function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_storage = m_manager(detail::AdapterTask_Clone, 0, &expr);
        m_invoker = &adapter::invoke;
        return *this;
    }

    result_type operator() () const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage);
    }

    function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*);

    void* m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
            std::free(m_storage);
        }
    }
};

template <typename TResult,
          typename A0>
class function<TResult(A0)>
{
public:
    typedef TResult result_type;

    function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(const function& other)
        : m_invoker(0)
    {
        *this = other;
    }

    template <typename TSignature>
    function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~function()
    {
        release();
    }

    function& operator= (const function& other)
    {
        if (this != &other)
        {
            release();
            if (other.m_invoker)
            {
                m_manager = other.m_manager;
                m_storage = m_manager(detail::AdapterTask_Clone, 0, other.m_storage);
                m_invoker = other.m_invoker;
            }
        }
        return *this;
    }

    template <typename TSignature>
    function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_storage = m_manager(detail::AdapterTask_Clone, 0, &expr);
        m_invoker = &adapter::template invoke<A0>;
        return *this;
    }

    result_type operator() (A0 a0) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            boost::forward<A0>(a0));
    }

    function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0);

    void* m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
            std::free(m_storage);
        }
    }
};

template <typename TResult,
          typename A0,
          typename A1>
class function<TResult(A0, A1)>
{
public:
    typedef TResult result_type;

    function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(const function& other)
        : m_invoker(0)
    {
        *this = other;
    }

    template <typename TSignature>
    function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~function()
    {
        release();
    }

    function& operator= (const function& other)
    {
        if (this != &other)
        {
            release();
            if (other.m_invoker)
            {
                m_manager = other.m_manager;
                m_storage = m_manager(detail::AdapterTask_Clone, 0, other.m_storage);
                m_invoker = other.m_invoker;
            }
        }
        return *this;
    }

    template <typename TSignature>
    function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_storage = m_manager(detail::AdapterTask_Clone, 0, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1));
    }

    function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1);

    void* m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
            std::free(m_storage);
        }
    }
};

template <typename TResult,
          typename A0,
          typename A1,
          typename A2>
class function<TResult(A0, A1, A2)>
{
public:
    typedef TResult result_type;

    function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(const function& other)
        : m_invoker(0)
    {
        *this = other;
    }

    template <typename TSignature>
    function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~function()
    {
        release();
    }

    function& operator= (const function& other)
    {
        if (this != &other)
        {
            release();
            if (other.m_invoker)
            {
                m_manager = other.m_manager;
                m_storage = m_manager(detail::AdapterTask_Clone, 0, other.m_storage);
                m_invoker = other.m_invoker;
            }
        }
        return *this;
    }

    template <typename TSignature>
    function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_storage = m_manager(detail::AdapterTask_Clone, 0, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1,
                                              A2>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1,
                            A2 a2) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1),
                            boost::forward<A2>(a2));
    }

    function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1,
                                        A2);

    void* m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
            std::free(m_storage);
        }
    }
};

template <typename TResult,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
class function<TResult(A0, A1, A2, A3)>
{
public:
    typedef TResult result_type;

    function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    function(const function& other)
        : m_invoker(0)
    {
        *this = other;
    }

    template <typename TSignature>
    function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~function()
    {
        release();
    }

    function& operator= (const function& other)
    {
        if (this != &other)
        {
            release();
            if (other.m_invoker)
            {
                m_manager = other.m_manager;
                m_storage = m_manager(detail::AdapterTask_Clone, 0, other.m_storage);
                m_invoker = other.m_invoker;
            }
        }
        return *this;
    }

    template <typename TSignature>
    function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_storage = m_manager(detail::AdapterTask_Clone, 0, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1,
                                              A2,
                                              A3>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1,
                            A2 a2,
                            A3 a3) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1),
                            boost::forward<A2>(a2),
                            boost::forward<A3>(a3));
    }

    function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1,
                                        A2,
                                        A3);

    void* m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
            std::free(m_storage);
        }
    }
};

// ====================================================================
// static_function<>
// ====================================================================

template <typename TSignature,
          std::size_t TStorageSize = WEOS_DEFAULT_STATIC_FUNCTION_SIZE>
class static_function;

template <typename TResult,
          std::size_t TStorageSize>
class static_function<TResult(), TStorageSize>
{
public:
    typedef TResult result_type;

    static_function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    static_function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    template <typename TSignature>
    static_function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~static_function()
    {
        release();
    }

    template <typename TSignature>
    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,
                                "The bind expression is too large for this function.");

        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_manager(detail::AdapterTask_CloneInto, &m_storage, &expr);
        m_invoker = &adapter::invoke;
        return *this;
    }

    result_type operator() () const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(&m_storage);
    }

    static_function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*);

    typename boost::aligned_storage<TStorageSize>::type m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);
            m_invoker = 0;
        }
    }
static_function(const static_function&);
static_function& operator= (const static_function&);
};

template <typename TResult,
          typename A0,
          std::size_t TStorageSize>
class static_function<TResult(A0), TStorageSize>
{
public:
    typedef TResult result_type;

    static_function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    static_function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    template <typename TSignature>
    static_function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~static_function()
    {
        release();
    }

    template <typename TSignature>
    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,
                                "The bind expression is too large for this function.");

        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_manager(detail::AdapterTask_CloneInto, &m_storage, &expr);
        m_invoker = &adapter::template invoke<A0>;
        return *this;
    }

    result_type operator() (A0 a0) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(&m_storage,
                            boost::forward<A0>(a0));
    }

    static_function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0);

    typename boost::aligned_storage<TStorageSize>::type m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);
            m_invoker = 0;
        }
    }
static_function(const static_function&);
static_function& operator= (const static_function&);
};

template <typename TResult,
          typename A0,
          typename A1,
          std::size_t TStorageSize>
class static_function<TResult(A0, A1), TStorageSize>
{
public:
    typedef TResult result_type;

    static_function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    static_function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    template <typename TSignature>
    static_function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~static_function()
    {
        release();
    }

    template <typename TSignature>
    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,
                                "The bind expression is too large for this function.");

        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_manager(detail::AdapterTask_CloneInto, &m_storage, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(&m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1));
    }

    static_function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1);

    typename boost::aligned_storage<TStorageSize>::type m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);
            m_invoker = 0;
        }
    }
static_function(const static_function&);
static_function& operator= (const static_function&);
};

template <typename TResult,
          typename A0,
          typename A1,
          typename A2,
          std::size_t TStorageSize>
class static_function<TResult(A0, A1, A2), TStorageSize>
{
public:
    typedef TResult result_type;

    static_function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    static_function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    template <typename TSignature>
    static_function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~static_function()
    {
        release();
    }

    template <typename TSignature>
    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,
                                "The bind expression is too large for this function.");

        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_manager(detail::AdapterTask_CloneInto, &m_storage, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1,
                                              A2>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1,
                            A2 a2) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(&m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1),
                            boost::forward<A2>(a2));
    }

    static_function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1,
                                        A2);

    typename boost::aligned_storage<TStorageSize>::type m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);
            m_invoker = 0;
        }
    }
static_function(const static_function&);
static_function& operator= (const static_function&);
};

template <typename TResult,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          std::size_t TStorageSize>
class static_function<TResult(A0, A1, A2, A3), TStorageSize>
{
public:
    typedef TResult result_type;

    static_function() WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    static_function(nullptr_t) WEOS_NOEXCEPT
        : m_invoker(0)
    {
    }

    template <typename TSignature>
    static_function(const detail::BindResult<result_type, TSignature>& expr)
        : m_invoker(0)
    {
        *this = expr;
    }

    ~static_function()
    {
        release();
    }

    template <typename TSignature>
    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)
    {
        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,
                                "The bind expression is too large for this function.");

        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;

        release();
        m_manager = &adapter::manage;
        m_manager(detail::AdapterTask_CloneInto, &m_storage, &expr);
        m_invoker = &adapter::template invoke<A0,
                                              A1,
                                              A2,
                                              A3>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1,
                            A2 a2,
                            A3 a3) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(&m_storage,
                            boost::forward<A0>(a0),
                            boost::forward<A1>(a1),
                            boost::forward<A2>(a2),
                            boost::forward<A3>(a3));
    }

    static_function& operator= (nullptr_t)
    {
        release();
        return *this;
    }

    /*explicit*/ operator bool() const WEOS_NOEXCEPT
    {
        return m_invoker != 0;
    }

private:
    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);
    typedef result_type (*invoker_type)(void*,
                                        A0,
                                        A1,
                                        A2,
                                        A3);

    typename boost::aligned_storage<TStorageSize>::type m_storage;
    manager_type m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);
            m_invoker = 0;
        }
    }
static_function(const static_function&);
static_function& operator= (const static_function&);
};

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_FUNCTIONAL_HPP

