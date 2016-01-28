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

#ifndef WEOS_KEIL_CMSIS_RTOS_THREAD_DETAIL_HPP
#define WEOS_KEIL_CMSIS_RTOS_THREAD_DETAIL_HPP

#include "../config.hpp"

#include "../functional.hpp"
#include "../tuple.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TFunction, typename... TArgs>
class DecayedFunction
{
public:
    typedef typename ::WEOS_NAMESPACE::weos_detail::invoke_result_type<TFunction, TArgs...>::type result_type;

    explicit DecayedFunction(TFunction&& f, TArgs&&... args)
        : m_boundFunction(WEOS_NAMESPACE::move(f),
                          WEOS_NAMESPACE::move(args)...)
    {
    }

    DecayedFunction(DecayedFunction&& other)
        : m_boundFunction(WEOS_NAMESPACE::move(other.m_boundFunction))
    {
    }

    result_type operator()()
    {
        typedef typename weos_detail::make_tuple_indices<
                1 + sizeof...(TArgs), 1>::type indices_type;
        return invoke(indices_type());
    }

private:
    template <std::size_t... TIndices>
    result_type invoke(weos_detail::TupleIndices<TIndices...>)
    {
        return WEOS_NAMESPACE::invoke(
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<0>(m_boundFunction)),
                    WEOS_NAMESPACE::move(WEOS_NAMESPACE::get<TIndices>(m_boundFunction))...);
    }

    tuple<TFunction, TArgs...> m_boundFunction;
};

// 30.2.6
template <typename T>
typename decay<T>::type decay_copy(T&& v)
{
    return WEOS_NAMESPACE::forward<T>(v);
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
    WEOS_SCOPED_ENUM_BEGIN(priority)
    {
        idle = osPriorityIdle,
        low = osPriorityLow,
        below_normal = osPriorityBelowNormal,
        normal = osPriorityNormal,
        above_normal = osPriorityAboveNormal,
        high = osPriorityHigh,
        realtime = osPriorityRealtime,

        // Deprecated enums
        belowNormal = osPriorityBelowNormal,
        aboveNormal = osPriorityAboveNormal,
    };
    WEOS_SCOPED_ENUM_END(priority)

    //! Creates default thread attributes.
    constexpr
    thread_attributes() noexcept
        : m_priority(priority::normal),
          m_customStackSize(0),
          m_customStack(nullptr),
          m_name("")
    {
    }

    //! Creates thread attributes.
    //!
    //! Creates thread attributes from a priority \p prio and a \p stack.
    template <typename T>
    constexpr
    thread_attributes(priority prio, T& stack) noexcept
        : m_priority(prio),
          m_customStackSize(sizeof(T)),
          m_customStack(&stack),
          m_name("")
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
    }

    thread_attributes(const thread_attributes&) = default;
    thread_attributes& operator=(const thread_attributes&) = default;

    // Use get_name().
    constexpr
    __attribute__((deprecated))
    const char* name() const noexcept
    {
        return m_name;
    }

    // Use set_name().
    __attribute__((deprecated))
    thread_attributes& setName(const char* name) noexcept
    {
        m_name = name;
        return *this;
    }

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

    // Use set_priority().
    __attribute__((deprecated))
    thread_attributes& setPriority(priority prio) noexcept
    {
        m_priority = prio;
        return *this;
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

    // Use set_stack().
    __attribute__((deprecated))
    thread_attributes& setStack(void* stack, std::size_t stackSize) noexcept
    {
        m_customStack = stack;
        m_customStackSize = stackSize;
        return *this;
    }

    //! Provides a custom stack.
    //! Makes the thread use the memory pointed to by \p stack whose size
    //! in bytes is passed in \p stackSize rather than the default stack.
    //!
    //! The default is a null-pointer for the stack and zero for its size.
    thread_attributes& set_stack(void* stack, std::size_t stackSize) noexcept
    {
        m_customStack = stack;
        m_customStackSize = stackSize;
        return *this;
    }

    // Use set_stack().
    template <typename T>
    __attribute__((deprecated))
    thread_attributes& setStack(T& stack) noexcept
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
        m_customStack = &stack;
        m_customStackSize = sizeof(T);
        return *this;
    }

    //! Provides a custom stack.
    //!
    //! Sets the thread's stack to \p stack.
    template <typename T>
    thread_attributes& set_stack(T& stack) noexcept
    {
        static_assert(sizeof(T) >= 4 * 16, "The stack is too small.");
        m_customStack = &stack;
        m_customStackSize = sizeof(T);
        return *this;
    }

    // Use get_stack_begin().
    constexpr
    __attribute__((deprecated))
    void* stackBegin() const noexcept
    {
        return m_customStack;
    }

    // Use get_stack_size().
    constexpr
    __attribute__((deprecated))
    std::size_t stackSize() const noexcept
    {
        return m_customStackSize;
    }

    //! Returns the start of the stack.
    constexpr
    void* get_stack_begin() const noexcept
    {
        return m_customStack;
    }

    //! Returns the size of the stack.
    constexpr
    std::size_t get_stack_size() const noexcept
    {
        return m_customStackSize;
    }

private:
    //! The thread's priority.
    priority m_priority;
    //! The size of the custom stack.
    std::size_t m_customStackSize;
    //! A pointer to the custom stack.
    void* m_customStack;
    //! The thread's name.
    const char* m_name;

    friend class thread;
};

// ----=====================================================================----
//     ThreadProperties
// ----=====================================================================----

namespace weos_detail
{

struct ThreadProperties
{
    class Deleter
    {
    public:
        Deleter(void* ownedStack) noexcept
            : m_ownedStack(ownedStack)
        {
        }

        Deleter(Deleter&& other) noexcept
            : m_ownedStack(other.m_ownedStack)
        {
            other.m_ownedStack = nullptr;
        }

        ~Deleter() noexcept;

        Deleter(const Deleter&) = delete;
        Deleter& operator=(const Deleter&) = delete;

        void* owned_stack() const noexcept
        {
            return m_ownedStack;
        }

        void release() noexcept
        {
            m_ownedStack = nullptr;
        }

    private:
        void* m_ownedStack;
    };

    ThreadProperties() = default;
    ThreadProperties(const thread_attributes& attrs) noexcept;

    ThreadProperties(const ThreadProperties&) = delete;
    ThreadProperties& operator=(const ThreadProperties&) = delete;

    Deleter allocate();

    void* align(std::size_t alignment, std::size_t size) noexcept;
    void* max_align() noexcept;
    void offset_by(std::size_t size) noexcept;


    const char* m_name{""};
    int m_priority{static_cast<int>(thread_attributes::priority::normal)};
    void* m_initialStackBase{nullptr};
    void* m_stackBegin{nullptr};
    std::size_t m_stackSize{0};
};

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_THREAD_DETAIL_HPP
