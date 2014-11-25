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
    atomic_flag() WEOS_NOEXCEPT
    {
    }

    // Construction from ATOMIC_FLAG_INIT.
    WEOS_CONSTEXPR atomic_flag(bool value) WEOS_NOEXCEPT
      : m_value(value)
    {
    }

    void clear(memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        fetch_and_set(0);
    }

    void clear(memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        fetch_and_set(0);
    }

    bool test_and_set(memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        return fetch_and_set(1);
    }

    bool test_and_set(memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        return fetch_and_set(1);
    }

private:
    volatile int m_value;

    int fetch_and_set(int newValue) volatile WEOS_NOEXCEPT
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
void atomic_flag_clear(atomic_flag* flag) WEOS_NOEXCEPT
{
    flag->clear();
}

inline
void atomic_flag_clear(volatile atomic_flag* flag) WEOS_NOEXCEPT
{
    flag->clear();
}

inline
void atomic_flag_clear_explicit(atomic_flag* flag, memory_order mo) WEOS_NOEXCEPT
{
    flag->clear(mo);
}

inline
void atomic_flag_clear_explicit(volatile atomic_flag* flag, memory_order mo) WEOS_NOEXCEPT
{
    flag->clear(mo);
}

inline
bool atomic_flag_test_and_set(atomic_flag* flag) WEOS_NOEXCEPT
{
    return flag->test_and_set();
}

inline
bool atomic_flag_test_and_set(volatile atomic_flag* flag) WEOS_NOEXCEPT
{
    return flag->test_and_set();
}

inline
bool atomic_flag_test_and_set_explicit(atomic_flag* flag, memory_order mo) WEOS_NOEXCEPT
{
    return flag->test_and_set(mo);
}

inline
bool atomic_flag_test_and_set_explicit(volatile atomic_flag* flag, memory_order mo) WEOS_NOEXCEPT
{
    return flag->test_and_set(mo);
}

// ----=====================================================================----
//     atomic_base
// ----=====================================================================----

namespace detail
{

#define WEOS_ATOMIC_MODIFY(type, op, arg)                                      \
    type old;                                                                  \
    while (1)                                                                  \
    {                                                                          \
        old = (type)__ldrex(&m_value);                                         \
        if (__strex((int)(old op arg), &m_value) == 0)                         \
            break;                                                             \
    }                                                                          \
    __dmb(0xF);                                                                \
    return old


template <typename T>
class atomic_base
{
    static_assert(sizeof(T) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic_base() WEOS_NOEXCEPT
    {
    }

    WEOS_CONSTEXPR atomic_base(T value) WEOS_NOEXCEPT
        : m_value(value)
    {
    }

    bool is_lock_free() const WEOS_NOEXCEPT
    {
        return true;
    }

    bool is_lock_free() const volatile WEOS_NOEXCEPT
    {
        return true;
    }

    T load(memory_order mo = memory_order_seq_cst) const WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (T)m_value;
    }

    T load(memory_order mo = memory_order_seq_cst) const volatile WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (T)m_value;
    }

    void store(T value, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    void store(T value, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    T exchange(T desired, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
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

    T exchange(T desired, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
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
                               memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order success,
                               memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_weak(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order success,
                                 memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(T& expected, T desired,
                                 memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
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
                                 memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
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

    T fetch_add(T arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    T fetch_add(T arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, +, arg);
    }

    T fetch_sub(T arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    T fetch_sub(T arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, -, arg);
    }

    T fetch_and(T arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    T fetch_and(T arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, &, arg);
    }

    T fetch_or(T arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    T fetch_or(T arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, |, arg);
    }

    T fetch_xor(T arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    T fetch_xor(T arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(T, ^, arg);
    }

    T operator= (T value) WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    T operator= (T value) volatile WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    T operator++() WEOS_NOEXCEPT
    {
        return fetch_add(1) + 1;
    }

    T operator++() volatile WEOS_NOEXCEPT
    {
        return fetch_add(1) + 1;
    }

    T operator++ (int) WEOS_NOEXCEPT
    {
        return fetch_add(1);
    }

    T operator++ (int) volatile WEOS_NOEXCEPT
    {
        return fetch_add(1);
    }

    T operator--() WEOS_NOEXCEPT
    {
        return fetch_sub(1) - 1;
    }

    T operator--() volatile WEOS_NOEXCEPT
    {
        return fetch_sub(1) - 1;
    }

    T operator-- (int) WEOS_NOEXCEPT
    {
        return fetch_sub(1);
    }

    T operator-- (int) volatile WEOS_NOEXCEPT
    {
        return fetch_sub(1);
    }

    T operator+= (T value) WEOS_NOEXCEPT
    {
        return fetch_add(value) + value;
    }

    T operator+= (T value) volatile WEOS_NOEXCEPT
    {
        return fetch_add(value) + value;
    }

    T operator-= (T value) WEOS_NOEXCEPT
    {
        return fetch_sub(value) - value;
    }

    T operator-= (T value) volatile WEOS_NOEXCEPT
    {
        return fetch_sub(value) - value;
    }

    T operator&= (T value) WEOS_NOEXCEPT
    {
        return fetch_and(value) & value;
    }

    T operator&= (T value) volatile WEOS_NOEXCEPT
    {
        return fetch_and(value) & value;
    }

    T operator|= (T value) WEOS_NOEXCEPT
    {
        return fetch_or(value) | value;
    }

    T operator|= (T value) volatile WEOS_NOEXCEPT
    {
        return fetch_or(value) | value;
    }

    T operator^= (T value) WEOS_NOEXCEPT
    {
        return fetch_xor(value) ^ value;
    }

    T operator^= (T value) volatile WEOS_NOEXCEPT
    {
        return fetch_xor(value) ^ value;
    }

    operator T() const WEOS_NOEXCEPT
    {
        return load();
    }

    operator T() const volatile WEOS_NOEXCEPT
    {
        return load();
    }

private:
    int m_value;

protected:
    // ---- Hidden methods
    atomic_base(const atomic_base&);
    atomic_base& operator=(const atomic_base&);
    atomic_base& operator=(const atomic_base&) volatile;
};

} // namespace detail

typedef detail::atomic_base<char>           atomic_char;
typedef detail::atomic_base<signed char>    atomic_schar;
typedef detail::atomic_base<unsigned char>  atomic_uchar;
typedef detail::atomic_base<short>          atomic_short;
typedef detail::atomic_base<unsigned short> atomic_ushort;
typedef detail::atomic_base<int>            atomic_int;
typedef detail::atomic_base<unsigned int>   atomic_uint;
typedef detail::atomic_base<long>           atomic_long;
typedef detail::atomic_base<unsigned long>  atomic_ulong;


template <typename T>
class atomic;

template <>
struct atomic<char> : public atomic_char
{
    typedef char T;
    typedef atomic_char base;

public:
    using base::operator=;
};

template <>
class atomic<signed char> : public atomic_schar
{
    typedef signed char T;
    typedef atomic_schar base;

public:
    using base::operator=;
};

template <>
class atomic<unsigned char> : public atomic_uchar
{
    typedef unsigned char T;
    typedef atomic_uchar base;

public:
    using base::operator=;
};

template <>
class atomic<short> : public atomic_short
{
    typedef short T;
    typedef atomic_short base;

public:
    using base::operator=;
};

template <>
class atomic<unsigned short> : public atomic_ushort
{
    typedef unsigned short T;
    typedef atomic_ushort base;

public:
    using base::operator=;
};

template <>
class atomic<int> : public atomic_int
{
    typedef int T;
    typedef atomic_int base;

public:
    using base::operator=;
};

template <>
class atomic<unsigned int> : public atomic_uint
{
    typedef unsigned int T;
    typedef atomic_uint base;

public:
    using base::operator=;
};

template <>
class atomic<long> : public atomic_long
{
    typedef long T;
    typedef atomic_long base;

public:
    using base::operator=;
};

template <>
class atomic<unsigned long> : public atomic_ulong
{
    typedef unsigned long T;
    typedef atomic_ulong base;

public:
    using base::operator=;
};


template <>
class atomic<bool>
{
public:
    atomic() WEOS_NOEXCEPT
    {
    }

    WEOS_CONSTEXPR atomic(bool value) WEOS_NOEXCEPT
        : m_value(value)
    {
    }

    bool is_lock_free() const WEOS_NOEXCEPT
    {
        return true;
    }

    bool is_lock_free() const volatile WEOS_NOEXCEPT
    {
        return true;
    }

    bool load(memory_order mo = memory_order_seq_cst) const WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (bool)m_value;
    }

    bool load(memory_order mo = memory_order_seq_cst) const volatile WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (bool)m_value;
    }

    void store(bool value, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    void store(bool value, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    bool exchange(bool desired, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        bool old;
        while (1)
        {
            old = (bool)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
    }

    bool exchange(bool desired, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        bool old;
        while (1)
        {
            old = (bool)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
    }

    bool compare_exchange_weak(bool& expected, bool desired,
                               memory_order success,
                               memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(bool& expected, bool desired,
                               memory_order success,
                               memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(bool& expected, bool desired,
                               memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_weak(bool& expected, bool desired,
                               memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_strong(bool& expected, bool desired,
                                 memory_order success,
                                 memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(bool& expected, bool desired,
                                 memory_order success,
                                 memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(bool& expected, bool desired,
                                 memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        while (1)
        {
            bool old = (bool)__ldrex(&m_value);
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

    bool compare_exchange_strong(bool& expected, bool desired,
                                 memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        while (1)
        {
            bool old = (bool)__ldrex(&m_value);
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

    bool operator= (bool value) WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    bool operator= (bool value) volatile WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    operator bool() const WEOS_NOEXCEPT
    {
        return load();
    }

    operator bool() const volatile WEOS_NOEXCEPT
    {
        return load();
    }

private:
    int m_value;

protected:
    // ---- Hidden methods
    atomic(const atomic&);
    atomic& operator=(const atomic&);
    atomic& operator=(const atomic&) volatile;
};

// Partial specialization for pointer types.
template <typename T>
struct atomic<T*>
{
    typedef T* pointer_type;

    static_assert(sizeof(pointer_type) <= sizeof(int),
                  "Atomics are only implemented up to the size of int.");

public:
    atomic() WEOS_NOEXCEPT
    {
    }

    WEOS_CONSTEXPR atomic(pointer_type value) WEOS_NOEXCEPT
        : m_value((int)value)
    {
    }

    bool is_lock_free() const WEOS_NOEXCEPT
    {
        return true;
    }

    bool is_lock_free() const volatile WEOS_NOEXCEPT
    {
        return true;
    }

    pointer_type load(memory_order mo = memory_order_seq_cst) const WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (pointer_type)m_value;
    }

    pointer_type load(memory_order mo = memory_order_seq_cst) const volatile WEOS_NOEXCEPT
    {
        __dmb(0xF);
        return (pointer_type)m_value;
    }

    void store(pointer_type value, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    void store(pointer_type value, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        while (1)
        {
            __ldrex(&m_value);
            if (__strex((int)value, &m_value) == 0)
                break;
        }
        __dmb(0xF);
    }

    pointer_type exchange(pointer_type desired, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        pointer_type old;
        while (1)
        {
            old = (pointer_type)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
    }

    pointer_type exchange(pointer_type desired, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        pointer_type old;
        while (1)
        {
            old = (pointer_type)__ldrex(&m_value);
            if (__strex((int)desired, &m_value) == 0)
                break;
        }
        __dmb(0xF);
        return old;
    }

    bool compare_exchange_weak(pointer_type& expected, pointer_type desired,
                               memory_order success,
                               memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(pointer_type& expected, pointer_type desired,
                               memory_order success,
                               memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_weak(expected, desired);
    }

    bool compare_exchange_weak(pointer_type& expected, pointer_type desired,
                               memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_weak(pointer_type& expected, pointer_type desired,
                               memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired, mo);
    }

    bool compare_exchange_strong(pointer_type& expected, pointer_type desired,
                                 memory_order success,
                                 memory_order failure) WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(pointer_type& expected, pointer_type desired,
                                 memory_order success,
                                 memory_order failure) volatile WEOS_NOEXCEPT
    {
        return compare_exchange_strong(expected, desired);
    }

    bool compare_exchange_strong(pointer_type& expected, pointer_type desired,
                                 memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        while (1)
        {
            pointer_type old = (pointer_type)__ldrex(&m_value);
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

    bool compare_exchange_strong(pointer_type& expected, pointer_type desired,
                                 memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        while (1)
        {
            pointer_type old = (pointer_type)__ldrex(&m_value);
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

    pointer_type fetch_add(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(pointer_type, +, arg);
    }

    pointer_type fetch_add(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(pointer_type, +, arg);
    }

    pointer_type fetch_sub(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(pointer_type, -, arg);
    }

    pointer_type fetch_sub(std::ptrdiff_t arg, memory_order mo = memory_order_seq_cst) volatile WEOS_NOEXCEPT
    {
        WEOS_ATOMIC_MODIFY(pointer_type, -, arg);
    }

    pointer_type operator= (pointer_type value) WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    pointer_type operator= (pointer_type value) volatile WEOS_NOEXCEPT
    {
        store(value);
        return value;
    }

    pointer_type operator++() WEOS_NOEXCEPT
    {
        return fetch_add(1) + 1;
    }

    pointer_type operator++() volatile WEOS_NOEXCEPT
    {
        return fetch_add(1) + 1;
    }

    pointer_type operator++ (int) WEOS_NOEXCEPT
    {
        return fetch_add(1);
    }

    pointer_type operator++ (int) volatile WEOS_NOEXCEPT
    {
        return fetch_add(1);
    }

    pointer_type operator--() WEOS_NOEXCEPT
    {
        return fetch_sub(1) - 1;
    }

    pointer_type operator--() volatile WEOS_NOEXCEPT
    {
        return fetch_sub(1) - 1;
    }

    pointer_type operator-- (int) WEOS_NOEXCEPT
    {
        return fetch_sub(1);
    }

    pointer_type operator-- (int) volatile WEOS_NOEXCEPT
    {
        return fetch_sub(1);
    }

    pointer_type operator+= (std::ptrdiff_t value) WEOS_NOEXCEPT
    {
        return fetch_add(value) + value;
    }

    pointer_type operator+= (std::ptrdiff_t value) volatile WEOS_NOEXCEPT
    {
        return fetch_add(value) + value;
    }

    pointer_type operator-= (std::ptrdiff_t value) WEOS_NOEXCEPT
    {
        return fetch_sub(value) - value;
    }

    pointer_type operator-= (std::ptrdiff_t value) volatile WEOS_NOEXCEPT
    {
        return fetch_sub(value) - value;
    }

    operator pointer_type() const WEOS_NOEXCEPT
    {
        return load();
    }

    operator pointer_type() const volatile WEOS_NOEXCEPT
    {
        return load();
    }

private:
    int m_value;

protected:
    // ---- Hidden methods
    atomic(const atomic&);
    atomic& operator=(const atomic&);
    atomic& operator=(const atomic&) volatile;
};

WEOS_END_NAMESPACE

#endif // WEOS_COMMON_ATOMIC_IMPL_ARMCC_HPP
