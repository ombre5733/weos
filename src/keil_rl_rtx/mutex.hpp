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

#ifndef WEOS_KEIL_RL_RTX_MUTEX_HPP
#define WEOS_KEIL_RL_RTX_MUTEX_HPP

#include "core.hpp"

#include "../chrono.hpp"
#include "../system_error.hpp"
#include "../common/mutexlocks.hpp"


WEOS_BEGIN_NAMESPACE


TODO: check all return codes once again

//! A plain mutex.
class mutex
{
public:
    //! The type of the native mutex handle.
    typedef OS_MUT native_handle_type;


    //! Creates a mutex.
    mutex();

    //! Destroys the mutex.
    ~mutex();

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
        return &m_mutex;
    }

protected:
    //! The native mutex.
    OS_MUT m_mutex;
    //! Keep track, if the mutex has already been locked by the current thread.
    bool m_locked;

private:
    // ---- Hidden methods.
    mutex(const mutex&);
    mutex& operator= (const mutex&);
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
            typename caster::type ticks
                    = caster::convert_and_clip(time - TClock::now());

            OS_RESULT result = os_mut_wait(&m_mutex, ticks);
            if (result != OS_R_TMO)
            {
                if (!m_locked)
                {
                    m_locked = true;
                    return true;
                }
                else
                {
                    // The mutex has been locked by the same thread again.
                    result = os_mut_release(&m_mutex);
                    if (result != OS_R_OK)
                    {
                        WEOS_THROW_SYSTEM_ERROR(result,
                                                "timed_mutex::try_lock_until failed");
                    }
                }
            }

            if (failure)
                throw;

            if (ticks == 0)
                return false;
        }
    }
};

//! A recursive mutex.
class recursive_mutex
{
public:
    //! The type of the native mutex handle.
    typedef OS_MUT native_handle_type;


    //! Creates a recursive mutex.
    recursive_mutex();

    //! Destroys the mutex.
    ~recursive_mutex();

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
    //!
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread. The owning thread must call unlock() as often as
    //! lock() or try_lock() has been called before.
    void unlock();

    //! Returns a native mutex handle.
    native_handle_type native_handle()
    {
        return &m_mutex;
    }

protected:
    //! The native mutex.
    OS_MUT m_mutex;

private:
    // ---- Hidden methods.
    recursive_mutex(const recursive_mutex&);
    recursive_mutex& operator= (const recursive_mutex&);
};

//! A recursive mutex with support for timeout.
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
        return try_lock_until(chrono::monotonic_clock::now() + timeout);
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
            typename caster::type ticks
                    = caster::convert_and_clip(time - TClock::now());

            OS_RESULT result = os_mut_wait(&m_mutex, ticks);
            if (result != OS_R_TMO)
                return true;

            if (failure)
            {
                WEOS_THROW_SYSTEM_ERROR(
                            result,
                            "recursive_timed_mutex::try_lock_until failed");
            }

            if (ticks == 0)
                return false;
        }
    }
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_RL_RTX_MUTEX_HPP
