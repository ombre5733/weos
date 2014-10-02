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

#ifndef WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
#define WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP

#include "core.hpp"

#include "chrono.hpp"
#include "system_error.hpp"
#include "type_traits.hpp"
#include "../common/mutexlocks.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace detail
{
// Just enough memory to hold a CMSIS mutex. This is the type to which
// the macro osMutexDef() defined in <cmsis_os.h> expands.
struct mutex_control_block_type
{
    std::uint32_t _[4];
};

} // namespace detail

//! A plain mutex.
class mutex
{
public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    mutex()
        : m_id(0),
          m_locked(false)
    {
        // Keil's RTOS wants a zero'ed control block type for initialization.
        m_cmsisMutexControlBlock._[0] = 0;
        osMutexDef_t mutexDef = { m_cmsisMutexControlBlock._ };
        m_id = osMutexCreate(&mutexDef);
        if (m_id == 0)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                    "mutex::mutex failed");
        }
    }

    //! Destroys the mutex.
    ~mutex()
    {
        if (m_id)
        {
            WEOS_ASSERT(!m_locked);
            osMutexDelete(m_id);
        }
    }

    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //! It is undefined behaviour, if the calling thread has already acquired
    //! the mutex and wants to lock it again.
    //!
    //! \sa try_lock()
    void lock()
    {
        osStatus result = osMutexWait(m_id, osWaitForever);
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "mutex::lock failed");

        if (!m_locked)
        {
            m_locked = true;
        }
        else
        {
            // The mutex has been locked twice. Undo one lock and throw an
            // exception.
            result = osMutexRelease(m_id);
            if (result != osOK)
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "mutex::lock failed");
            WEOS_THROW_SYSTEM_ERROR(errc::resource_deadlock_would_occur,
                                    "deadlock in mutex::lock");
        }
    }

    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns \p false without blocking.
    bool try_lock()
    {
        osStatus result = osMutexWait(m_id, 0);

        // If osMutexWait(mutex, millisec) fails to acquire a mutex within the
        // timeout, the error code is
        //   osErrorTimeoutResource if millisec != 0
        //   osErrorResource if millisec == 0
        if (result == osErrorResource)
        {
            // The mutex is owned by another thread.
            return false;
        }
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "mutex::try_lock failed");

        if (!m_locked)
        {
            m_locked = true;
            return true;
        }
        else
        {
            // The mutex has already been locked by the current thread. Unlock
            // it once and pretend that try_lock() failed.
            result = osMutexRelease(m_id);
            if (result != osOK)
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "mutex::try_lock failed");
            return false;
        }
    }

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock()
    {
        WEOS_ASSERT(m_locked);
        m_locked = false;
        osStatus result = osMutexRelease(m_id);
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "mutex::unlock failed");
    }

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return m_id;
    }

protected:
    //! The native mutex.
    detail::mutex_control_block_type m_cmsisMutexControlBlock;
    //! The native mutex handle.
    osMutexId m_id;
    //! This flag is set, if the current thread has already blocked the mutex.
    bool m_locked;

private:
    // ---- Hidden methods.
    mutex(const mutex&);
    const mutex& operator= (const mutex&);
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
        typedef typename WEOS_NAMESPACE::common_type<
                             typename TClock::duration,
                             TDuration>::type difference_type;
        typedef chrono::detail::internal_time_cast<difference_type> caster;

        while (true)
        {
            typename caster::type millisecs
                    = caster::convert_and_clip(time - TClock::now());

            osStatus result = osMutexWait(m_id, millisecs);
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
                    result = osMutexRelease(m_id);
                    if (result != osOK)
                    {
                        WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                                "timed_mutex::try_lock_until failed");
                    }
                }
            }
            if (   result != osErrorResource
                && result != osErrorTimeoutResource)
            {
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "timed_mutex::try_lock_until failed");
            }

            if (millisecs == 0)
                return false;
        }
    }
};

//! A recursive mutex.
class recursive_mutex
{
public:
    //! The type of the native mutex handle.
    typedef osMutexId native_handle_type;


    //! Creates a mutex.
    recursive_mutex()
        : m_id(0)
    {
        // Keil's RTOS wants a zero'ed control block type for initialization.
        m_cmsisMutexControlBlock._[0] = 0;
        osMutexDef_t mutexDef = { m_cmsisMutexControlBlock._ };
        m_id = osMutexCreate(&mutexDef);
        if (m_id == 0)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                    "recursive_mutex::recursive_mutex failed");
        }
    }

    //! Destroys the mutex.
    ~recursive_mutex()
    {
        if (m_id)
            osMutexDelete(m_id);
    }

    //! Locks the mutex.
    //!
    //! Blocks the current thread until this mutex has been locked by it.
    //! The mutex can be locked recursively by one thread and must be unlocked
    //! as often as it has been locked before.
    //!
    //! \sa try_lock()
    void lock()
    {
        osStatus result = osMutexWait(m_id, osWaitForever);
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "recursive_mutex::lock failed");
    }

    //! Tests and locks the mutex if it is available.
    //!
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. Locking may be done recursively.
    //!
    //! \sa lock()
    bool try_lock()
    {
        osStatus result = osMutexWait(m_id, 0);

        // If osMutexWait(mutex, millisec) fails to acquire a mutex within the
        // timeout, the error code is
        //   osErrorTimeoutResource if millisec != 0
        //   osErrorResource if millisec == 0
        if (result == osErrorResource)
        {
            // The mutex is owned by another thread.
            return false;
        }
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "recursive_mutex::try_lock failed");

        return true;
    }

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock()
    {
        osStatus result = osMutexRelease(m_id);
        if (result != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                    "recursive_mutex::unlock failed");
    }

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return m_id;
    }

protected:
    //! The native mutex.
    detail::mutex_control_block_type m_cmsisMutexControlBlock;
    //! The native mutex handle.
    osMutexId m_id;

private:
    // ---- Hidden methods.
    recursive_mutex(const recursive_mutex&);
    const recursive_mutex& operator= (const recursive_mutex&);
};

//! A recursive mutex with timeout support.
class recursive_timed_mutex : public recursive_mutex
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
        typedef typename WEOS_NAMESPACE::common_type<
                             typename TClock::duration,
                             TDuration>::type difference_type;
        typedef chrono::detail::internal_time_cast<difference_type> caster;

        while (true)
        {
            typename caster::type millisecs
                    = caster::convert_and_clip(time - TClock::now());

            osStatus result = osMutexWait(m_id, millisecs);
            if (result == osOK)
                return true;

            if (   result != osErrorResource
                && result != osErrorTimeoutResource)
            {
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result),
                                        "recursive_timed_mutex::try_lock_until failed");
            }

            if (millisecs == 0)
                return false;
        }
    }
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
