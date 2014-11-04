
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

#include "../type_traits.hpp"
#include "../utility.hpp"

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
struct is_placeholder : WEOS_NAMESPACE::integral_constant<int, 0>
{
};

template <int TIndex>
struct is_placeholder<placeholders::placeholder<TIndex> >
        : WEOS_NAMESPACE::integral_constant<int, TIndex>
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
    explicit argument_tuple(WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
        : m_a0(WEOS_NAMESPACE::forward<T0>(t0)),
          m_a1(WEOS_NAMESPACE::forward<T1>(t1)),
          m_a2(WEOS_NAMESPACE::forward<T2>(t2)),
          m_a3(WEOS_NAMESPACE::forward<T3>(t3))
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
    argument_tuple(WEOS_RV_REF(argument_tuple) other)
        : m_a0(WEOS_NAMESPACE::forward<A0>(other.m_a0)),
          m_a1(WEOS_NAMESPACE::forward<A1>(other.m_a1)),
          m_a2(WEOS_NAMESPACE::forward<A2>(other.m_a2)),
          m_a3(WEOS_NAMESPACE::forward<A3>(other.m_a3))
    {
    }

    // Accessors
    A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) const { return m_a1; }

    A2& get(WEOS_NAMESPACE::integral_constant<std::size_t, 2>) { return m_a2; }
    const A2& get(WEOS_NAMESPACE::integral_constant<std::size_t, 2>) const { return m_a2; }

    A3& get(WEOS_NAMESPACE::integral_constant<std::size_t, 3>) { return m_a3; }
    const A3& get(WEOS_NAMESPACE::integral_constant<std::size_t, 3>) const { return m_a3; }

private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;

    WEOS_COPYABLE_AND_MOVABLE(argument_tuple)
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
    explicit argument_tuple(WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
        : m_a0(WEOS_NAMESPACE::forward<T0>(t0)),
          m_a1(WEOS_NAMESPACE::forward<T1>(t1)),
          m_a2(WEOS_NAMESPACE::forward<T2>(t2))
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
    argument_tuple(WEOS_RV_REF(argument_tuple) other)
        : m_a0(WEOS_NAMESPACE::forward<A0>(other.m_a0)),
          m_a1(WEOS_NAMESPACE::forward<A1>(other.m_a1)),
          m_a2(WEOS_NAMESPACE::forward<A2>(other.m_a2))
    {
    }

    // Accessors
    A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) const { return m_a1; }

    A2& get(WEOS_NAMESPACE::integral_constant<std::size_t, 2>) { return m_a2; }
    const A2& get(WEOS_NAMESPACE::integral_constant<std::size_t, 2>) const { return m_a2; }

private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;

    WEOS_COPYABLE_AND_MOVABLE(argument_tuple)
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
    explicit argument_tuple(WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
        : m_a0(WEOS_NAMESPACE::forward<T0>(t0)),
          m_a1(WEOS_NAMESPACE::forward<T1>(t1))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0),
          m_a1(other.m_a1)
    {
    }

    // Move constructor
    argument_tuple(WEOS_RV_REF(argument_tuple) other)
        : m_a0(WEOS_NAMESPACE::forward<A0>(other.m_a0)),
          m_a1(WEOS_NAMESPACE::forward<A1>(other.m_a1))
    {
    }

    // Accessors
    A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) const { return m_a0; }

    A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) { return m_a1; }
    const A1& get(WEOS_NAMESPACE::integral_constant<std::size_t, 1>) const { return m_a1; }

private:
    A0 m_a0;
    A1 m_a1;

    WEOS_COPYABLE_AND_MOVABLE(argument_tuple)
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
    explicit argument_tuple(WEOS_FWD_REF(T0) t0)
        : m_a0(WEOS_NAMESPACE::forward<T0>(t0))
    {
    }

    // Copy constructor
    argument_tuple(const argument_tuple& other)
        : m_a0(other.m_a0)
    {
    }

    // Move constructor
    argument_tuple(WEOS_RV_REF(argument_tuple) other)
        : m_a0(WEOS_NAMESPACE::forward<A0>(other.m_a0))
    {
    }

    // Accessors
    A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) { return m_a0; }
    const A0& get(WEOS_NAMESPACE::integral_constant<std::size_t, 0>) const { return m_a0; }

private:
    A0 m_a0;

    WEOS_COPYABLE_AND_MOVABLE(argument_tuple)
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
    typedef typename WEOS_NAMESPACE::add_const<
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
    return t.get(WEOS_NAMESPACE::integral_constant<std::size_t, TIndex>());
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
    return t.get(WEOS_NAMESPACE::integral_constant<std::size_t, TIndex>());
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
        : WEOS_NAMESPACE::integral_constant<
              std::size_t,
              argument_tuple<A0, A1, A2, A3>::size>
{
};

template <typename A0,
          typename A1,
          typename A2,
          typename A3>
struct argument_tuple_size<const argument_tuple<A0, A1, A2, A3> >
        : WEOS_NAMESPACE::integral_constant<
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
argument_tuple<WEOS_FWD_REF(A0)>
forward_as_argument_tuple(WEOS_FWD_REF(A0) a0)
{
    return argument_tuple<WEOS_FWD_REF(A0)>(
            WEOS_NAMESPACE::forward<A0>(a0));
}

template <typename A0,
          typename A1>
inline
argument_tuple<WEOS_FWD_REF(A0),
               WEOS_FWD_REF(A1)>
forward_as_argument_tuple(WEOS_FWD_REF(A0) a0,
                          WEOS_FWD_REF(A1) a1)
{
    return argument_tuple<WEOS_FWD_REF(A0),
                          WEOS_FWD_REF(A1)>(
            WEOS_NAMESPACE::forward<A0>(a0),
            WEOS_NAMESPACE::forward<A1>(a1));
}

template <typename A0,
          typename A1,
          typename A2>
inline
argument_tuple<WEOS_FWD_REF(A0),
               WEOS_FWD_REF(A1),
               WEOS_FWD_REF(A2)>
forward_as_argument_tuple(WEOS_FWD_REF(A0) a0,
                          WEOS_FWD_REF(A1) a1,
                          WEOS_FWD_REF(A2) a2)
{
    return argument_tuple<WEOS_FWD_REF(A0),
                          WEOS_FWD_REF(A1),
                          WEOS_FWD_REF(A2)>(
            WEOS_NAMESPACE::forward<A0>(a0),
            WEOS_NAMESPACE::forward<A1>(a1),
            WEOS_NAMESPACE::forward<A2>(a2));
}

template <typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
argument_tuple<WEOS_FWD_REF(A0),
               WEOS_FWD_REF(A1),
               WEOS_FWD_REF(A2),
               WEOS_FWD_REF(A3)>
forward_as_argument_tuple(WEOS_FWD_REF(A0) a0,
                          WEOS_FWD_REF(A1) a1,
                          WEOS_FWD_REF(A2) a2,
                          WEOS_FWD_REF(A3) a3)
{
    return argument_tuple<WEOS_FWD_REF(A0),
                          WEOS_FWD_REF(A1),
                          WEOS_FWD_REF(A2),
                          WEOS_FWD_REF(A3)>(
            WEOS_NAMESPACE::forward<A0>(a0),
            WEOS_NAMESPACE::forward<A1>(a1),
            WEOS_NAMESPACE::forward<A2>(a2),
            WEOS_NAMESPACE::forward<A3>(a3));
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
    typedef typename WEOS_NAMESPACE::add_rvalue_reference<temp_type>::type type;
};

template <typename TBound>
struct unpack_argument
{
    template <typename TType, typename TUnbound>
    WEOS_FWD_REF(TType) operator() (WEOS_FWD_REF(TType) bound,
                                     TUnbound& unbound) const
    {
        return WEOS_NAMESPACE::forward<TType>(bound);
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
        return WEOS_NAMESPACE::forward<typename unpacked_argument_type<
                                  bound_type, TUnbound>::type>(
                    get<TIndex - 1>(unbound));
    }
};

// ====================================================================
// MemFnResult
// ====================================================================

template <typename TMemberPointer>
class MemFnResult;

// Result of mem_fn(TResult (TClass::*) () )
template <typename TResult,
          typename TClass>
class MemFnResult<TResult (TClass::*) () >
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) () ;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)();
    }

    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*) const
    {
        return ((*ptr).*m_pm)();
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    result_type operator() ( TClass& object) const
    {
        return (object.*m_pm)();
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    result_type operator() ( TClass&& object) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)();
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    result_type operator() ( TClass* object) const
    {
        return (object->*m_pm)();
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer>
    result_type operator() (WEOS_FWD_REF(TPointer) object) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object);
    }

};

// Result of mem_fn(TResult (TClass::*) () const)
template <typename TResult,
          typename TClass>
class MemFnResult<TResult (TClass::*) () const>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) () const;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)();
    }

    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*) const
    {
        return ((*ptr).*m_pm)();
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    result_type operator() (const TClass& object) const
    {
        return (object.*m_pm)();
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    result_type operator() (const TClass&& object) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)();
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    result_type operator() (const TClass* object) const
    {
        return (object->*m_pm)();
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer>
    result_type operator() (WEOS_FWD_REF(TPointer) object) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object);
    }

};

// Result of mem_fn(TResult (TClass::*) () volatile)
template <typename TResult,
          typename TClass>
class MemFnResult<TResult (TClass::*) () volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) () volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)();
    }

    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*) const
    {
        return ((*ptr).*m_pm)();
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    result_type operator() (volatile TClass& object) const
    {
        return (object.*m_pm)();
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    result_type operator() (volatile TClass&& object) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)();
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    result_type operator() (volatile TClass* object) const
    {
        return (object->*m_pm)();
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer>
    result_type operator() (WEOS_FWD_REF(TPointer) object) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object);
    }

};

// Result of mem_fn(TResult (TClass::*) () const volatile)
template <typename TResult,
          typename TClass>
class MemFnResult<TResult (TClass::*) () const volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) () const volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)();
    }

    template <typename TPointer>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*) const
    {
        return ((*ptr).*m_pm)();
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    result_type operator() (const volatile TClass& object) const
    {
        return (object.*m_pm)();
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    result_type operator() (const volatile TClass&& object) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)();
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    result_type operator() (const volatile TClass* object) const
    {
        return (object->*m_pm)();
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer>
    result_type operator() (WEOS_FWD_REF(TPointer) object) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object);
    }

};

// Result of mem_fn(TResult (TClass::*) (A0) )
template <typename TResult,
          typename TClass,
          typename A0>
class MemFnResult<TResult (TClass::*) (A0) >
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0) ;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0));
    }

    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0>
    result_type operator() ( TClass& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0>
    result_type operator() ( TClass&& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0>
    result_type operator() ( TClass* object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0) const)
template <typename TResult,
          typename TClass,
          typename A0>
class MemFnResult<TResult (TClass::*) (A0) const>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0) const;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0));
    }

    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0>
    result_type operator() (const TClass& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0>
    result_type operator() (const TClass&& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0>
    result_type operator() (const TClass* object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0) volatile)
template <typename TResult,
          typename TClass,
          typename A0>
class MemFnResult<TResult (TClass::*) (A0) volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0) volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0));
    }

    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0>
    result_type operator() (volatile TClass& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0>
    result_type operator() (volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0>
    result_type operator() (volatile TClass* object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0) const volatile)
template <typename TResult,
          typename TClass,
          typename A0>
class MemFnResult<TResult (TClass::*) (A0) const volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0) const volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0));
    }

    template <typename TPointer,
              typename T0>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0>
    result_type operator() (const volatile TClass& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0>
    result_type operator() (const volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0>
    result_type operator() (const volatile TClass* object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1) )
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1>
class MemFnResult<TResult (TClass::*) (A0, A1) >
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1) ;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1));
    }

    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1>
    result_type operator() ( TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1>
    result_type operator() ( TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1>
    result_type operator() ( TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1) const)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1>
class MemFnResult<TResult (TClass::*) (A0, A1) const>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1) const;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1));
    }

    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1>
    result_type operator() (const TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1>
    result_type operator() (const TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1>
    result_type operator() (const TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1) volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1>
class MemFnResult<TResult (TClass::*) (A0, A1) volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1) volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1));
    }

    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1>
    result_type operator() (volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1>
    result_type operator() (volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1>
    result_type operator() (volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1) const volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1>
class MemFnResult<TResult (TClass::*) (A0, A1) const volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1) const volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1));
    }

    template <typename TPointer,
              typename T0,
              typename T1>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1>
    result_type operator() (const volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1>
    result_type operator() (const volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1>
    result_type operator() (const volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2) )
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2>
class MemFnResult<TResult (TClass::*) (A0, A1, A2) >
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2) ;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() ( TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() ( TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() ( TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2) const)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2>
class MemFnResult<TResult (TClass::*) (A0, A1, A2) const>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2) const;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2) volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2>
class MemFnResult<TResult (TClass::*) (A0, A1, A2) volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2) volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2) const volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2>
class MemFnResult<TResult (TClass::*) (A0, A1, A2) const volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2) const volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (const volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2, A3) )
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
class MemFnResult<TResult (TClass::*) (A0, A1, A2, A3) >
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2, A3) ;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2),
                WEOS_NAMESPACE::forward<T3>(t3));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() ( TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() ( TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2),
                                          WEOS_NAMESPACE::forward<T3>(t3));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() ( TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2),
                               WEOS_NAMESPACE::forward<T3>(t3));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2, A3) const)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
class MemFnResult<TResult (TClass::*) (A0, A1, A2, A3) const>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2, A3) const;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2),
                WEOS_NAMESPACE::forward<T3>(t3));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2),
                                          WEOS_NAMESPACE::forward<T3>(t3));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2),
                               WEOS_NAMESPACE::forward<T3>(t3));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2, A3) volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
class MemFnResult<TResult (TClass::*) (A0, A1, A2, A3) volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2, A3) volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2),
                WEOS_NAMESPACE::forward<T3>(t3));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2),
                                          WEOS_NAMESPACE::forward<T3>(t3));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2),
                               WEOS_NAMESPACE::forward<T3>(t3));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3));
    }

};

// Result of mem_fn(TResult (TClass::*) (A0, A1, A2, A3) const volatile)
template <typename TResult,
          typename TClass,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
class MemFnResult<TResult (TClass::*) (A0, A1, A2, A3) const volatile>
{
public:
    typedef TResult result_type;

private:
    typedef TResult (TClass::* mem_fn_t) (A0, A1, A2, A3) const volatile;
    mem_fn_t m_pm;

    // Helpers to differentiate between smart pointers and references/pointers to derived classes
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) object,
                     const volatile TClass*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)(
                WEOS_NAMESPACE::forward<T0>(t0),
                WEOS_NAMESPACE::forward<T1>(t1),
                WEOS_NAMESPACE::forward<T2>(t2),
                WEOS_NAMESPACE::forward<T3>(t3));
    }

    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type call(WEOS_FWD_REF(TPointer) ptr,
                     const volatile void*,
                     WEOS_FWD_REF(T0) t0,
                     WEOS_FWD_REF(T1) t1,
                     WEOS_FWD_REF(T2) t2,
                     WEOS_FWD_REF(T3) t3) const
    {
        return ((*ptr).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm)
        : m_pm(pm)
    {
    }

    // Reference to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const volatile TClass& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object.*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                              WEOS_NAMESPACE::forward<T1>(t1),
                              WEOS_NAMESPACE::forward<T2>(t2),
                              WEOS_NAMESPACE::forward<T3>(t3));
    }

#if defined(WEOS_USE_CXX11)

    // Reference to movable object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const volatile TClass&& object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (WEOS_NAMESPACE::move(object).*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                                          WEOS_NAMESPACE::forward<T1>(t1),
                                          WEOS_NAMESPACE::forward<T2>(t2),
                                          WEOS_NAMESPACE::forward<T3>(t3));
    }

#endif // WEOS_USE_CXX11

    // Pointer to object
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (const volatile TClass* object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return (object->*m_pm)(WEOS_NAMESPACE::forward<T0>(t0),
                               WEOS_NAMESPACE::forward<T1>(t1),
                               WEOS_NAMESPACE::forward<T2>(t2),
                               WEOS_NAMESPACE::forward<T3>(t3));
    }

    // Smart pointer, reference/pointer to derived class
    template <typename TPointer,
              typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(TPointer) object,
                            WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return call(WEOS_NAMESPACE::forward<TPointer>(object),
                    &object,
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3));
    }

};

// Result of mem_fn(TResult TClass::*)
template <typename TResult, typename TClass>
class MemFnResult<TResult TClass::*>
{
    typedef TResult TClass::* mem_fn_t;
    mem_fn_t m_pm;

public:
    explicit WEOS_CONSTEXPR MemFnResult(mem_fn_t pm) WEOS_NOEXCEPT
        : m_pm(pm)
    {
    }

    TResult& operator() (TClass& object) const WEOS_NOEXCEPT
    {
        return object.*m_pm;
    }

    WEOS_CONSTEXPR
    const TResult& operator() (const TClass& object) const WEOS_NOEXCEPT
    {
        return object.*m_pm;
    }

#if defined(WEOS_USE_CXX11)

    TResult&& operator()(TClass&& object) const WEOS_NOEXCEPT
    {
        return WEOS_NAMESPACE::forward<TClass>(object).*m_pm;
    }

    const TResult&& operator()(const TClass&& object) const WEOS_NOEXCEPT
    {
        return WEOS_NAMESPACE::forward<const TClass>(object).*m_pm;
    }

#endif // WEOS_USE_CXX11

    TResult& operator() (TClass* object) const WEOS_NOEXCEPT
    {
        return object->*m_pm;
    }

    WEOS_CONSTEXPR
    const TResult& operator() (const TClass* object) const WEOS_NOEXCEPT
    {
        return object->*m_pm;
    }

    // Smart pointer, reference/pointer to derived class
    //! \todo Missing
};

// ====================================================================
// deduce_result_type
// ====================================================================

// Default case with explicit result type.
template <typename TResult, typename TCallable>
struct deduce_result_type
{
    typedef TResult type;
};

// Function
template <typename R>
struct deduce_result_type<detail::unspecified_type,
                          R  () >
{
    typedef R type;
};

// Function reference
template <typename R>
struct deduce_result_type<detail::unspecified_type,
                          R (&) () >
{
    typedef R type;
};

// Function pointer
template <typename R>
struct deduce_result_type<detail::unspecified_type,
                          R (*) () >
{
    typedef R type;
};

// Member function pointer
template <typename R,
          typename C>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) () >
{
    typedef R type;
};

template <typename R,
          typename C>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) () const>
{
    typedef R type;
};

template <typename R,
          typename C>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) () volatile>
{
    typedef R type;
};

template <typename R,
          typename C>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) () const volatile>
{
    typedef R type;
};

// Function
template <typename R,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R  (A0) >
{
    typedef R type;
};

// Function reference
template <typename R,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (&) (A0) >
{
    typedef R type;
};

// Function pointer
template <typename R,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (*) (A0) >
{
    typedef R type;
};

// Member function pointer
template <typename R,
          typename C,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0) >
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0) const>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0) volatile>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0) const volatile>
{
    typedef R type;
};

// Function
template <typename R,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R  (A0, A1) >
{
    typedef R type;
};

// Function reference
template <typename R,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (&) (A0, A1) >
{
    typedef R type;
};

// Function pointer
template <typename R,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (*) (A0, A1) >
{
    typedef R type;
};

// Member function pointer
template <typename R,
          typename C,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1) >
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1) const>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1) volatile>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1) const volatile>
{
    typedef R type;
};

// Function
template <typename R,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R  (A0, A1, A2) >
{
    typedef R type;
};

// Function reference
template <typename R,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (&) (A0, A1, A2) >
{
    typedef R type;
};

// Function pointer
template <typename R,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (*) (A0, A1, A2) >
{
    typedef R type;
};

// Member function pointer
template <typename R,
          typename C,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1, A2) >
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1, A2) const>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1, A2) volatile>
{
    typedef R type;
};

template <typename R,
          typename C,
          typename A0,
          typename A1,
          typename A2>
struct deduce_result_type<detail::unspecified_type,
                          R (C::*) (A0, A1, A2) const volatile>
{
    typedef R type;
};

// ====================================================================
// BindResult
// ====================================================================

// MemberPointerWrapper will wrap member pointers using mem_fn<>.
// The default case does nothing.
template <typename TType>
struct MemberPointerWrapper
{
    typedef TType type;

    static const TType& wrap(const TType& t)
    {
        return t;
    }

#if defined(WEOS_USE_CXX11)
    static TType&& wrap(TType&& t)
    {
        return static_cast<TType&&>(t);
    }
#endif // WEOS_USE_CXX11
};

// In the special case of a member pointer, mem_fn<> is applied.
template <typename TType, typename TClass>
struct MemberPointerWrapper<TType TClass::*>
{
    typedef MemFnResult<TType TClass::*> type;

    static type wrap(TType TClass::* pm)
    {
        return type(pm);
    }
};

// When the best overload for bind<>() is determined, the compiler
// instantiates MemberPointerWrapper<void>, which forms a reference
// to void in turn. A solution is to provide a template
// specialization for this case. It is never used, because there
// are better matches for bind<>.
template <>
struct MemberPointerWrapper<void>
{
    typedef void type;
};

// The result of a bind<>() call.
// The TSignature will be something of the form
// TFunctor(TBoundArg0, TBoundArg1, ...),
// where TFunctor can be a function pointer or a MemFnResult
template <typename TResult, typename TSignature>
struct BindResult;

template <typename TResult, typename F>
struct BindResult<TResult, F()>
{
    typedef TResult result_type;

    // The bound functor. To be used internally only.
    typedef F _functor_type_;

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
    BindResult(WEOS_RV_REF(BindResult) other)
        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),
          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))
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
    result_type operator() (WEOS_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (WEOS_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    //! \todo We can never have a member function pointer.
    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                  "The callable has not been wrapped.");

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0)
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor();
    }

    // Invoke function pointer and return void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0) const
    {
        // The bind expression returns void. Thus, ignore the return value
        // of the functor.
        m_functor();
    }

    // Invoke function pointer and return non-void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0)
    {
        return m_functor();
    }

    // Invoke function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value,
                dispatch_tag>::type* = 0) const
    {
        return m_functor();
    }

    WEOS_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0>
struct BindResult<TResult, F(A0)>
{
    typedef TResult result_type;

    // The bound functor. To be used internally only.
    typedef F _functor_type_;

    // Constructor with perfect forwarding
    template <typename T0>
    explicit BindResult(const F& f,
                        WEOS_FWD_REF(T0) t0)
        : m_functor(f),
          m_arguments(WEOS_NAMESPACE::forward<T0>(t0))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(WEOS_RV_REF(BindResult) other)
        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),
          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))
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
    result_type operator() (WEOS_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (WEOS_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    //! \todo We can never have a member function pointer.
    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                  "The callable has not been wrapped.");

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0)
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    // Invoke member function pointer and return non-void (const qualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)();
    }

    WEOS_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1>
struct BindResult<TResult, F(A0, A1)>
{
    typedef TResult result_type;

    // The bound functor. To be used internally only.
    typedef F _functor_type_;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1>
    explicit BindResult(const F& f,
                        WEOS_FWD_REF(T0) t0,
                        WEOS_FWD_REF(T1) t1)
        : m_functor(f),
          m_arguments(WEOS_NAMESPACE::forward<T0>(t0),
                      WEOS_NAMESPACE::forward<T1>(t1))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(WEOS_RV_REF(BindResult) other)
        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),
          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))
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
    result_type operator() (WEOS_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (WEOS_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    //! \todo We can never have a member function pointer.
    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                  "The callable has not been wrapped.");

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                dispatch_tag>::type* = 0) const
    {
        return (*unpack_argument<
                typename argument_tuple_element<0, arguments_type>::type>()(
                    get<0>(m_arguments), unbound_args).*m_functor)(
            unpack_argument<
                typename argument_tuple_element<1, arguments_type>::type>()(
                    get<1>(m_arguments), unbound_args));
    }

    WEOS_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1,
          typename A2>
struct BindResult<TResult, F(A0, A1, A2)>
{
    typedef TResult result_type;

    // The bound functor. To be used internally only.
    typedef F _functor_type_;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2>
    explicit BindResult(const F& f,
                        WEOS_FWD_REF(T0) t0,
                        WEOS_FWD_REF(T1) t1,
                        WEOS_FWD_REF(T2) t2)
        : m_functor(f),
          m_arguments(WEOS_NAMESPACE::forward<T0>(t0),
                      WEOS_NAMESPACE::forward<T1>(t1),
                      WEOS_NAMESPACE::forward<T2>(t2))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(WEOS_RV_REF(BindResult) other)
        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),
          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))
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
    result_type operator() (WEOS_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (WEOS_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1, A2> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    //! \todo We can never have a member function pointer.
    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                  "The callable has not been wrapped.");

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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

    WEOS_COPYABLE_AND_MOVABLE(BindResult)
};

template <typename TResult, typename F,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
struct BindResult<TResult, F(A0, A1, A2, A3)>
{
    typedef TResult result_type;

    // The bound functor. To be used internally only.
    typedef F _functor_type_;

    // Constructor with perfect forwarding
    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    explicit BindResult(const F& f,
                        WEOS_FWD_REF(T0) t0,
                        WEOS_FWD_REF(T1) t1,
                        WEOS_FWD_REF(T2) t2,
                        WEOS_FWD_REF(T3) t3)
        : m_functor(f),
          m_arguments(WEOS_NAMESPACE::forward<T0>(t0),
                      WEOS_NAMESPACE::forward<T1>(t1),
                      WEOS_NAMESPACE::forward<T2>(t2),
                      WEOS_NAMESPACE::forward<T3>(t3))
    {
    }

    // Copy construction
    BindResult(const BindResult& other)
        : m_functor(other.m_functor),
          m_arguments(other.m_arguments)
    {
    }

    // Move construction
    BindResult(WEOS_RV_REF(BindResult) other)
        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),
          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))
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
    result_type operator() (WEOS_FWD_REF(T0) t0)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0>
    result_type operator() (WEOS_FWD_REF(T0) t0) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3)
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    result_type operator() (WEOS_FWD_REF(T0) t0,
                            WEOS_FWD_REF(T1) t1,
                            WEOS_FWD_REF(T2) t2,
                            WEOS_FWD_REF(T3) t3) const
    {
        return this->invoke<result_type>(
                forward_as_argument_tuple(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3)));
    }

private:
    typedef argument_tuple<A0, A1, A2, A3> arguments_type;

    F m_functor;
    arguments_type m_arguments;

    struct dispatch_tag;

    //! \todo We can never have a member function pointer.
    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,
                  "The callable has not been wrapped.");

    // Invoke function pointer and return void (unqualified).
    template <typename TReturn, typename TUnbound>
    TReturn invoke(
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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
            WEOS_FWD_REF(TUnbound) unbound_args,
            typename WEOS_NAMESPACE::enable_if<
                !WEOS_NAMESPACE::is_same<TReturn, void>::value
                && WEOS_NAMESPACE::is_member_function_pointer<F>::value,
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

    WEOS_COPYABLE_AND_MOVABLE(BindResult)
};


struct bind_helper_null_type;

template <typename TResult,
          typename TCallable,
          typename A0 = bind_helper_null_type,
          typename A1 = bind_helper_null_type,
          typename A2 = bind_helper_null_type,
          typename A3 = bind_helper_null_type>
struct bind_helper
{
    // Deduce the result type.
    typedef typename deduce_result_type<TResult, TCallable>::type result_type;
    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.
    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;
    typedef typename wrapper_type::type functor_type;
    typedef BindResult<result_type,
                       functor_type(typename WEOS_NAMESPACE::decay<A0>::type,
                                    typename WEOS_NAMESPACE::decay<A1>::type,
                                    typename WEOS_NAMESPACE::decay<A2>::type,
                                    typename WEOS_NAMESPACE::decay<A3>::type)> type;
};

template <typename TResult,
          typename TCallable,
          typename A0,
          typename A1,
          typename A2>
struct bind_helper<TResult, TCallable,
                   A0,
                   A1,
                   A2,
                   bind_helper_null_type>
{
    // Deduce the result type.
    typedef typename deduce_result_type<TResult, TCallable>::type result_type;
    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.
    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;
    typedef typename wrapper_type::type functor_type;
    typedef BindResult<result_type,
                       functor_type(typename WEOS_NAMESPACE::decay<A0>::type,
                                    typename WEOS_NAMESPACE::decay<A1>::type,
                                    typename WEOS_NAMESPACE::decay<A2>::type)> type;
};

template <typename TResult,
          typename TCallable,
          typename A0,
          typename A1>
struct bind_helper<TResult, TCallable,
                   A0,
                   A1,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    // Deduce the result type.
    typedef typename deduce_result_type<TResult, TCallable>::type result_type;
    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.
    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;
    typedef typename wrapper_type::type functor_type;
    typedef BindResult<result_type,
                       functor_type(typename WEOS_NAMESPACE::decay<A0>::type,
                                    typename WEOS_NAMESPACE::decay<A1>::type)> type;
};

template <typename TResult,
          typename TCallable,
          typename A0>
struct bind_helper<TResult, TCallable,
                   A0,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    // Deduce the result type.
    typedef typename deduce_result_type<TResult, TCallable>::type result_type;
    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.
    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;
    typedef typename wrapper_type::type functor_type;
    typedef BindResult<result_type,
                       functor_type(typename WEOS_NAMESPACE::decay<A0>::type)> type;
};

template <typename TResult,
          typename TCallable>
struct bind_helper<TResult, TCallable,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type,
                   bind_helper_null_type>
{
    // Deduce the result type.
    typedef typename deduce_result_type<TResult, TCallable>::type result_type;
    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.
    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;
    typedef typename wrapper_type::type functor_type;
    typedef BindResult<result_type,
                       functor_type()> type;
};

} // namespace detail

// ====================================================================
// mem_fn<>
// ====================================================================

template <typename TResult, typename TClass>
inline
detail::MemFnResult<TResult TClass::*> mem_fn(TResult TClass::* pm) WEOS_NOEXCEPT
{
    return detail::MemFnResult<TResult TClass::*>(pm);
}

// ====================================================================
// bind<>
// ====================================================================

// template <typename F, typename... TArgs>
// /*unspecified*/ bind(F&& f, TArgs&&... args);
template <typename TCallable>
inline
typename detail::bind_helper<detail::unspecified_type,
                             TCallable>::type
bind(WEOS_FWD_REF(TCallable) f)
{
    typedef detail::bind_helper<detail::unspecified_type,
                                TCallable> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)));
}

template <typename TCallable,
          typename A0>
inline
typename detail::bind_helper<detail::unspecified_type,
                             TCallable,
                             A0>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0)
{
    typedef detail::bind_helper<detail::unspecified_type,
                                TCallable,
                                A0> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0));
}

template <typename TCallable,
          typename A0,
          typename A1>
inline
typename detail::bind_helper<detail::unspecified_type,
                             TCallable,
                             A0,
                             A1>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1)
{
    typedef detail::bind_helper<detail::unspecified_type,
                                TCallable,
                                A0,
                                A1> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1));
}

template <typename TCallable,
          typename A0,
          typename A1,
          typename A2>
inline
typename detail::bind_helper<detail::unspecified_type,
                             TCallable,
                             A0,
                             A1,
                             A2>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1,
     WEOS_FWD_REF(A2) a2)
{
    typedef detail::bind_helper<detail::unspecified_type,
                                TCallable,
                                A0,
                                A1,
                                A2> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1),
                      WEOS_NAMESPACE::forward<A2>(a2));
}

template <typename TCallable,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename detail::bind_helper<detail::unspecified_type,
                             TCallable,
                             A0,
                             A1,
                             A2,
                             A3>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1,
     WEOS_FWD_REF(A2) a2,
     WEOS_FWD_REF(A3) a3)
{
    typedef detail::bind_helper<detail::unspecified_type,
                                TCallable,
                                A0,
                                A1,
                                A2,
                                A3> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1),
                      WEOS_NAMESPACE::forward<A2>(a2),
                      WEOS_NAMESPACE::forward<A3>(a3));
}

// template <typename R, typename F, typename... TArgs>
// /*unspecified*/ bind(F&& f, TArgs&&... args);
template <typename TResult,
          typename TCallable>
inline
typename detail::bind_helper<TResult,
                             TCallable>::type
bind(WEOS_FWD_REF(TCallable) f)
{
    typedef detail::bind_helper<TResult,
                                TCallable> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)));
}

template <typename TResult,
          typename TCallable,
          typename A0>
inline
typename detail::bind_helper<TResult,
                             TCallable,
                             A0>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0)
{
    typedef detail::bind_helper<TResult,
                                TCallable,
                                A0> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0));
}

template <typename TResult,
          typename TCallable,
          typename A0,
          typename A1>
inline
typename detail::bind_helper<TResult,
                             TCallable,
                             A0,
                             A1>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1)
{
    typedef detail::bind_helper<TResult,
                                TCallable,
                                A0,
                                A1> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1));
}

template <typename TResult,
          typename TCallable,
          typename A0,
          typename A1,
          typename A2>
inline
typename detail::bind_helper<TResult,
                             TCallable,
                             A0,
                             A1,
                             A2>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1,
     WEOS_FWD_REF(A2) a2)
{
    typedef detail::bind_helper<TResult,
                                TCallable,
                                A0,
                                A1,
                                A2> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1),
                      WEOS_NAMESPACE::forward<A2>(a2));
}

template <typename TResult,
          typename TCallable,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
inline
typename detail::bind_helper<TResult,
                             TCallable,
                             A0,
                             A1,
                             A2,
                             A3>::type
bind(WEOS_FWD_REF(TCallable) f,
     WEOS_FWD_REF(A0) a0,
     WEOS_FWD_REF(A1) a1,
     WEOS_FWD_REF(A2) a2,
     WEOS_FWD_REF(A3) a3)
{
    typedef detail::bind_helper<TResult,
                                TCallable,
                                A0,
                                A1,
                                A2,
                                A3> helper_type;
    typedef typename helper_type::wrapper_type wrapper_type;
    typedef typename helper_type::type bind_result_type;

    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f)),
                      WEOS_NAMESPACE::forward<A0>(a0),
                      WEOS_NAMESPACE::forward<A1>(a1),
                      WEOS_NAMESPACE::forward<A2>(a2),
                      WEOS_NAMESPACE::forward<A3>(a3));
}

// ====================================================================
// function<>
// ====================================================================

namespace detail
{

class AnonymousClass;

// A small functor.
// This type is just large enough to hold a member function pointer
// plus a pointer to an instance or a function pointer plus an
// argument of pointer size.
struct SmallFunctor
{
    union Callable
    {
        void* objectPointer;
        const void* constObjectPointer;
        void (*functionPointer)();
        void* AnonymousClass::*memberDataPointer;
        void (AnonymousClass::*memberFunctionPointer)();
    };
    union Argument
    {
        void* objectPointer;
        const void* constObjectPointer;
    };

    Callable callable;
    Argument argument;
};
struct SmallFunctorStorage
{
    void* get() { return &data; }
    const void* get() const { return &data; }

    template <typename T>
    T& get() { return *static_cast<T*>(get()); }

    template <typename T>
    const T& get() const { return *static_cast<const T*>(get()); }

    SmallFunctor data;
};

// Adapters for the implementation of the polymorphic function<>.
// Required members:
// static bool isEmpty(const F& f)
//     ... checks if f is empty
// static void init(SmallFunctorStorage& self, F&& f)
//     ... inits self from f
// static void manage(AdapterTask task, SmallFunctorStorage& self,
//                    const SmallFunctorStorage* other)
//     ... clones *other into self   if task == AdapterTask_Clone
//     ... destroys self             if task == AdapterTask_Destroy
// template <typename... TArgs>
// static /*undefined*/ invoke(const SmallFunctorStorage& self, TArgs... args)
//     ... invokes self with args


enum AdapterTask
{
    AdapterTask_Clone,
    AdapterTask_Destroy
};

typedef void (*manager_function)(AdapterTask task, SmallFunctorStorage& self, const SmallFunctorStorage* other);

// An adapter which allows to use a function pointer in function<>.
template <typename TSignature>
struct FunctionPointerAdapter
{
};

// An adapter which allows to use a bind expression in function<>.
template <typename TBindResult>
class BindAdapter
{
    static const std::size_t smallSize = sizeof(SmallFunctorStorage);
    static const std::size_t smallAlign = alignment_of<SmallFunctorStorage>::value;

    typedef typename TBindResult::_functor_type_ functor_type;
    static const bool can_store_inplace =
           sizeof(TBindResult) <= smallSize
        && alignment_of<TBindResult>::value <= smallAlign
        && (smallAlign % alignment_of<TBindResult>::value == 0);
    typedef integral_constant<bool, can_store_inplace> store_inplace;

    static void doInit(SmallFunctorStorage& self, const TBindResult& f, true_type)
    {
        new (self.get()) TBindResult(f);
    }

    static void doInit(SmallFunctorStorage& self, const TBindResult& f, false_type)
    {
        self.get<TBindResult*>() = new TBindResult(f);
    }

    // Clone a bind result which fits into the small functor storage.
    static void doClone(SmallFunctorStorage& self, const SmallFunctorStorage& other, true_type)
    {
        new (self.get()) TBindResult(other.get<TBindResult>());
    }

    // Clone a bind result which does not fit into the small functor storage.
    static void doClone(SmallFunctorStorage& self, const SmallFunctorStorage& other, false_type)
    {
        self.get<TBindResult*>() = new TBindResult(*other.get<TBindResult*>());
    }

    // Destroy a bind result which fits into the SFS.
    static void doDestroy(SmallFunctorStorage& self, true_type)
    {
        self.get<TBindResult>().~TBindResult();
    }

    // Destroy a bind result which doesn't fit into the SFS.
    static void doDestroy(SmallFunctorStorage& self, false_type)
    {
        delete self.get<TBindResult*>();
    }

public:
    static bool isEmpty(const TBindResult&)
    {
        return false;
    }

    static void init(SmallFunctorStorage& self, const TBindResult& f)
    {
        doInit(self, f, store_inplace());
    }

    static void manage(AdapterTask task, SmallFunctorStorage& self, const SmallFunctorStorage* other)
    {
        switch (task)
        {
        case AdapterTask_Clone:
            doClone(self, *other, store_inplace());
        case AdapterTask_Destroy:
            doDestroy(self, store_inplace());
        }
    }

    static typename TBindResult::result_type invoke(
            const SmallFunctorStorage& self)
    {
        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();
        return (*const_cast<TBindResult*>(functor))();
    }

    template <typename T0>
    static typename TBindResult::result_type invoke(
            const SmallFunctorStorage& self,
            T0 t0)
    {
        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();
        return (*const_cast<TBindResult*>(functor))(
                    WEOS_NAMESPACE::forward<T0>(t0));
    }

    template <typename T0,
              typename T1>
    static typename TBindResult::result_type invoke(
            const SmallFunctorStorage& self,
            T0 t0,
            T1 t1)
    {
        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();
        return (*const_cast<TBindResult*>(functor))(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1));
    }

    template <typename T0,
              typename T1,
              typename T2>
    static typename TBindResult::result_type invoke(
            const SmallFunctorStorage& self,
            T0 t0,
            T1 t1,
            T2 t2)
    {
        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();
        return (*const_cast<TBindResult*>(functor))(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2));
    }

    template <typename T0,
              typename T1,
              typename T2,
              typename T3>
    static typename TBindResult::result_type invoke(
            const SmallFunctorStorage& self,
            T0 t0,
            T1 t1,
            T2 t2,
            T3 t3)
    {
        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();
        return (*const_cast<TBindResult*>(functor))(
                    WEOS_NAMESPACE::forward<T0>(t0),
                    WEOS_NAMESPACE::forward<T1>(t1),
                    WEOS_NAMESPACE::forward<T2>(t2),
                    WEOS_NAMESPACE::forward<T3>(t3));
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

    /*
    template <typename TCallable>
    function(TCallable f)
    {
    }

    */

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
                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);
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
        adapter::init(m_storage, expr);
        m_manager = &adapter::manage;
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
    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&);

    detail::SmallFunctorStorage m_storage;
    detail::manager_function m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
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

    /*
    template <typename TCallable>
    function(TCallable f)
    {
    }

    */

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
                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);
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
        adapter::init(m_storage, expr);
        m_manager = &adapter::manage;
        m_invoker = &adapter::template invoke<A0>;
        return *this;
    }

    result_type operator() (A0 a0) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            WEOS_NAMESPACE::forward<A0>(a0));
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
    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&,
                                        A0);

    detail::SmallFunctorStorage m_storage;
    detail::manager_function m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
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

    /*
    template <typename TCallable>
    function(TCallable f)
    {
    }

    */

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
                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);
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
        adapter::init(m_storage, expr);
        m_manager = &adapter::manage;
        m_invoker = &adapter::template invoke<A0,
                                              A1>;
        return *this;
    }

    result_type operator() (A0 a0,
                            A1 a1) const
    {
        WEOS_ASSERT(m_invoker);
        return (*m_invoker)(m_storage,
                            WEOS_NAMESPACE::forward<A0>(a0),
                            WEOS_NAMESPACE::forward<A1>(a1));
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
    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&,
                                        A0,
                                        A1);

    detail::SmallFunctorStorage m_storage;
    detail::manager_function m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
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

    /*
    template <typename TCallable>
    function(TCallable f)
    {
    }

    */

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
                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);
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
        adapter::init(m_storage, expr);
        m_manager = &adapter::manage;
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
                            WEOS_NAMESPACE::forward<A0>(a0),
                            WEOS_NAMESPACE::forward<A1>(a1),
                            WEOS_NAMESPACE::forward<A2>(a2));
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
    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&,
                                        A0,
                                        A1,
                                        A2);

    detail::SmallFunctorStorage m_storage;
    detail::manager_function m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
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

    /*
    template <typename TCallable>
    function(TCallable f)
    {
    }

    */

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
                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);
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
        adapter::init(m_storage, expr);
        m_manager = &adapter::manage;
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
                            WEOS_NAMESPACE::forward<A0>(a0),
                            WEOS_NAMESPACE::forward<A1>(a1),
                            WEOS_NAMESPACE::forward<A2>(a2),
                            WEOS_NAMESPACE::forward<A3>(a3));
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
    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&,
                                        A0,
                                        A1,
                                        A2,
                                        A3);

    detail::SmallFunctorStorage m_storage;
    detail::manager_function m_manager;
    invoker_type m_invoker;

    void release()
    {
        if (m_invoker)
        {
            m_manager(detail::AdapterTask_Destroy, m_storage, 0);
            m_invoker = 0;
        }
    }
};

// ====================================================================
// static_function<>
// ====================================================================

template <typename TSignature,
          std::size_t TStorageSize = WEOS_DEFAULT_STATIC_FUNCTION_SIZE>
class static_function;

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_FUNCTIONAL_HPP

