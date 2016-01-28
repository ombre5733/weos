/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#ifndef WEOS_COMMON_TUPLE_HPP
#define WEOS_COMMON_TUPLE_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../type_traits.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

//! A tuple.
template <typename... TTypes>
class tuple;

//! Retrieves the type of the \p TIndex-th element in the tuple type \p TTuple.
template <std::size_t TIndex, typename TTuple>
class tuple_element;

//! Returns the size of a tuple.
template <typename TTuple>
struct tuple_size;

template <std::size_t I, typename... T>
WEOS_FORCE_INLINE
constexpr typename tuple_element<I, tuple<T...>>::type& get(tuple<T...>&) noexcept;

template <std::size_t I, typename... T>
WEOS_FORCE_INLINE
constexpr typename tuple_element<I, tuple<T...>>::type&& get(tuple<T...>&&) noexcept;

template <std::size_t I, typename... T>
WEOS_FORCE_INLINE
constexpr const typename tuple_element<I, tuple<T...>>::type& get(const tuple<T...>&) noexcept;

namespace weos_detail
{

using std::size_t;
using std::swap;

// ----=====================================================================----
//     TupleIndices
// ----=====================================================================----

template <size_t... TIndices>
struct TupleIndices
{
};

template <size_t TStart, size_t TEnd, typename TTemporary>
struct make_tuple_indices_impl;

template <size_t TStart, size_t TEnd, size_t... TIndices>
struct make_tuple_indices_impl<TStart, TEnd, TupleIndices<TIndices...>>
{
    typedef typename make_tuple_indices_impl<
                TStart + 1,
                TEnd,
                TupleIndices<TIndices..., TStart>>::type
        type;
};

// Specialization for TStart == TEnd.
template <size_t TStart, size_t... TIndices>
struct make_tuple_indices_impl<TStart, TStart, TupleIndices<TIndices...>>
{
    typedef TupleIndices<TIndices...> type;
};

template <size_t TEnd, size_t TStart = 0>
struct make_tuple_indices
{
    static_assert(TStart <= TEnd, "make_tuple_indices: invalid range");
    typedef typename make_tuple_indices_impl<TStart, TEnd, TupleIndices<>>::type
        type;
};

// ----=====================================================================----
//     TupleTypes
// ----=====================================================================----

template <typename... TTypes>
struct TupleTypes
{
};

template <typename TTuple, size_t TStart, size_t TEnd, typename TTemporary>
struct make_tuple_types_impl;

template <typename TTuple, size_t TStart, size_t TEnd, typename... TTypes>
struct make_tuple_types_impl<TTuple, TStart, TEnd, TupleTypes<TTypes...>>
{
    typedef typename remove_reference<TTuple>::type plain_tuple;
    typedef typename conditional<is_lvalue_reference<TTuple>::value,
                                 typename tuple_element<TStart, plain_tuple>::type&,
                                 typename tuple_element<TStart, plain_tuple>::type>::type
        next_type;

    typedef typename make_tuple_types_impl<TTuple, TStart + 1, TEnd,
                                           TupleTypes<TTypes..., next_type>>::type
        type;
};

// Specialization for TStart == TEnd.
template <typename TTuple, size_t TStart, typename... TTypes>
struct make_tuple_types_impl<TTuple, TStart, TStart, TupleTypes<TTypes...>>
{
    typedef TupleTypes<TTypes...> type;
};

template <typename TTuple,
          size_t TEnd = tuple_size<typename remove_reference<TTuple>::type>::value,
          size_t TStart = 0>
struct make_tuple_types
{
    static_assert(TStart <= TEnd, "make_tuple_types: invalid range");
    typedef typename make_tuple_types_impl<TTuple,
                                           TStart, TEnd,
                                           TupleTypes<>>::type
        type;
};

// ----=====================================================================----
//     IndexedTupleElement
// ----=====================================================================----

// Stores one element of a tuple. The index of the tuple element is held as
// template parameter. If TType is empty and not final, we can apply the
// empty-base class optimization.
template <size_t TIndex, typename TType,
          bool TIsEmptyAndNotFinal = is_empty<TType>::value>
struct IndexedTupleElement;

// Swap two IndexTupleElement objects.
template <size_t TIndex, typename TType, bool TIsEmptyAndNotFinal>
WEOS_FORCE_INLINE
void swap(IndexedTupleElement<TIndex, TType, TIsEmptyAndNotFinal>& a,
          IndexedTupleElement<TIndex, TType, TIsEmptyAndNotFinal>& b) // TODO: noexcept
{
    swap(a.get(), b.get());
}

template <size_t TIndex, typename TType>
struct IndexedTupleElement<TIndex, TType, true> : private TType
{
    // Creates a tuple element by value-initializing the underlying type.
    constexpr
    IndexedTupleElement()
        : TType()
    {
    }

    // Constructor using perfect forwarding from a value u.
    //
    // Note: We do not want to have an
    //     IndexedTupleElement(IndexedTupleElement&&)
    // because it would match
    //     IndexedTupleElement(IndexedTupleElement&)
    // and interfere with the copy-constructor
    //     IndexedTupleElement(const IndexedTupleElement&)
    // See here:
    // http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
    template <typename UType,
              typename _ = typename enable_if<!is_base_of<IndexedTupleElement,
                                                          typename decay<UType>::type>::value
                                              && is_constructible<TType, UType>::value>::type>
    constexpr
    IndexedTupleElement(UType&& u)
        : TType(WEOS_NAMESPACE::forward<UType>(u))
    {
    }

    // Copy-constructs a tuple element.
    IndexedTupleElement(const IndexedTupleElement& other) = default;

    // Move-constructs a tuple element.
    IndexedTupleElement(IndexedTupleElement&& other) // TODO: noexcept
        : TType(WEOS_NAMESPACE::move(other))
    {
    }

    IndexedTupleElement& operator=(const IndexedTupleElement&) = delete;

    template <typename T>
    IndexedTupleElement& operator=(T&& t)
    {
        TType::operator=(WEOS_NAMESPACE::forward<T>(t));
        return *this;
    }

    // Accesses the element.
    WEOS_CONSTEXPR_FROM_CXX14
    TType& get() noexcept
    {
        return static_cast<TType&>(*this);
    }

    // Accesses the element.
    WEOS_CONSTEXPR_FROM_CXX14
    const TType& get() const noexcept
    {
        return static_cast<const TType&>(*this);
    }

    // Swaps this element with the \p other element. The function cannot
    // be void because we want to use variadicCall() on it.
    int swap(IndexedTupleElement& other) // noexcept if TType is noexcept-swappable
    {
        WEOS_NAMESPACE::weos_detail::swap(*this, other);
        return 0;
    }
};

template <size_t TIndex, typename TType>
struct IndexedTupleElement<TIndex, TType, false>
{
    // Creates a tuple element by value-initializing the underlying type.
    constexpr
    IndexedTupleElement()
        : m_value()
    {
        static_assert(!is_reference<TType>::value,
                      "Cannot default-construct a reference.");
    }

    // Constructor using perfect forwarding from a value u.
    template <typename UType,
              typename _ = typename enable_if<!is_base_of<IndexedTupleElement,
                                                          typename decay<UType>::type>::value
                                              && is_constructible<TType, UType>::value>::type>
    constexpr
    IndexedTupleElement(UType&& u)
        : m_value(WEOS_NAMESPACE::forward<UType>(u))
    {
    }

    // Copy-constructs a tuple element.
    IndexedTupleElement(const IndexedTupleElement& other) = default;

    // Move-constructs a tuple element.
    IndexedTupleElement(IndexedTupleElement&& other)
        : m_value(WEOS_NAMESPACE::forward<TType>(other.m_value))
    {
    }

    IndexedTupleElement& operator=(const IndexedTupleElement&) = delete;

    template <typename T>
    IndexedTupleElement& operator=(T&& t)
    {
        m_value = WEOS_NAMESPACE::forward<T>(t);
        return *this;
    }

    // Accesses the element.
    WEOS_CONSTEXPR_FROM_CXX14
    TType& get() noexcept
    {
        return m_value;
    }

    // Accesses the element.
    WEOS_CONSTEXPR_FROM_CXX14
    const TType& get() const noexcept
    {
        return m_value;
    }

    // Swaps this element with the \p other element. The function cannot
    // be void because we want to use variadicCall() on it.
    int swap(IndexedTupleElement& other) // noexcept if TType is noexcept-swappable
    {
        WEOS_NAMESPACE::weos_detail::swap(*this, other);
        return 0;
    }

private:
    TType m_value;
};

// ----=====================================================================----
//     TupleImpl
// ----=====================================================================----

template <typename... T>
void variadicCall(T&&...) noexcept
{
}

template <typename TIndices, typename... TTypes>
struct TupleImpl;

template <size_t... TIndices, typename... TTypes>
struct TupleImpl<TupleIndices<TIndices...>, TTypes...>
        : public IndexedTupleElement<TIndices, TTypes>...
{
public:
    constexpr
    TupleImpl()
    {
    }

    // Constructs a tuple.
    // The elements with indices in TFirstIndices are constructed by forwarding
    // the parameters firstValues. The elements with indices TLastIndices
    // are constructed by value-initialization.
    template <size_t... TFirstIndices, typename... TFirstTypes,
              size_t... TLastIndices, typename... TLastTypes,
              typename... UTypes>
    explicit
    TupleImpl(TupleIndices<TFirstIndices...>, TupleTypes<TFirstTypes...>,
              TupleIndices<TLastIndices...>, TupleTypes<TLastTypes...>,
              UTypes&&... firstValues)
        : IndexedTupleElement<TFirstIndices, TFirstTypes>(forward<UTypes>(firstValues))...,
          IndexedTupleElement<TLastIndices, TLastTypes>()...
    {
    }

    // Copy-constructs from another TupleImpl.
    TupleImpl(const TupleImpl&) = default;

    // Move-constructs from another TupleImpl.
    TupleImpl(TupleImpl&& other)
        : IndexedTupleElement<TIndices, TTypes>(
              WEOS_NAMESPACE::forward<TTypes>(
                  static_cast<IndexedTupleElement<TIndices, TTypes>&>(other).get()))...
    {
    }

    // Move-constructs from another tuple.
    template <typename TTuple>
    constexpr
    TupleImpl(TTuple&& t)
        : IndexedTupleElement<TIndices, TTypes>(
              WEOS_NAMESPACE::forward<typename tuple_element<
                  TIndices, typename make_tuple_types<TTuple>::type>::type>(
                      WEOS_NAMESPACE::get<TIndices>(t)))...
    {
    }

    // Copy-assigns the other TupleImpl to this one.
    TupleImpl& operator=(const TupleImpl& other)
    {
        variadicCall(IndexedTupleElement<TIndices, TTypes>::operator=(
            static_cast<const IndexedTupleElement<TIndices, TTypes>&>(other).get())...);
        return *this;
    }

    // Move-assigns the other TupleImpl to this one.
    TupleImpl& operator=(TupleImpl&& other)
    {
        variadicCall(IndexedTupleElement<TIndices, TTypes>::operator=(
            WEOS_NAMESPACE::forward<TTypes>(
                static_cast<IndexedTupleElement<TIndices, TTypes>&>(other).get()))...);
        return *this;
    }

    // Move-assigns from another tuple.
    template <typename TTuple>
    TupleImpl& operator=(TTuple&& t)
    {
        variadicCall(IndexedTupleElement<TIndices, TTypes>::operator=(
            WEOS_NAMESPACE::forward<typename tuple_element<
                TIndices, typename make_tuple_types<TTuple>::type>::type>(
                    WEOS_NAMESPACE::get<TIndices>(t)))...);
        return *this;
    }

    // Swaps this tuple with the other tuple.
    void swap(TupleImpl& other) /* noexcept if TFirst noexcept swappable && rest_type noexcept swappable */
    {
        variadicCall(IndexedTupleElement<TIndices, TTypes>::swap(
                         static_cast<IndexedTupleElement<TIndices, TTypes>&>(other))...);
    }
};

template <typename... T>
struct all : WEOS_NAMESPACE::true_type {};

template <typename H, typename... T>
struct all<H, T...> : WEOS_NAMESPACE::conditional<H::value, all<T...>, WEOS_NAMESPACE::false_type>::type
{
};

template <typename... T>
struct any : WEOS_NAMESPACE::false_type {};

template <typename H, typename... T>
struct any<H, T...> : WEOS_NAMESPACE::conditional<H::value, WEOS_NAMESPACE::true_type, any<T...>>::type
{
};

} // namespace weos_detail

template <typename... TTypes>
class tuple
{
    typedef weos_detail::TupleImpl<
        typename weos_detail::make_tuple_indices<sizeof...(TTypes)>::type, TTypes...>
        implementation_type;
    implementation_type m_impl;


    template <std::size_t I, typename... T>
    friend constexpr typename tuple_element<I, tuple<T...>>::type& get(tuple<T...>&) noexcept;

    template <std::size_t I, typename... T>
    friend constexpr typename tuple_element<I, tuple<T...>>::type&& get(tuple<T...>&&) noexcept;

    template <std::size_t I, typename... T>
    friend constexpr const typename tuple_element<I, tuple<T...>>::type& get(const tuple<T...>&) noexcept;


public:
    //! Constructs a tuple by value-initializing its elements.
    // TODO: ARMCC does not like this
    //template <typename TT = true_type,
    //          typename = typename enable_if<weos_detail::all<TT, is_default_constructible<TTypes>...>::value>::type>
    constexpr
    tuple() noexcept(weos_detail::all<is_nothrow_default_constructible<TTypes>...>::value)
        : m_impl()
    {
    }

    //! Constructs a tuple by initializing the elements from the given \p args.
    explicit constexpr
    tuple(const TTypes&... args) noexcept(weos_detail::all<is_nothrow_copy_constructible<TTypes>...>::value)
        : m_impl(typename weos_detail::make_tuple_indices<sizeof...(TTypes)>::type(),
                 typename weos_detail::make_tuple_types<tuple, sizeof...(TTypes)>::type(),
                 typename weos_detail::make_tuple_indices<0>::type(),
                 typename weos_detail::make_tuple_types<tuple, 0>::type(),
                 args...)
    {
    }

    // TODO:
    // template <typename... UTypes>
    // constexpr tuple(UTypes&&... args) (without explicit)

    //! Constructs a tuple by perfect forwarding of the given \p args.
    template <typename... UTypes> // TODO: enable if convertible
    explicit constexpr
    tuple(UTypes&&... args)
        : m_impl(typename weos_detail::make_tuple_indices<sizeof...(UTypes)>::type(),
                 typename weos_detail::make_tuple_types<tuple, sizeof...(UTypes)>::type(),
                 typename weos_detail::make_tuple_indices<sizeof...(TTypes), sizeof...(UTypes)>::type(),
                 typename weos_detail::make_tuple_types<tuple, sizeof...(TTypes), sizeof...(UTypes)>::type(),
                 forward<UTypes>(args)...)
    {
    }

    //! Copy-constructs a tuple by copying from the \p other tuple.
    // TODO
    template <typename... UTypes>
    constexpr
    tuple(const tuple<UTypes...>& other);

    //! Move-constructs a tuple by moving from the \p other tuple.
    // TODO
    template <typename... UTypes>
    constexpr
    tuple(tuple<UTypes...>&& other);

    //! Copy-constructs a tuple from the \p other tuple.
    tuple(const tuple& other) = default;

    //! Move-constructs a tuple by moving from the \p other tuple.
    tuple(tuple&& other) = default;

    // TODO: constructors with allocator

    // TODO: assignment

    //! Swaps this tuple with the \p other tuple.
    void swap(tuple& other) noexcept(noexcept(m_impl.swap(m_impl)))
    {
        m_impl.swap(other.m_impl);
    }
};

// Tuple with zero elements.
template <>
class tuple<>
{
public:
    constexpr
    tuple() noexcept
    {
    }

    void swap(tuple&) noexcept
    {
    }
};

// TODO: Partial specialization for a pair-like (2-element) tuple.

// ----=====================================================================----
//     tuple_element
// ----=====================================================================----

// Specialization when TIndex is out of bounds.
template <std::size_t TIndex>
struct tuple_element<TIndex, weos_detail::TupleTypes<>>
{
    static_assert(TIndex != TIndex, "The element index is out of bounds.");
};

// Specialization for TIndex == 0.
template <typename TFirst, typename... TRest>
struct tuple_element<0, weos_detail::TupleTypes<TFirst, TRest...>>
{
    typedef TFirst type;
};

// General case for accessing the \p TIndex-th type.
template <std::size_t TIndex, typename TFirst, typename... TRest>
struct tuple_element<TIndex, weos_detail::TupleTypes<TFirst, TRest...>>
{
    typedef typename tuple_element<TIndex - 1,
                                   weos_detail::TupleTypes<TRest...>>::type type;
};

template <std::size_t TIndex, typename... TTypes>
struct tuple_element<TIndex, tuple<TTypes...>>
{
    typedef typename tuple_element<TIndex, weos_detail::TupleTypes<TTypes...>>::type
        type;
};

//! Returns the \p TIndex-th type of a const tuple.
template <std::size_t TIndex, typename TTuple>
struct tuple_element<TIndex, const TTuple>
{
    typedef typename add_const<
                typename tuple_element<TIndex, TTuple>::type>::type type;
};

//! Returns the \p TIndex-th type of a volatile tuple.
template <std::size_t TIndex, typename TTuple>
struct tuple_element<TIndex, volatile TTuple>
{
    typedef typename add_volatile<
                typename tuple_element<TIndex, TTuple>::type>::type type;
};

//! Returns the \p TIndex-th type of a const volatile tuple.
template <std::size_t TIndex, class TTuple>
struct tuple_element<TIndex, const volatile TTuple>
{
    typedef typename add_cv<
                typename tuple_element<TIndex, TTuple>::type>::type type;
};

// ----=====================================================================----
//     tuple_size
// ----=====================================================================----

template <typename... TTypes>
struct tuple_size<tuple<TTypes...>> : public integral_constant<std::size_t, sizeof...(TTypes)>
{
};

//! Returns the size of a const tuple.
template <typename TTuple>
struct tuple_size<const TTuple> : public integral_constant<std::size_t, tuple_size<TTuple>::value>
{
};

//! Returns the size of a volatile tuple.
template <typename TTuple>
struct tuple_size<volatile TTuple> : public integral_constant<std::size_t, tuple_size<TTuple>::value>
{
};

//! Returns the size of a const volatile tuple.
template <typename TTuple>
struct tuple_size<const volatile TTuple> : public integral_constant<std::size_t, tuple_size<TTuple>::value>
{
};

// ----=====================================================================----
//     get()
// ----=====================================================================----

template <std::size_t TIndex, typename... TTypes>
WEOS_FORCE_INLINE
constexpr typename tuple_element<TIndex, tuple<TTypes...>>::type& get(tuple<TTypes...>& t) noexcept
{
    typedef typename tuple_element<TIndex, tuple<TTypes...>>::type type;
    return static_cast<weos_detail::IndexedTupleElement<TIndex, type>&>(t.m_impl).get();
}

template <std::size_t TIndex, typename... TTypes>
WEOS_FORCE_INLINE
constexpr typename tuple_element<TIndex, tuple<TTypes...>>::type&& get(tuple<TTypes...>&& t) noexcept
{
    typedef typename tuple_element<TIndex, tuple<TTypes...>>::type type;
    return static_cast<type&&>(
                static_cast<weos_detail::IndexedTupleElement<TIndex, type>&&>(t.m_impl).get());
}

template <std::size_t TIndex, typename... TTypes>
WEOS_FORCE_INLINE
constexpr const typename tuple_element<TIndex, tuple<TTypes...>>::type& get(const tuple<TTypes...>& t) noexcept
{
    typedef typename tuple_element<TIndex, tuple<TTypes...>>::type type;
    return static_cast<const weos_detail::IndexedTupleElement<TIndex, type>&>(t.m_impl).get();
}

// ----=====================================================================----
//     forward_as_tuple()
// ----=====================================================================----

//! Forwards the given \p args in a tuple. The tuple must be consumed before
//! the next sequence point.
template <typename... TTypes>
WEOS_FORCE_INLINE
constexpr tuple<TTypes&&...> forward_as_tuple(TTypes&&... args) noexcept
{
    return tuple<TTypes&&...>(WEOS_NAMESPACE::forward<TTypes>(args)...);
}

// ----=====================================================================----
//     swap()
// ----=====================================================================----

//! Swaps two tuples \p a and \p b.
template <typename... TTypes> // TODO: enable_if all swappable & noexcept
void swap(tuple<TTypes...>& a, tuple<TTypes...>& b)
{
    a.swap(b);
}

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_TUPLE_HPP
