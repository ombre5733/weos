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

#ifndef WEOS_CXX11_THREAD_DETAIL_HPP
#define WEOS_CXX11_THREAD_DETAIL_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "_core.hpp"

#include "../functional.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"
#include "../_common/_index_sequence.hpp"
#include "../_common/_invoke.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TFunction, typename... TArgs>
class DecayedFunction
{
public:
    typedef typename WEOS_NAMESPACE::weos_detail::invoke_result_type<TFunction, TArgs...>::type result_type;

    explicit DecayedFunction(TFunction&& f, TArgs&&... args)
        : m_boundFunction(std::move(f),
                          std::move(args)...)
    {
    }

    DecayedFunction(DecayedFunction&& other)
        : m_boundFunction(std::move(other.m_boundFunction))
    {
    }

    result_type operator()()
    {
        using indices_type = typename WEOS_NAMESPACE::weos_detail::makeIndexSequence<
                                 1 + sizeof...(TArgs), 1>::type;
        return invoke(indices_type());
    }

private:
    template <std::size_t... TIndices>
    result_type invoke(WEOS_NAMESPACE::weos_detail::IndexSequence<TIndices...>)
    {
        return std::invoke(std::move(std::get<0>(m_boundFunction)),
                           std::move(std::get<TIndices>(m_boundFunction))...);
    }

    std::tuple<TFunction, TArgs...> m_boundFunction;
};

// 30.2.6
template <typename T>
typename decay<T>::type decay_copy(T&& v)
{
    return std::forward<T>(v);
}

} // namespace weos_detail

// ----=====================================================================----
//     thread_attributes
// ----=====================================================================----

//! The thread attributes.
class thread_attributes
{
public:
    //! An enumeration of thread priorities.
    enum class priority
    {
        idle = -3,
        low = -2,
        below_normal = -1,
        normal = 0,
        above_normal = 1,
        high = 2,
        realtime = 3
    };

private:
    template <typename T>
    struct can_be_stack
    {
        typedef typename decay<T>::type D;
        static constexpr bool value = !is_same<D, priority>::value &&
                                      !is_pointer<D>::value;
    };

public:
    //! Creates default thread attributes.
    constexpr
    thread_attributes() noexcept
        : m_stackBegin(nullptr),
          m_stackSize(0),
          m_name(""),
          m_priority(priority::normal)
    {
    }

    //! Creates thread attributes.
    template <typename T,
              typename = typename enable_if<can_be_stack<T>::value>::type>
    constexpr
    thread_attributes(T& stack, priority prio = priority::normal) noexcept
        : m_stackBegin(&stack),
          m_stackSize(sizeof(T)),
          m_name(""),
          m_priority(prio)
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
    }

    //! Creates thread attributes.
    constexpr
    thread_attributes(void* stack, std::size_t stackSize,
                      priority prio = priority::normal) noexcept
        : m_stackBegin(stack),
          m_stackSize(stackSize),
          m_name(""),
          m_priority(prio)
    {
    }

    //! Creates thread attributes.
    template <typename T,
              typename = typename enable_if<can_be_stack<T>::value>::type>
    constexpr
    thread_attributes(const char* name, T& stack,
                      priority prio = priority::normal) noexcept
        : m_stackBegin(&stack),
          m_stackSize(sizeof(T)),
          m_name(name),
          m_priority(prio)
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
    }

    //! Creates thread attributes.
    constexpr
    thread_attributes(const char* name, void* stack, std::size_t stackSize,
                      priority prio = priority::normal) noexcept
        : m_stackBegin(stack),
          m_stackSize(stackSize),
          m_name(name),
          m_priority(prio)
    {
    }

    thread_attributes(const thread_attributes&) = default;
    thread_attributes& operator=(const thread_attributes&) = default;

    //! Sets the name.
    //! Sets the name to \p name. The default is the empty string.
    thread_attributes& set_name(const char* name) noexcept
    {
        m_name = name;
        return *this;
    }

    //! Returns the name of the thread.
    constexpr
    const char* get_name() const noexcept
    {
        return m_name;
    }

    //! Sets the priority.
    //! Sets the thread priority to \p prio.
    //!
    //! The default value is priority::normal.
    thread_attributes& set_priority(priority prio) noexcept
    {
        m_priority = prio;
        return *this;
    }

    //! Returs the priority.
    constexpr
    priority get_priority() const noexcept
    {
        return m_priority;
    }

    //! Provides a custom stack.
    //! Makes the thread use the memory pointed to by \p stack whose size
    //! in bytes is passed in \p stackSize rather than the default stack.
    //!
    //! The default is a null-pointer for the stack and zero for its size.
    thread_attributes& set_stack(void* stack, std::size_t stackSize) noexcept
    {
        m_stackBegin = stack;
        m_stackSize = stackSize;
        return *this;
    }

    //! Provides a custom stack.
    //!
    //! Sets the thread's stack to \p stack.
    template <typename T>
    thread_attributes& set_stack(T& stack) noexcept
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
        m_stackBegin = &stack;
        m_stackSize = sizeof(T);
        return *this;
    }

    //! Returns the start of the stack.
    constexpr
    void* get_stack_begin() const noexcept
    {
        return m_stackBegin;
    }

    //! Returns the size of the stack.
    constexpr
    std::size_t get_stack_size() const noexcept
    {
        return m_stackSize;
    }

private:
    //! A pointer to the custom stack.
    void* m_stackBegin;
    //! The size of the custom stack.
    std::size_t m_stackSize;
    //! The thread's name.
    const char* m_name;
    //! The thread's priority.
    priority m_priority;

    friend class thread;
};

WEOS_END_NAMESPACE

#endif // WEOS_CXX11_THREAD_DETAIL_HPP
