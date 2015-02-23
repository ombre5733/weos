/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2015, Manuel Freiberger
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

#ifndef WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
#define WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP


#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../chrono.hpp"
#include "../system_error.hpp"
#include "../type_traits.hpp"
#include "../common/mutexlocks.hpp"
#include "_sleep.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{
// Just enough memory to hold a CMSIS mutex. This is the type to which
// the macro osMutexDef() defined in <cmsis_os.h> expands.
struct mutex_control_block_type
{
    std::uint32_t _[4];
};

} // namespace weos_detail

//! A plain mutex.
class mutex
{
public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    mutex();

    //! Destroys the mutex.
    ~mutex();

    mutex(const mutex&) = delete;
    const mutex& operator=(const mutex&) = delete;

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
    bool try_lock();

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock();

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return m_id;
    }

protected:
    //! The native mutex.
    weos_detail::mutex_control_block_type m_cmsisMutexControlBlock;
    //! The native mutex handle.
    osMutexId m_id;
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
                result = osMutexWait(m_id, 0);
                timeout = true;
            }
            else
            {
                milliseconds converted = duration_cast<milliseconds>(remainingSpan);
                if (converted < milliseconds(1))
                    converted = milliseconds(1);
                else if (converted > milliseconds(0xFFFE))
                    converted = milliseconds(0xFFFE);
                result = osMutexWait(m_id, converted.count());
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
                    result = osMutexRelease(m_id);
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
public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    recursive_mutex();

    //! Destroys the mutex.
    ~recursive_mutex();

    recursive_mutex(const recursive_mutex&) = delete;
    const recursive_mutex& operator=(const recursive_mutex&) = delete;

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
    bool try_lock();

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock();

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return m_id;
    }

protected:
    //! The native mutex.
    weos_detail::mutex_control_block_type m_cmsisMutexControlBlock;
    //! The native mutex handle.
    osMutexId m_id;
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
                result = osMutexWait(m_id, 0);
                timeout = true;
            }
            else
            {
                milliseconds converted = duration_cast<milliseconds>(remainingSpan);
                if (converted < milliseconds(1))
                    converted = milliseconds(1);
                else if (converted > milliseconds(0xFFFE))
                    converted = milliseconds(0xFFFE);
                result = osMutexWait(m_id, converted.count());
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

#endif // WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
