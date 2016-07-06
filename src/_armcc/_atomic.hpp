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

#ifndef WEOS_ARMCC_ATOMIC_HPP
#define WEOS_ARMCC_ATOMIC_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../mutex.hpp"
#include "../type_traits.hpp"


#if __ARMCC_VERSION >= 5060000
// Do not warn about deprecated __ldrex and __strex.
#pragma diag_suppress 3731
#endif


namespace std
{

enum memory_order
{
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
};

// ----=====================================================================----
//     atomic_thread_fence
// ----=====================================================================----

inline
void atomic_thread_fence(memory_order /*mo*/) noexcept
{
    __dmb(0xF);
}

// ----=====================================================================----
//     atomic_flag
// ----=====================================================================----

#define ATOMIC_FLAG_INIT   { false }

class atomic_flag
{
public:
    atomic_flag() noexcept = default;
    ~atomic_flag() = default;

    // Construction from ATOMIC_FLAG_INIT.
    constexpr
    atomic_flag(bool value) noexcept
      : m_value(value)
    {
    }

    void clear(memory_order mo = memory_order_seq_cst) noexcept
    {
        fetch_and_set(0);
    }

    void clear(memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        fetch_and_set(0);
    }

    bool test_and_set(memory_order mo = memory_order_seq_cst) noexcept
    {
        return fetch_and_set(1);
    }

    bool test_and_set(memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        return fetch_and_set(1);
    }

    atomic_flag(const atomic_flag&) = delete;
    atomic_flag& operator=(const atomic_flag&) = delete;
    atomic_flag& operator=(const atomic_flag&) volatile = delete;

private:
    volatile int m_value;

    int fetch_and_set(int newValue) volatile noexcept
    {
        int status, oldValue;
        do
        {
            // Read the old value and create a monitor.
            oldValue = __ldrex(&m_value);
            // Try to store the new value in the lock variable. The return value
            // is zero when the write has succeeded.
            status = __strex(newValue, &m_value);
        } while (status != 0);
        // No new memory accesses can be started until the following barrier
        // succeeds.
        __dmb(0xF);
        return oldValue;
    }
};

inline
void atomic_flag_clear(atomic_flag* flag) noexcept
{
    flag->clear();
}

inline
void atomic_flag_clear(volatile atomic_flag* flag) noexcept
{
    flag->clear();
}

inline
void atomic_flag_clear_explicit(atomic_flag* flag, memory_order mo) noexcept
{
    flag->clear(mo);
}

inline
void atomic_flag_clear_explicit(volatile atomic_flag* flag, memory_order mo) noexcept
{
    flag->clear(mo);
}

inline
bool atomic_flag_test_and_set(atomic_flag* flag) noexcept
{
    return flag->test_and_set();
}

inline
bool atomic_flag_test_and_set(volatile atomic_flag* flag) noexcept
{
    return flag->test_and_set();
}

inline
bool atomic_flag_test_and_set_explicit(atomic_flag* flag, memory_order mo) noexcept
{
    return flag->test_and_set(mo);
}

inline
bool atomic_flag_test_and_set_explicit(volatile atomic_flag* flag, memory_order mo) noexcept
{
    return flag->test_and_set(mo);
}

// ----=====================================================================----
//     atomic_base
// ----=====================================================================----

#define WEOS_ATOMIC_MODIFY(type, op, arg)                                      \
    type old;                                                                  \
    while (1)                                                                  \
    {                                                                          \
        old = (type)__ldrex(&this->m_value);                                   \
        if (__strex((int)(old op arg), &this->m_value) == 0)                   \
            break;                                                             \
    }                                                                          \
    __dmb(0xF);                                                                \
    return old

namespace weos_detail
{
    extern mutex g_atomicMutex;
}

#define WEOS_LOCKED_MODIFY(type, op, arg)                                      \
    lock_guard<mutex> lock(weos_detail::g_atomicMutex);                        \
    type old = m_value;                                                        \
    m_value = old op arg;                                                      \
    return old

namespace weos_detail
{

template <typename T, bool TSmall = sizeof(T) <= sizeof(int)>
class atomic_base
{
    static_assert(sizeof(T) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic_base() noexcept = default;
    ~atomic_base() = default;

    constexpr
    atomic_base(T value) noexcept
        : m_value(*(int*)&value)
    {
    }

    atomic_base(const atomic_base&) = delete;

    bool is_lock_free() const noexcept
    {
        return true;
    }

    bool is_lock_free() const volatile noexcept
    {
        return true;
    }

    void store(T value, memory_order mo = memory_order_seq_cst) noexcept
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex(*(int*)&value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    void store(T value, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex(*(int*)&value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    T load(memory_order mo = memory_order_seq_cst) const noexcept
    {
        __dmb(0xF);
        return *(T*)&m_value;
    }

    T load(memory_order mo = memory_order_seq_cst) const volatile noexcept
    {
        __dmb(0xF);
        return *(T*)&m_value;
    }

    operator T() const noexcept
    {
        return load();
    }

    operator T() const volatile noexcept
    {
        return load();
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) noexcept
    {
        int old;
        while (1)
        {
            old = __ldrex(&m_value);
            if (__strex(*(int*)&desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return *(T*)&old;
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        int old;
        while (1)
        {
            old = __ldrex(&m_value);
            if (__strex(*(int*)&desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return *(T*)&old;
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order success,
                               memory_order failure) noexcept
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order success,
                               memory_order failure) volatile noexcept
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) noexcept
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) volatile noexcept
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order mo = memory_order_seq_cst) noexcept
    {
        while (1)
        {
            int old = __ldrex(&m_value);
            if (*(T*)&old == expected)
            {
                if (__strex(*(int*)&desired, &m_value) == 0)
                {
                    __dmb(0xF);
                    return true;
                }
            }
            else
            {
                __clrex();
                expected = *(T*)&old;
                __dmb(0xF);
                return false;
            }
        }
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        while (1)
        {
            int old = __ldrex(&m_value);
            if (*(T*)&old == expected)
            {
                if (__strex(*(int*)&desired, &m_value) == 0)
                {
                    __dmb(0xF);
                    return true;
                }
            }
            else
            {
                __clrex();
                expected = *(T*)&old;
                __dmb(0xF);
                return false;
            }
        }
    }

    T operator= (T value) noexcept
    {
        store(value);
        return value;
    }

    T operator= (T value) volatile noexcept
    {
        store(value);
        return value;
    }

    atomic_base& operator=(const atomic_base&) = delete;
    atomic_base& operator=(const atomic_base&) volatile = delete;

    // Integral

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_add(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_add(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_sub(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_sub(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_and(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_and(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_or(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_or(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_xor(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_xor(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++() noexcept
    {
        return fetch_add(1) + 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++() volatile noexcept
    {
        return fetch_add(1) + 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++ (int) noexcept
    {
        return fetch_add(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++ (int) volatile noexcept
    {
        return fetch_add(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator--() noexcept
    {
        return fetch_sub(1) - 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator--() volatile noexcept
    {
        return fetch_sub(1) - 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-- (int) noexcept
    {
        return fetch_sub(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-- (int) volatile noexcept
    {
        return fetch_sub(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator+= (T value) noexcept
    {
        return fetch_add(value) + value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator+= (T value) volatile noexcept
    {
        return fetch_add(value) + value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-= (T value) noexcept
    {
        return fetch_sub(value) - value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-= (T value) volatile noexcept
    {
        return fetch_sub(value) - value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator&= (T value) noexcept
    {
        return fetch_and(value) & value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator&= (T value) volatile noexcept
    {
        return fetch_and(value) & value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator|= (T value) noexcept
    {
        return fetch_or(value) | value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator|= (T value) volatile noexcept
    {
        return fetch_or(value) | value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator^= (T value) noexcept
    {
        return fetch_xor(value) ^ value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator^= (T value) volatile noexcept
    {
        return fetch_xor(value) ^ value;
    }

protected:
    // The storage must be sizeof(int) because WEOS_ATOMIC_MODIFY operates
    // on words.
    int m_value;
};

template <typename T>
class atomic_base<T, /*bool TSmall =*/ false>
{
public:
    atomic_base() noexcept = default;
    ~atomic_base() = default;

    constexpr
    atomic_base(T value) noexcept
        : m_value(value)
    {
    }

    atomic_base(const atomic_base&) = delete;

    bool is_lock_free() const noexcept
    {
        return false;
    }

    bool is_lock_free() const volatile noexcept
    {
        return false;
    }

    void store(T value, memory_order mo = memory_order_seq_cst) noexcept
    {
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        m_value = value;
    }

    void store(T value, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        m_value = value;
    }

    T load(memory_order mo = memory_order_seq_cst) const noexcept
    {
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        return m_value;
    }

    T load(memory_order mo = memory_order_seq_cst) const volatile noexcept
    {
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        return m_value;
    }

    operator T() const noexcept
    {
        return load();
    }

    operator T() const volatile noexcept
    {
        return load();
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) noexcept
    {
        using namespace std;
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        swap(desired, m_value);
        return desired;
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        using namespace std;
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        swap(desired, m_value);
        return desired;
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order success,
                               memory_order failure) noexcept
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order success,
                               memory_order failure) volatile noexcept
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) noexcept
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) volatile noexcept
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order mo = memory_order_seq_cst) noexcept
    {
        using namespace std;
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        if (m_value == expected)
        {
            m_value = desired;
            return true;
        }
        else
        {
            expected = m_value;
            return false;
        }
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        using namespace std;
        lock_guard<mutex> lock(weos_detail::g_atomicMutex);
        if (m_value == expected)
        {
            m_value = desired;
            return true;
        }
        else
        {
            expected = m_value;
            return false;
        }
    }

    T operator= (T value) noexcept
    {
        store(value);
        return value;
    }

    T operator= (T value) volatile noexcept
    {
        store(value);
        return value;
    }

    atomic_base& operator=(const atomic_base&) = delete;
    atomic_base& operator=(const atomic_base&) volatile = delete;

    // Integral

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_add(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_LOCKED_MODIFY(T, +, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_add(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_LOCKED_MODIFY(T, +, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_sub(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_LOCKED_MODIFY(T, -, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_sub(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_LOCKED_MODIFY(T, -, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_and(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_LOCKED_MODIFY(T, &, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_and(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_LOCKED_MODIFY(T, &, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_or(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_LOCKED_MODIFY(T, |, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_or(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_LOCKED_MODIFY(T, |, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_xor(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_LOCKED_MODIFY(T, ^, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    fetch_xor(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_LOCKED_MODIFY(T, ^, arg);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++() noexcept
    {
        return fetch_add(1) + 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++() volatile noexcept
    {
        return fetch_add(1) + 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++ (int) noexcept
    {
        return fetch_add(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator++ (int) volatile noexcept
    {
        return fetch_add(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator--() noexcept
    {
        return fetch_sub(1) - 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator--() volatile noexcept
    {
        return fetch_sub(1) - 1;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-- (int) noexcept
    {
        return fetch_sub(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-- (int) volatile noexcept
    {
        return fetch_sub(1);
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator+= (T value) noexcept
    {
        return fetch_add(value) + value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator+= (T value) volatile noexcept
    {
        return fetch_add(value) + value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-= (T value) noexcept
    {
        return fetch_sub(value) - value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator-= (T value) volatile noexcept
    {
        return fetch_sub(value) - value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator&= (T value) noexcept
    {
        return fetch_and(value) & value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator&= (T value) volatile noexcept
    {
        return fetch_and(value) & value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator|= (T value) noexcept
    {
        return fetch_or(value) | value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator|= (T value) volatile noexcept
    {
        return fetch_or(value) | value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator^= (T value) noexcept
    {
        return fetch_xor(value) ^ value;
    }

    template <bool Q = is_integral<T>::value>
    typename enable_if<Q, T>::type
    operator^= (T value) volatile noexcept
    {
        return fetch_xor(value) ^ value;
    }

protected:
    T m_value;
};

} // namespace weos_detail

// ----=====================================================================----
//     atomic<T>
// ----=====================================================================----

template <typename T>
class atomic : public weos_detail::atomic_base<T>
{
    typedef weos_detail::atomic_base<T> base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

// ----=====================================================================----
//     atomic<bool>
// ----=====================================================================----

typedef weos_detail::atomic_base<bool> atomic_bool;

template <>
struct atomic<bool> : public atomic_bool
{
    typedef bool T;
    typedef atomic_bool base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

// ----=====================================================================----
//     atomic<integral>
// ----=====================================================================----

typedef weos_detail::atomic_base<char>           atomic_char;
typedef weos_detail::atomic_base<signed char>    atomic_schar;
typedef weos_detail::atomic_base<unsigned char>  atomic_uchar;
typedef weos_detail::atomic_base<short>          atomic_short;
typedef weos_detail::atomic_base<unsigned short> atomic_ushort;
typedef weos_detail::atomic_base<int>            atomic_int;
typedef weos_detail::atomic_base<unsigned int>   atomic_uint;
typedef weos_detail::atomic_base<long>           atomic_long;
typedef weos_detail::atomic_base<unsigned long>  atomic_ulong;

template <>
struct atomic<char> : public atomic_char
{
    typedef char T;
    typedef atomic_char base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<signed char> : public atomic_schar
{
    typedef signed char T;
    typedef atomic_schar base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<unsigned char> : public atomic_uchar
{
    typedef unsigned char T;
    typedef atomic_uchar base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<short> : public atomic_short
{
    typedef short T;
    typedef atomic_short base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<unsigned short> : public atomic_ushort
{
    typedef unsigned short T;
    typedef atomic_ushort base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<int> : public atomic_int
{
    typedef int T;
    typedef atomic_int base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<unsigned int> : public atomic_uint
{
    typedef unsigned int T;
    typedef atomic_uint base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<long> : public atomic_long
{
    typedef long T;
    typedef atomic_long base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

template <>
class atomic<unsigned long> : public atomic_ulong
{
    typedef unsigned long T;
    typedef atomic_ulong base;

public:
    atomic() noexcept = default;

    constexpr
    atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

// ----=====================================================================----
//     atomic<T*>
// ----=====================================================================----

// Partial specialization for pointer types.
template <typename T>
struct atomic<T*> : public weos_detail::atomic_base<T*>
{
    typedef T* pointer_type;
    typedef weos_detail::atomic_base<T*> base;

    static_assert(sizeof(pointer_type) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic() noexcept = default;

    constexpr
    atomic(pointer_type value) noexcept
        : base(value)
    {
    }

    pointer_type fetch_add(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(pointer_type, +, arg);
    }

    pointer_type fetch_add(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(pointer_type, +, arg);
    }

    pointer_type fetch_sub(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(pointer_type, -, arg);
    }

    pointer_type fetch_sub(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(pointer_type, -, arg);
    }

    pointer_type operator++() noexcept
    {
        return fetch_add(1) + 1;
    }

    pointer_type operator++() volatile noexcept
    {
        return fetch_add(1) + 1;
    }

    pointer_type operator++ (int) noexcept
    {
        return fetch_add(1);
    }

    pointer_type operator++ (int) volatile noexcept
    {
        return fetch_add(1);
    }

    pointer_type operator--() noexcept
    {
        return fetch_sub(1) - 1;
    }

    pointer_type operator--() volatile noexcept
    {
        return fetch_sub(1) - 1;
    }

    pointer_type operator-- (int) noexcept
    {
        return fetch_sub(1);
    }

    pointer_type operator-- (int) volatile noexcept
    {
        return fetch_sub(1);
    }

    pointer_type operator+= (std::ptrdiff_t value) noexcept
    {
        return fetch_add(value) + value;
    }

    pointer_type operator+= (std::ptrdiff_t value) volatile noexcept
    {
        return fetch_add(value) + value;
    }

    pointer_type operator-= (std::ptrdiff_t value) noexcept
    {
        return fetch_sub(value) - value;
    }

    pointer_type operator-= (std::ptrdiff_t value) volatile noexcept
    {
        return fetch_sub(value) - value;
    }

    using base::operator=;
};

#undef WEOS_ATOMIC_MODIFY
#undef WEOS_LOCKED_MODIFY

} // namespace std

// Actually, the pragma should be disabled again. But then ARMCC issues warnings
// whenever atomic<> is instanciated.
//#if __ARMCC_VERSION >= 5060000
//#pragma diag_warning 3731
//#endif

#endif // WEOS_ARMCC_ATOMIC_HPP
