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

#ifndef WEOS_COMMON_ATOMIC_IMPL_ARMCC_HPP
#define WEOS_COMMON_ATOMIC_IMPL_ARMCC_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


WEOS_BEGIN_NAMESPACE

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
//     atomic_flag
// ----=====================================================================----

#define ATOMIC_FLAG_INIT   0

class atomic_flag
{
public:
    atomic_flag() noexcept
    {
    }

    // Construction from ATOMIC_FLAG_INIT.
    constexpr atomic_flag(bool value) noexcept
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

    // ---- Hidden methods
    atomic_flag(const atomic_flag&);
    atomic_flag& operator= (const atomic_flag&);
    atomic_flag& operator=(const atomic_flag&) volatile;
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
//     atomic_base & atomic_integral
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

namespace detail
{

template <typename T>
class atomic_base
{
    static_assert(sizeof(T) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic_base() noexcept
    {
    }

    constexpr atomic_base(T value) noexcept
        : m_value((int)value)
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
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    void store(T value, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    T load(memory_order mo = memory_order_seq_cst) const noexcept
    {
        __dmb(0xF);
        return (T)m_value;
    }

    T load(memory_order mo = memory_order_seq_cst) const volatile noexcept
    {
        __dmb(0xF);
        return (T)m_value;
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
        T old;
        while (1)
        {
            old = (T)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        T old;
        while (1)
        {
            old = (T)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
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
            T old = (T)__ldrex(&m_value);
            if (old == expected)
            {
                if (__strex((int)desired, &m_value) == 0)
                {
                    __dmb(0xF);
                    return true;
                }
            }
            else
            {
                __clrex();
                expected = old;
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
            T old = (T)__ldrex(&m_value);
            if (old == expected)
            {
                if (__strex((int)desired, &m_value) == 0)
                {
                    __dmb(0xF);
                    return true;
                }
            }
            else
            {
                __clrex();
                expected = old;
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

protected:
    int m_value;
};

template <typename T>
class atomic_integral : public atomic_base<T>
{
    typedef atomic_base<T> base;

public:
    atomic_integral() noexcept
    {
    }

    constexpr atomic_integral(T value) noexcept
        : base(value)
    {
    }

    T fetch_add(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    T fetch_add(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    T fetch_sub(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    T fetch_sub(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    T fetch_and(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    T fetch_and(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    T fetch_or(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    T fetch_or(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    T fetch_xor(T arg, memory_order mo = memory_order_seq_cst) noexcept
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    T fetch_xor(T arg, memory_order mo = memory_order_seq_cst) volatile noexcept
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    T operator++() noexcept
    {
        return fetch_add(1) + 1;
    }

    T operator++() volatile noexcept
    {
        return fetch_add(1) + 1;
    }

    T operator++ (int) noexcept
    {
        return fetch_add(1);
    }

    T operator++ (int) volatile noexcept
    {
        return fetch_add(1);
    }

    T operator--() noexcept
    {
        return fetch_sub(1) - 1;
    }

    T operator--() volatile noexcept
    {
        return fetch_sub(1) - 1;
    }

    T operator-- (int) noexcept
    {
        return fetch_sub(1);
    }

    T operator-- (int) volatile noexcept
    {
        return fetch_sub(1);
    }

    T operator+= (T value) noexcept
    {
        return fetch_add(value) + value;
    }

    T operator+= (T value) volatile noexcept
    {
        return fetch_add(value) + value;
    }

    T operator-= (T value) noexcept
    {
        return fetch_sub(value) - value;
    }

    T operator-= (T value) volatile noexcept
    {
        return fetch_sub(value) - value;
    }

    T operator&= (T value) noexcept
    {
        return fetch_and(value) & value;
    }

    T operator&= (T value) volatile noexcept
    {
        return fetch_and(value) & value;
    }

    T operator|= (T value) noexcept
    {
        return fetch_or(value) | value;
    }

    T operator|= (T value) volatile noexcept
    {
        return fetch_or(value) | value;
    }

    T operator^= (T value) noexcept
    {
        return fetch_xor(value) ^ value;
    }

    T operator^= (T value) volatile noexcept
    {
        return fetch_xor(value) ^ value;
    }

    using base::operator=;
};

} // namespace detail

// ----=====================================================================----
//     atomic<T>
// ----=====================================================================----

template <typename T>
class atomic : public detail::atomic_base<T>
{
    typedef detail::atomic_base<T> base;

public:
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

// ----=====================================================================----
//     atomic<bool>
// ----=====================================================================----

typedef detail::atomic_base<bool> atomic_bool;

template <>
struct atomic<bool> : public atomic_bool
{
    typedef bool T;
    typedef atomic_bool base;

public:
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
        : base(value)
    {
    }

    using base::operator=;
};

// ----=====================================================================----
//     atomic<integral>
// ----=====================================================================----

typedef detail::atomic_integral<char>           atomic_char;
typedef detail::atomic_integral<signed char>    atomic_schar;
typedef detail::atomic_integral<unsigned char>  atomic_uchar;
typedef detail::atomic_integral<short>          atomic_short;
typedef detail::atomic_integral<unsigned short> atomic_ushort;
typedef detail::atomic_integral<int>            atomic_int;
typedef detail::atomic_integral<unsigned int>   atomic_uint;
typedef detail::atomic_integral<long>           atomic_long;
typedef detail::atomic_integral<unsigned long>  atomic_ulong;

template <>
struct atomic<char> : public atomic_char
{
    typedef char T;
    typedef atomic_char base;

public:
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
    atomic() noexcept
    {
    }

    constexpr atomic(T value) noexcept
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
struct atomic<T*> : public detail::atomic_base<T*>
{
    typedef T* pointer_type;
    typedef detail::atomic_base<T*> base;

    static_assert(sizeof(pointer_type) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic() noexcept
    {
    }

    constexpr atomic(pointer_type value) noexcept
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

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_ATOMIC_IMPL_ARMCC_HPP
