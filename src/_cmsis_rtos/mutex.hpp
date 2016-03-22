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

#ifndef WEOS_CMSIS_RTOS_MUTEX_HPP
#define WEOS_CMSIS_RTOS_MUTEX_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../chrono.hpp"
#include "../system_error.hpp"
#include "../type_traits.hpp"
#include "../_common/mutexlocks.hpp"
#include "_sleep.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

//! A plain mutex.
class mutex
{
    // The CMSIS-RTOS control block (OS_MUCB from ${CMSIS-RTOS}/SRC/rt_TypeDef.h)
    // must have the following layout:
    // struct OS_MUCB
    // {
    //     uint8_t cb_type;
    //     uint16_t level;
    //     void* p_lnk;
    //     void* owner;
    //     void* p_mlnk;
    // };
    static_assert(osCMSIS_RTX <= ((4<<16) | 78), "Check the layout of OS_MUCB.");

public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    constexpr
    mutex() noexcept
        : m_cmsisMutexControlBlock{3 /* cb_type & level */, 0 /* p_lnk */, 0 /* owner */, 0 /* p_mlnk */},
          m_locked(false)
    {
    }

    //! Destroys the mutex.
    ~mutex();

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //! It is undefined behaviour, if the calling thread has already acquired
    //! the mutex and wants to lock it again.
    //!
    //! \sa try_lock()
    void lock();

    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns \p false without blocking.
    bool try_lock() noexcept;

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock() noexcept;

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return static_cast<native_handle_type>(
                    static_cast<void*>(m_cmsisMutexControlBlock));
    }

protected:
    //! The native mutex.
    std::uint32_t m_cmsisMutexControlBlock[4];
    //! This flag is set, if the current thread has already blocked the mutex.
    bool m_locked;
};

//! A mutex with timeout support.
class timed_mutex : public mutex
{
public:
    //! Tries to lock the mutex.
    //!
    //! Tries to lock the mutex and returns either when it has been locked or
    //! the duration \p timeout has expired. The method returns \p true, if the
    //! mutex has been locked.
    template <typename TRep, typename TPeriod>
    inline
    bool try_lock_for(const chrono::duration<TRep, TPeriod>& timeout)
    {
        return try_lock_until(chrono::steady_clock::now() + timeout);
    }

    //! Tries to lock the mutex.
    //!
    //! Tries to lock the mutex before the given \p time point. The return
    //! value is \p true, if the mutex has been locked.
    template <typename TClock, typename TDuration>
    bool try_lock_until(const chrono::time_point<TClock, TDuration>& time)
    {
        using namespace chrono;

        bool timeout = false;
        while (!timeout)
        {
            osStatus result;
            auto remainingSpan = time - TClock::now();
            if (remainingSpan <= TDuration::zero())
            {
                result = osMutexWait(native_handle(), 0);
                timeout = true;
            }
            else
            {
                milliseconds converted = duration_cast<milliseconds>(remainingSpan);
                if (converted < milliseconds(1))
                    converted = milliseconds(1);
                else if (converted > milliseconds(0xFFFE))
                    converted = milliseconds(0xFFFE);
                result = osMutexWait(native_handle(), converted.count());
            }

            if (result == osOK)
            {
                if (!m_locked)
                {
                    m_locked = true;
                    return true;
                }
                else
                {
                    // The mutex has been locked by the same thread again.
                    // Undo the last lock, then sleep until the time point
                    // has been reached and return with a failure.
                    result = osMutexRelease(native_handle());
                    if (result != osOK)
                    {
                        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                                "timed_mutex::try_lock_until failed");
                    }
                    if (!timeout)
                        weos_detail::sleep_until(time);
                    return false;
                }
            }

            if (   result != osErrorResource
                && result != osErrorTimeoutResource)
            {
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "timed_mutex::try_lock_until failed");
            }
        }

        return false;
    }
};

//! A recursive mutex.
class recursive_mutex
{
    // The CMSIS-RTOS control block (OS_MUCB from ${CMSIS-RTOS}/SRC/rt_TypeDef.h)
    // must have the following layout:
    // struct OS_MUCB
    // {
    //     uint8_t cb_type;
    //     uint16_t level;
    //     void* p_lnk;
    //     void* owner;
    //     void* p_mlnk;
    // };
    static_assert(osCMSIS_RTX <= ((4<<16) | 78), "Check the layout of OS_MUCB.");

public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    recursive_mutex()
        : m_cmsisMutexControlBlock{3 /* cb_type & level */, 0 /* p_lnk */, 0 /* owner */, 0 /* p_mlnk */}
    {
    }

    //! Destroys the mutex.
    ~recursive_mutex();

    recursive_mutex(const recursive_mutex&) = delete;
    recursive_mutex& operator=(const recursive_mutex&) = delete;

    //! Locks the mutex.
    //!
    //! Blocks the current thread until this mutex has been locked by it.
    //! The mutex can be locked recursively by one thread and must be unlocked
    //! as often as it has been locked before.
    //!
    //! \sa try_lock()
    void lock();

    //! Tests and locks the mutex if it is available.
    //!
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. Locking may be done recursively.
    //!
    //! \sa lock()
    bool try_lock() noexcept;

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock() noexcept;

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return static_cast<native_handle_type>(
                    static_cast<void*>(m_cmsisMutexControlBlock));
    }

protected:
    //! The native mutex.
    std::uint32_t m_cmsisMutexControlBlock[4];
};

//! A recursive mutex with timeout support.
class recursive_timed_mutex : public recursive_mutex
{
public:
    //! \cond
    //! Tries to lock the mutex with a timeout.
    //!
    //! This is overload for the case when the timeout is given in milliseconds.
    bool try_lock_for(chrono::milliseconds ms);
    //! \endcond

    //! Tries to lock the mutex.
    //!
    //! Tries to lock the mutex and returns either when it has been locked or
    //! the duration \p timeout has expired. The method returns \p true, if the
    //! mutex has been locked.
    template <typename TRep, typename TPeriod>
    inline
    bool try_lock_for(const chrono::duration<TRep, TPeriod>& timeout)
    {
        using namespace chrono;

        milliseconds converted = duration_cast<milliseconds>(timeout);
        if (converted < timeout)
            ++converted;

        return try_lock_for(converted);
    }

    //! Tries to lock the mutex.
    //!
    //! Tries to lock the mutex before the given \p time point. The return
    //! value is \p true, if the mutex has been locked.
    template <typename TClock, typename TDuration>
    bool try_lock_until(const chrono::time_point<TClock, TDuration>& time)
    {
        using namespace chrono;

        bool timeout = false;
        while (!timeout)
        {
            osStatus result;
            auto remainingSpan = time - TClock::now();
            if (remainingSpan <= TDuration::zero())
            {
                result = osMutexWait(native_handle(), 0);
                timeout = true;
            }
            else
            {
                milliseconds converted = duration_cast<milliseconds>(remainingSpan);
                if (converted < milliseconds(1))
                    converted = milliseconds(1);
                else if (converted > milliseconds(0xFFFE))
                    converted = milliseconds(0xFFFE);
                result = osMutexWait(native_handle(), converted.count());
            }

            if (result == osOK)
                return true;

            if (   result != osErrorResource
                && result != osErrorTimeoutResource)
            {
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "recursive_timed_mutex::try_lock_until failed");
            }
        }

        return false;
    }
};

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_MUTEX_HPP
