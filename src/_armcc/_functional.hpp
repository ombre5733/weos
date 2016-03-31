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

#ifndef WEOS_ARMCC_FUNCTIONAL_HPP
#define WEOS_ARMCC_FUNCTIONAL_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../_config.hpp"

#include "../exception.hpp"
#include "../memory.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"
#include "../_common/_alloc.hpp"
#include "../_common/_invoke.hpp"

#include <new>
#ifndef WEOS_NO_FUNCTION_TARGET
#include <typeinfo>
#endif // WEOS_NO_FUNCTION_TARGET


namespace std
{

struct bad_function_call : public std::exception
{
};

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
struct is_placeholder : integral_constant<int, 0>
{
};

template <int TIndex>
struct is_placeholder<placeholders::placeholder<TIndex>>
        : integral_constant<int, TIndex>
{
};

template <typename T>
struct weos_is_placeholder : false_type
{
};

template <int I>
struct weos_is_placeholder<placeholders::placeholder<I>> : true_type
{
    static constexpr int index = I - 1;
};

// ----=====================================================================----
//     WeakResultType
// ----=====================================================================----

namespace weos_detail
{

// Checks if T has a member named 'result_type', i.e. if T::result_type exists.
template <typename T>
class has_result_type_member
{
    typedef char yes;
    struct no { char _[2]; };

    template <typename U>
    static yes test(typename U::result_type* = 0);

    template <typename U>
    static no test(...);

public:
    static const bool value = sizeof(test<T>(0)) == 1;
};

template <typename T, bool THasResultType = has_result_type_member<T>::value>
struct WeakResultTypeHelper
{
    typedef typename T::result_type result_type;
};

template <typename T>
struct WeakResultTypeHelper<T, false>
{
};

// Function object
template <typename T>
struct WeakResultType : public WeakResultTypeHelper<T>
{
};

// Function
template <typename R, typename... TArgs>
struct WeakResultType<R (TArgs...)>
{
    typedef R result_type;
};

// Function reference
template <typename R, typename... TArgs>
struct WeakResultType<R (&)(TArgs...)>
{
    typedef R result_type;
};

// Function pointer
template <typename R, typename... TArgs>
struct WeakResultType<R (*)(TArgs...)>
{
    typedef R result_type;
};

// Member function pointer
template <typename R, typename C, typename... TArgs>
struct WeakResultType<R (C::*) (TArgs...) >
{
    typedef R result_type;
};

// Member function pointer (const)
template <typename R, typename C, typename... TArgs>
struct WeakResultType<R (C::*) (TArgs...) const>
{
    typedef R result_type;
};

// Member function pointer (volatile)
template <typename R, typename C, typename... TArgs>
struct WeakResultType<R (C::*) (TArgs...) volatile>
{
    typedef R result_type;
};

// Member function pointer (const volatile)
template <typename R, typename C, typename... TArgs>
struct WeakResultType<R (C::*) (TArgs...) const volatile>
{
    typedef R result_type;
};

} // namespace weos_detail

// ----=====================================================================----
//     reference_wrapper
// ----=====================================================================----

template <typename T>
class reference_wrapper : public weos_detail::WeakResultType<T>
{
public:
    typedef T type;

    reference_wrapper(T& t) noexcept
        : m_wrapped(addressof(t))
    {
    }

    reference_wrapper(T&&) = delete;

    reference_wrapper(const reference_wrapper&) = default;
    reference_wrapper& operator=(const reference_wrapper&) noexcept = default;

    operator T& () const noexcept
    {
        return *m_wrapped;
    }

    T& get() const noexcept
    {
        return *m_wrapped;
    }

    template <typename... TArgs>
    typename result_of<T&(TArgs&&...)>::type operator()(TArgs&&... args) const
    {
        return WEOS_NAMESPACE::weos_detail::invoke(*m_wrapped, std::forward<TArgs>(args)...);
    }

private:
    T* m_wrapped;
};

//! Create a reference wrapper from a reference \p t.
template <typename T>
reference_wrapper<T> ref(T& t) noexcept
{
    return reference_wrapper<T>(t);
}

//! Create a reference wrapper from a reference wrapper \p w.
template <typename T>
reference_wrapper<T> ref(reference_wrapper<T> w) noexcept
{
    return w;
}

template <typename T>
void ref(const T&& t) = delete;

//! Create a const-reference wrapper from a const-reference \p t.
template <typename T>
reference_wrapper<const T> cref(const T& t) noexcept
{
    return reference_wrapper<const T>(t);
}

//! Create a const-reference wrapper from a reference wrapper \p w.
template <typename T>
reference_wrapper<const T> cref(reference_wrapper<T> w) noexcept
{
    return reference_wrapper<const T>(w.get());
}

template <typename T>
void cref(const T&& t) = delete;

template <int TIndex>
struct weos_is_placeholder<const placeholders::placeholder<TIndex>> : true_type
{
    static const std::size_t index = TIndex - 1;
};

namespace weos_detail
{

template <typename T>
struct is_reference_wrapper : false_type
{
};

template <typename T>
struct is_reference_wrapper<reference_wrapper<T>> : true_type
{
};

} // namespace weos_detail

// ----=====================================================================----
//     MemFnResult
// ----=====================================================================----

namespace weos_detail
{

template <typename TMemberPointer>
class MemFnResult : public WeakResultType<TMemberPointer>
{
public:
    MemFnResult(TMemberPointer pm) noexcept
        : m_pm(pm)
    {
    }

    template <typename... TArgs>
    typename WEOS_NAMESPACE::weos_detail::invoke_result_type<TMemberPointer, TArgs...>::type
    operator()(TArgs&&... args) const
    {
        return WEOS_NAMESPACE::weos_detail::invoke(m_pm, std::forward<TArgs>(args)...);
    }

private:
    TMemberPointer m_pm;
};

} // namespace weos_detail

// ----=====================================================================----
//     mem_fn<>
// ----=====================================================================----

template <typename TResult, typename TClass>
inline
weos_detail::MemFnResult<TResult TClass::*> mem_fn(TResult TClass::* pm) noexcept
{
    return weos_detail::MemFnResult<TResult TClass::*>(pm);
}

// ----=====================================================================----
//     BindExpression & BindExpressionResult
// ----=====================================================================----

namespace weos_detail
{

template <typename T>
struct is_bind_expression : false_type {}; // TODO

} // namespace weos_detail

template <typename T>
struct is_bind_expression : weos_detail::is_bind_expression<T> {}; // TODO

namespace weos_detail
{
// -----------------------------------------------------------------------------
// ArgumentSelector
// -----------------------------------------------------------------------------

template <typename TBoundArg, typename TUnboundArgs,
          bool TIsReferenceWrapper = is_reference_wrapper<typename decay<TBoundArg>::type>::value,
          bool TIsPlaceholder = weos_is_placeholder<typename decay<TBoundArg>::type>::value,
          bool TIsBindExpression = is_bind_expression<TBoundArg>::value>
struct ArgumentSelector
{
    static_assert(   !TIsReferenceWrapper
                  && !TIsPlaceholder
                  && !TIsBindExpression, "Logic error");

    typedef TBoundArg& type;

    static type select(TBoundArg& bound, TUnboundArgs& /*unbound*/)
    {
        return bound;
    }
};

template <typename TBoundArg, typename TUnboundArgs>
struct ArgumentSelector<TBoundArg, TUnboundArgs, true, false, false>
{
    typedef typename TBoundArg::type& type;

    static type select(TBoundArg& bound, TUnboundArgs& /*unbound*/)
    {
        return bound.get();
    }
};

template <typename TBoundArg, typename TUnboundArgs>
struct ArgumentSelector<TBoundArg, TUnboundArgs, false, true, false>
{
    static const auto index = weos_is_placeholder<typename decay<TBoundArg>::type>::index;
    typedef typename tuple_element<index, TUnboundArgs>::type type;

    static type select(TBoundArg& /*bound*/, TUnboundArgs& unbound)
    {
        return std::forward<type>(std::get<index>(unbound));
    }
};

template <typename TBoundArg, typename TUnboundArgs>
struct ArgumentSelector<TBoundArg, TUnboundArgs, false, false, true>
{
    static_assert(!is_same<TBoundArg, TBoundArg>::value,
                  "Not implemented, yet");
};

template <typename TF, typename TBoundArgs, typename TUnboundArgs>
struct bind_expression_result_type;

template <typename TF, typename... TBoundArgs, typename TUnboundArgs>
struct bind_expression_result_type<TF, tuple<TBoundArgs...>, TUnboundArgs>
{
    typedef decltype(WEOS_NAMESPACE::weos_detail::invoke(
                         std::declval<TF&>(),
                         std::declval<typename ArgumentSelector<TBoundArgs, TUnboundArgs>::type>()...))
          type;
};

template <typename TF, typename TBoundArgs, size_t... TBoundIndices, typename TUnboundArgs>
inline
typename bind_expression_result_type<TF, TBoundArgs, TUnboundArgs>::type
invokeBindExpression(TF& functor,
                     TBoundArgs& boundArgs, TupleIndices<TBoundIndices...>,
                     TUnboundArgs&& unboundArgs)
{
    return WEOS_NAMESPACE::weos_detail::invoke(
                functor,
                ArgumentSelector<typename tuple_element<TBoundIndices, TBoundArgs>::type,
                                 TUnboundArgs>::select(
                    std::get<TBoundIndices>(boundArgs), unboundArgs)...);
}

// The result of a bind<>() call.
template <typename TFunctor, typename... TBoundArgs>
class BindExpression : public WeakResultType<typename decay<TFunctor>::type>
{
protected:
    typedef typename decay<TFunctor>::type functor_type;
    typedef tuple<typename decay<TBoundArgs>::type...> bound_args_type;
    typedef typename make_tuple_indices<sizeof...(TBoundArgs)>::type bound_indices_type;

public:
    template <typename F, typename... TArgs,
              typename _ = typename enable_if<is_constructible<functor_type, F>::value
                                              && !is_same<typename decay<F>::type,
                                                          BindExpression>::value>::type>
    explicit BindExpression(F&& f, TArgs&&... boundArgs)
        : m_functor(std::forward<F>(f)),
          m_boundArgs(std::forward<TArgs>(boundArgs)...)
    {
    }

    BindExpression(const BindExpression& other) = default;
    BindExpression& operator=(const BindExpression& other) = default;

    BindExpression(BindExpression&& other)
        : m_functor(std::move(other.m_functor)),
          m_boundArgs(std::move(other.m_boundArgs))
    {
    }

    BindExpression& operator=(BindExpression&& other)
    {
        m_functor = std::move(other.m_functor);
        m_boundArgs = std::move(other.m_boundArgs);
        return *this;
    }

    template <typename... TArgs>
    typename bind_expression_result_type<functor_type, bound_args_type, tuple<TArgs&&...>>::type
    operator()(TArgs&&... args)
    {
        return invokeBindExpression(m_functor,
                                    m_boundArgs, bound_indices_type(),
                                    forward_as_tuple(std::forward<TArgs>(args)...));
    }

    template <typename... TArgs>
    typename bind_expression_result_type<const functor_type, const bound_args_type, tuple<TArgs&&...>>::type
    operator()(TArgs&&... args) const
    {
        return invokeBindExpression(m_functor,
                                    m_boundArgs, bound_indices_type(),
                                    forward_as_tuple(std::forward<TArgs>(args)...));
    }

private:
    functor_type m_functor;
    bound_args_type m_boundArgs;
};

template <typename TFunctor, typename... TBoundArgs>
struct is_bind_expression<BindExpression<TFunctor, TBoundArgs...>> : true_type
{
};

// The result of a bind<TResult>() call.
template <typename TResult, typename TFunctor, typename... TBoundArgs>
class BindExpressionResult : public BindExpression<TFunctor, TBoundArgs...>
{
    typedef BindExpression<TFunctor, TBoundArgs...> base_type;
    typedef typename base_type::functor_type functor_type;

public:
    typedef TResult result_type;

    template <typename F, typename... TArgs,
              typename _ = typename enable_if<is_constructible<functor_type, F>::value
                                              && !is_same<typename decay<F>::type,
                                                          BindExpressionResult>::value>::type>
    explicit BindExpressionResult(F&& f, TArgs&&... boundArgs)
        : base_type(std::forward<F>(f),
                    std::forward<TArgs>(boundArgs)...)
    {
    }

    BindExpressionResult(const BindExpressionResult& other) = default;
    BindExpressionResult& operator=(const BindExpressionResult& other) = default;

    BindExpressionResult(BindExpressionResult&& other)
        : base_type(std::forward<base_type>(other))
    {
    }

    BindExpressionResult& operator=(BindExpressionResult&& other)
    {
        base_type::operator=(std::forward<base_type>(other));
        return *this;
    }

    template <typename... TArgs>
    result_type operator()(TArgs&&... args)
    {
        return base_type::operator()(std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    result_type operator()(TArgs&&... args) const
    {
        return base_type::operator()(std::forward<TArgs>(args)...);
    }
};

template <typename TResult, typename TFunctor, typename... TBoundArgs>
struct is_bind_expression<BindExpressionResult<TResult, TFunctor, TBoundArgs...>>
        : true_type
{
};

} // namespace weos_detail

// ----=====================================================================----
//     bind
// ----=====================================================================----

template <typename F, typename... TBoundArgs>
inline
weos_detail::BindExpression<F, TBoundArgs...> bind(F&& f, TBoundArgs&&... boundArgs)
{
    typedef weos_detail::BindExpression<F, TBoundArgs...> type;
    return type(std::forward<F>(f),
                std::forward<TBoundArgs>(boundArgs)...);
}

template <typename R, typename F, typename... TBoundArgs>
inline
weos_detail::BindExpressionResult<R, F, TBoundArgs...> bind(F&& f, TBoundArgs&&... boundArgs)
{
    typedef weos_detail::BindExpressionResult<R, F, TBoundArgs...> type;
    return type(std::forward<F>(f),
                std::forward<TBoundArgs>(boundArgs)...);
}

// ----=====================================================================----
//     function
// ----=====================================================================----

namespace weos_detail
{

class AnonymousClass;

// A small functor.
// This type is just large enough to hold a member function pointer
// plus a pointer to an instance or a function pointer plus an
// argument of pointer size.
struct SmallFunctor
{
    virtual ~SmallFunctor() {}

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

template <typename TSignature>
class InvokerBase;

template <typename TResult, typename... TArgs>
class InvokerBase<TResult(TArgs...)>
{
public:
    InvokerBase() noexcept = default;
    virtual ~InvokerBase() {}

    // Clones into newly allocated storage.
    virtual InvokerBase* clone() const = 0;
    // Clones into the given memory.
    virtual void clone(InvokerBase* memory) const = 0;

    virtual void destroy() noexcept = 0;
    virtual void destroyAndDeallocate() noexcept = 0;

    virtual TResult operator()(TArgs&&...) = 0;

#ifndef WEOS_NO_FUNCTION_TARGET
    virtual const void* target(const std::type_info& info) const noexcept = 0;
    virtual const std::type_info& targetType() const noexcept = 0;
#endif // WEOS_NO_FUNCTION_TARGET

    InvokerBase(const InvokerBase&) = delete;
    InvokerBase& operator=(const InvokerBase&) = delete;
};

template <typename TAllocator, typename TCallable, typename TSignature>
class Invoker;

template <typename TAllocator, typename TCallable,
          typename TResult, typename... TArgs>
class Invoker<TAllocator, TCallable, TResult(TArgs...)>
    : public InvokerBase<TResult(TArgs...)>
{
public:
    typedef InvokerBase<TResult(TArgs...)> base_type;

    explicit
    Invoker(TCallable&& f)
        : m_callableAllocator(std::move(f), TAllocator())
    {
    }

    explicit
    Invoker(TCallable&& f, TAllocator&& allocator)
        : m_callableAllocator(std::move(f),
                              std::move(allocator))
    {
    }

    explicit
    Invoker(const TCallable& f, TAllocator&& allocator)
        : m_callableAllocator(f, std::move(allocator))
    {
    }

    explicit
    Invoker(const TCallable& f, const TAllocator& allocator)
        : m_callableAllocator(f, allocator)
    {
    }

    virtual base_type* clone() const override
    {
        using traits = allocator_traits<TAllocator>;
        using allocator_t = typename traits::template rebind_alloc<Invoker>;
        using deallocator_t = WEOS_NAMESPACE::weos_detail::deallocator<allocator_t>;


        allocator_t allocator(std::get<1>(m_callableAllocator));
        unique_ptr<Invoker, deallocator_t> mem(
                    allocator.allocate(1), deallocator_t(allocator));
        new (mem.get()) Invoker(std::get<0>(m_callableAllocator),
                                TAllocator(allocator));
        return mem.release();
    }

    virtual void clone(base_type* memory) const override
    {
        new (memory) Invoker(std::get<0>(m_callableAllocator),
                             std::get<1>(m_callableAllocator));
    }

    virtual void destroy() noexcept override
    {
        m_callableAllocator.~tuple<TCallable, TAllocator>();
    }

    virtual void destroyAndDeallocate() noexcept override
    {
        using traits = allocator_traits<TAllocator>;
        using allocator_t = typename traits::template rebind_alloc<Invoker>;

        allocator_t allocator(std::get<1>(m_callableAllocator));
        m_callableAllocator.~tuple<TCallable, TAllocator>();
        allocator.deallocate(this, 1);
    }

    virtual TResult operator()(TArgs&&... args) override
    {
        return WEOS_NAMESPACE::weos_detail::invoke(std::get<0>(m_callableAllocator),
                                                   std::forward<TArgs>(args)...);
    }

#ifndef WEOS_NO_FUNCTION_TARGET
    virtual const void* target(const std::type_info& info) const noexcept override
    {
        if (typeid(TCallable) == info)
            return &std::get<0>(m_callableAllocator);
        else
            return nullptr;
    }

    virtual const std::type_info& targetType() const noexcept override
    {
        return typeid(TCallable);
    }
#endif // WEOS_NO_FUNCTION_TARGET

private:
    tuple<TCallable, TAllocator> m_callableAllocator;
};

} // namespace weos_detail

template <typename TSignature>
class function;

template <typename TResult, typename... TArgs>
class function<TResult(TArgs...)>
{
    using invoker_base_type = weos_detail::InvokerBase<TResult(TArgs...)>;
    using storage_type = typename aligned_storage<sizeof(weos_detail::SmallFunctor)>::type;

    template <typename F>
    using invokeResult = decltype(WEOS_NAMESPACE::weos_detail::invoke(
                                      declval<F&>(), declval<TArgs>()...));

    template <typename F>
    using isCallable = is_convertible<invokeResult<F>, TResult>;

    template <typename TInvoker>
    class fitsInplace
    {
        static const std::size_t smallSize = sizeof(storage_type);
        static const std::size_t smallAlign = alignment_of<storage_type>::value;
    public:
        static constexpr bool value = sizeof(TInvoker) <= smallSize
                                      && alignment_of<TInvoker>::value <= smallAlign
                                      && (smallAlign % alignment_of<TInvoker>::value == 0);
    };

    template <typename F, bool TFunctionPointer =    is_function<F>::value
                                                  || is_member_pointer<F>::value>
    struct notNull
    {
        static constexpr bool check(F fp) { return fp; }
    };

    template <typename F>
    struct notNull<F, false>
    {
        static constexpr bool check(const F&) { return true; }
    };

public:
    typedef TResult result_type;

    function() noexcept
        : m_invoker(nullptr)
    {
    }

    function(nullptr_t) noexcept
        : m_invoker(nullptr)
    {
    }

    function(const function& other)
        : m_invoker(nullptr)
    {
        if (other.m_invoker)
        {
            if (other.m_invoker == (const invoker_base_type*)&other.m_storage)
            {
                m_invoker = (invoker_base_type*)&m_storage;
                other.m_invoker->clone(m_invoker);
            }
            else
            {
                m_invoker = other.m_invoker->clone();
            }
        }
    }

    function(function&& other) noexcept
        : m_invoker(nullptr)
    {
        if (other.m_invoker)
        {
            if (other.m_invoker == (const invoker_base_type*)&other.m_storage)
            {
                m_invoker = (invoker_base_type*)&m_storage;
                other.m_invoker->clone(m_invoker);
            }
            else
            {
                m_invoker = other.m_invoker;
                other.m_invoker = nullptr;
            }
        }
    }

    template <typename TCallable,
              typename = typename enable_if<!is_same<typename decay<TCallable>::type,
                                                     function>::value
                                            && isCallable<TCallable>::value>::type>
    function(TCallable f)
        : m_invoker(nullptr)
    {
        typedef weos_detail::Invoker<allocator<TCallable>, TCallable, TResult(TArgs...)> invoker_type;
        using allocator_t = allocator<invoker_type>;
        using deallocator_t = WEOS_NAMESPACE::weos_detail::deallocator<allocator_t>;

        if (notNull<TCallable>::check(f))
        {
            if (   fitsInplace<invoker_type>::value
                && is_nothrow_copy_constructible<TCallable>::value)
            {
                // Construct inplace using placement new.
                m_invoker = (invoker_base_type*)&m_storage;
                new (m_invoker) invoker_type(std::move(f));
            }
            else
            {
                allocator_t alloc;
                unique_ptr<invoker_type, deallocator_t> mem(
                            alloc.allocate(1), deallocator_t(alloc));
                new (mem.get()) invoker_type(std::move(f),
                                               allocator<TCallable>(alloc));
                m_invoker = mem.release();
            }
        }
    }

    template <typename TAllocator>
    function(allocator_arg_t, const TAllocator&) noexcept
        : m_invoker(nullptr)
    {
    }

    template<typename TAllocator>
    function(allocator_arg_t, const TAllocator&, nullptr_t) noexcept
        : m_invoker(nullptr)
    {
    }

    // TODO
    template<typename TAllocator>
    function(allocator_arg_t, const TAllocator&, const function&);

    // TODO
    template<typename TAllocator>
    function(allocator_arg_t, const TAllocator&, function&&);

    //! Constructs a function from the callable \p f using the
    //! allocator \p alloc.
    template <typename TCallable, typename TAllocator,
              typename = typename enable_if<isCallable<TCallable>::value>::type>
    function(allocator_arg_t, const TAllocator& alloc, TCallable f)
        : m_invoker(nullptr)
    {
        typedef weos_detail::Invoker<TAllocator, TCallable, TResult(TArgs...)> invoker_type;
        using traits = allocator_traits<TAllocator>;
        using allocator_t = typename traits::template rebind_alloc<invoker_type>;
        using deallocator_t = WEOS_NAMESPACE::weos_detail::deallocator<allocator_t>;

        if (notNull<TCallable>::check(f))
        {
            if (   fitsInplace<invoker_type>::value
                && is_nothrow_copy_constructible<TCallable>::value
                && is_nothrow_copy_constructible<TAllocator>::value)
            {
                // Construct inplace using placement new.
                m_invoker = (invoker_base_type*)&m_storage;
                new (m_invoker) invoker_type(std::move(f), alloc);
            }
            else
            {
                allocator_t allocator(alloc);
                unique_ptr<invoker_type, deallocator_t> mem(
                            allocator.allocate(1), deallocator_t(allocator));
                new (mem.get()) invoker_type(std::move(f),
                                             TAllocator(allocator));
                m_invoker = mem.release();
            }
        }
    }

    ~function()
    {
        release();
    }

    function& operator=(const function& other)
    {
        function(other).swap(*this);
        return *this;
    }

    function& operator=(function&& other)
    {
        release();
        if (other.m_invoker == (invoker_base_type*)&other.m_storage)
        {
            m_invoker = (invoker_base_type*)&m_storage;
            other.m_invoker->clone((invoker_base_type*)&m_storage);
        }
        else
        {
            m_invoker = other.m_invoker;
            other.m_invoker = nullptr;
        }

        return *this;
    }

    function& operator=(nullptr_t) noexcept
    {
        release();
        return *this;
    }

    template <typename TCallable,
              typename = typename enable_if<!is_same<typename decay<TCallable>::type,
                                                     function>::value
                                            && isCallable<typename decay<TCallable>::type>::value>::type>
    function& operator=(TCallable&& f)
    {
        function(std::forward<TCallable>(f)).swap(*this);
        return *this;
    }

    result_type operator()(TArgs... args) const
    {
        if (!m_invoker)
            throw WEOS_EXCEPTION(bad_function_call());
        return (*m_invoker)(std::forward<TArgs>(args)...);
    }

    void swap(function& other) noexcept
    {
        if (this == &other)
            return;

        if (m_invoker == (invoker_base_type*)&m_storage
            && other.m_invoker == (invoker_base_type*)&other.m_storage)
        {
            // Both are small. We can only swap via some temporary space.
            storage_type tempStorage;
            invoker_base_type* tempInvoker = (invoker_base_type*)&tempStorage;

            m_invoker->clone(tempInvoker);
            m_invoker->destroy();
            other.m_invoker->clone((invoker_base_type*)&m_storage);
            other.m_invoker->destroy();
            tempInvoker->clone((invoker_base_type*)&other.m_storage);
            tempInvoker->destroy();

            m_invoker = (invoker_base_type*)&m_storage;
            other.m_invoker = (invoker_base_type*)&other.m_storage;
        }
        else if (other.m_invoker == (invoker_base_type*)&other.m_storage)
        {
            // The other function is small. Our internal storage is available.
            // Handles the case where this->m_invoker == nullptr.
            other.m_invoker->clone((invoker_base_type*)&m_storage);
            other.m_invoker->destroy();
            other.m_invoker = m_invoker;
            m_invoker = (invoker_base_type*)&m_storage;
        }
        else if (m_invoker == (invoker_base_type*)&m_storage)
        {
            // We are small. The other internal storage is free.
            // Handles the case where other.m_invoker == nullptr.
            m_invoker->clone((invoker_base_type*)&other.m_storage);
            m_invoker->destroy();
            m_invoker = other.m_invoker;
            other.m_invoker = (invoker_base_type*)&other.m_storage;
        }
        else
        {
            // Both are large or both are nullptr.
            std::swap(m_invoker, other.m_invoker);
        }
    }

    explicit operator bool() const noexcept
    {
        return m_invoker != nullptr;
    }

#ifndef WEOS_NO_FUNCTION_TARGET
    //! Returns a pointer to the stored target.
    template <typename T>
    T* target() noexcept
    {
        if (m_invoker)
            return (T*)m_invoker->target(typeid(T)); // TODO: change the cast; could have a const-correctness problem here
        else
            return nullptr;
    }

    //! Returns a pointer to the stored target.
    template <typename T>
    const T* target() const noexcept
    {
        if (m_invoker)
            return (const T*)m_invoker->target(typeid(T)); // TODO: change the cast
        else
            return nullptr;
    }

    //! Returns the type of the stored target.
    const std::type_info& target_type() const noexcept
    {
        if (m_invoker)
            return m_invoker->targetType();
        else
            return typeid(void);
    }
#endif // WEOS_NO_FUNCTION_TARGET

private:
    storage_type m_storage;
    invoker_base_type* m_invoker;

    void release() noexcept
    {
        if (m_invoker == (invoker_base_type*)&m_storage)
            m_invoker->destroy();
        else if (m_invoker)
            m_invoker->destroyAndDeallocate();
        m_invoker = nullptr;
    }
};

//! Returns \p true, if \p f has no target.
template <typename TResult, typename... TArgs>
bool operator==(const function<TResult(TArgs...)>& f, nullptr_t) noexcept
{
    return !f;
}

//! Returns \p true, if \p f has no target.
template <typename TResult, typename... TArgs>
bool operator==(nullptr_t, const function<TResult(TArgs...)>& f) noexcept
{
    return !f;
}

//! Returns \p true, if \p f has a target.
template <typename TResult, typename... TArgs>
bool operator!=(const function<TResult(TArgs...)>& f, nullptr_t) noexcept
{
    return (bool)f;
}

//! Returns \p true, if \p f has a target.
template <typename TResult, typename... TArgs>
bool operator!=(nullptr_t, const function<TResult(TArgs...)>& f) noexcept
{
    return (bool)f;
}

//! Swaps two functions \p x and \p y.
template <typename TResult, typename... TArgs>
void swap(function<TResult(TArgs...)>& x, function<TResult(TArgs...)>& y)
{
    x.swap(y);
}

} // namespace std

#endif // WEOS_ARMCC_FUNCTIONAL_HPP
