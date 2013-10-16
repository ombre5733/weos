/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
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

#include "../config.hpp"
#include "chrono.hpp"
#include "error.hpp"

#include <boost/config.hpp>
#include <boost/utility.hpp>

#include <cstdint>

namespace weos
{

struct defer_lock_t {};
struct try_to_lock_t {};
struct adopt_lock_t {};

BOOST_CONSTEXPR_OR_CONST defer_lock_t defer_lock = defer_lock_t();
BOOST_CONSTEXPR_OR_CONST try_to_lock_t try_to_lock = try_to_lock_t();
BOOST_CONSTEXPR_OR_CONST adopt_lock_t adopt_lock = adopt_lock_t();

namespace detail
{

// The header (first 32 bits) of the mutex control block. The full definition
// can be found in ../3rdparty/keil_cmsis_rtos/SRC/rt_TypeDef.h
struct MutexControlBlockHeader
{
    std::uint8_t controlBlockType;
    std::uint8_t ownerPriority;
    std::uint16_t nestingLevel;
};

template <typename DerivedT>
class basic_mutex : boost::noncopyable
{
public:
    // Create a generic mutex.
    basic_mutex()
    {
        os_mut_init(&m_mutex);
    }

    // Locks the mutex. Calls post_lock_check() after a successful lock.
    void lock()
    {
        OS_RESULT result = os_mut_wait(&m_mutex, 0xFFFF);
        if (result == OS_R_TMO)
        {
            ::weos::throw_exception(weos::system_error(result, rl_rtx_category()));
        }
        derived()->post_lock_check(mutexControlBlockHeader());
    }

    // Tries to lock the mutex. If successful, returns the result of calling
    // post_try_lock_correction().
    bool try_lock()
    {
        OS_RESULT result = os_mut_wait(&m_mutex, 0);
        if (result == OS_R_OK)
        {
            return derived()->post_try_lock_correction(
                        m_id, mutexControlBlockHeader());
        }

        return false;
    }

    void unlock()
    {
        OS_RESULT result = os_mut_release(&m_mutex);
        // Just check the return code but do not throw because unlock is
        // called from the destructor of lock_guard, for example.
        //! \todo I think, we can throw exceptions, too.
        TODO:
        WEOS_ASSERT(result == OS_R_OK);
        if (result != OS_R_OK)
        {
            throw;
        }
    }

protected:
    OS_MUT m_mutex;

    MutexControlBlockHeader* mutexControlBlockHeader()
    {
        TODO: check this
        return reinterpret_cast<MutexControlBlockHeader*>(
                    m_cmsisMutexControlBlock);
    }

    DerivedT* derived()
    {
        return static_cast<DerivedT*>(this);
    }

    void post_lock_check(MutexControlBlockHeader* /*mucb*/)
    {
    }

    bool post_try_lock_correction(OS_MUT& /*mutex*/,
                                  MutexControlBlockHeader* /*mucb*/)
    {
        return true;
    }
};

// A helper to lock a mutex.
struct mutex_try_locker
{
    mutex_try_locker(OS_MUT& mutex)
        : m_mutex(mutex)
    {
    }

    // Tries to lock a mutex up to \p millisec milliseconds. If a mutex
    // has been locked, the method returns \p true to signal that no further
    // waiting is necessary.
    bool operator() (std::int32_t millisec) const
    {
        OS_RESULT result = os_mut_wait(&m_mutex, millisec);
        return result != OS_R_TMO;
    }

private:
    OS_MUT& m_mutex;
};

template <typename DerivedT>
class basic_timed_mutex : public basic_mutex<DerivedT>
{
    typedef basic_mutex<DerivedT> base;

public:
    //! Tries to lock the mutex.
    //! Tries to lock the mutex and returns either when it has been locked or
    //! the duration \p d has expired. The method returns \p true, if the
    //! mutex has been locked.
    template <typename RepT, typename PeriodT>
    bool try_lock_for(const chrono::duration<RepT, PeriodT>& d)
    {
        mutex_try_locker locker(base::m_id);
        if (chrono::detail::cmsis_wait<
                RepT, PeriodT, mutex_try_locker>::wait(d, locker))
        {
            return base::derived()->post_try_lock_correction(
                        base::m_mutex, base::mutexControlBlockHeader());
        }

        return false;
    }

    template <typename ClockT, typename DurationT>
    bool try_lock_until(const chrono::time_point<ClockT, DurationT>& timePoint)
    {
        // Not implemented, yet.
        WEOS_ASSERT(false);
        return false;
    }
};

//! This adapter turns a recursive mutey into a non-recursive one.
template <typename BaseT>
class nonrecursive_adapter : public BaseT
{
public:
    void post_lock_check(MutexControlBlockHeader* mucb)
    {
        WEOS_ASSERT(mucb->nestingLevel == 1);
    }

    bool post_try_lock_correction(OS_MUT& mutex, MutexControlBlockHeader* mucb)
    {
        if (mucb->nestingLevel == 1)
            return true;

        WEOS_ASSERT(mucb->nestingLevel == 2);
        OS_RESULT result = os_mut_release(&mutex);
        WEOS_ASSERT(result == OS_R_OK);
        return false;
    }
};

} // namespace detail

class mutex
#ifndef WEOS_DOXYGEN_RUN
        : public detail::nonrecursive_adapter<detail::basic_mutex<mutex> >
#endif // WEOS_DOXYGEN_RUN
{
public:
#ifdef WEOS_DOXYGEN_RUN
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
#endif // WEOS_DOXYGEN_RUN
};

class timed_mutex
#ifndef WEOS_DOXYGEN_RUN
        : public detail::nonrecursive_adapter<
                     detail::basic_timed_mutex<timed_mutex> >
#endif // WEOS_DOXYGEN_RUN
{
public:
#ifdef WEOS_DOXYGEN_RUN
    //! Creates a mutex with support for timeout.
    timed_mutex();
    //! Destroys the mutex.
    ~timed_mutex();
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
#endif // WEOS_DOXYGEN_RUN
};

//! A recursive mutex.
class recursive_mutex
#ifndef WEOS_DOXYGEN_RUN
        : public detail::basic_mutex<recursive_mutex>
#endif // WEOS_DOXYGEN_RUN
{
public:
};

//! A recursive mutex with support for timeout.
class recursive_timed_mutex
#ifndef WEOS_DOXYGEN_RUN
        : public detail::basic_timed_mutex<recursive_timed_mutex>
#endif // WEOS_DOXYGEN_RUN
{
public:
};

//! A lock guard for RAII-style mutex locking.
template <class MutexT>
class lock_guard : boost::noncopyable
{
public:
    typedef MutexT mutex_type;

    //! Creates a lock guard.
    //! Creates a lock guard and locks the given \p mutex.
    explicit lock_guard(mutex_type& mutex)
        : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    //! Creates a lock guard which adopts a lock.
    //! Creates a lock guard for a \p mutex but does not lock the mutex. Instead
    //! the calling thread must have locked the mutex before creating the
    //! guard.
    //! The guard will still unlock the mutex when it goes out of scope.
    lock_guard(mutex_type& mutex, adopt_lock_t /*tag*/)
        : m_mutex(mutex)
    {
    }

    //! Destroys the lock guard.
    //! Destroys the lock guard and thereby unlocks the guarded mutex.
    ~lock_guard()
    {
        m_mutex.unlock();
    }

private:
    //! The mutex which is guarded.
    mutex_type& m_mutex;
};

//! A unique lock for a mutex.
template <class Mutex>
class unique_lock
{
public:
    typedef Mutex mutex_type;

    //! Creates a lock which is not associated with a mutex.
    unique_lock() BOOST_NOEXCEPT
        : m_mutex(0),
          m_locked(false)
    {
    }

    //! Creates a unique lock with locking.
    //! Creates a unique lock tied to the \p mutex and locks it.
    explicit unique_lock(mutex_type& mutex)
        : m_mutex(&mutex),
          m_locked(false)
    {
        mutex.lock();
        m_locked = true;
    }

    //! Creates a unique lock without locking.
    //! Creates a unique lock which will be tied to the given \p mutex but
    //! does not lock this mutex.
    unique_lock(mutex_type& mutex, defer_lock_t /*tag*/) BOOST_NOEXCEPT
        : m_mutex(&mutex),
          m_locked(false)
    {
    }

    unique_lock(mutex_type& mutex, try_to_lock_t /*tag*/)
        : m_mutex(&mutex),
          m_locked(false)
    {
        m_locked = mutex.try_lock();
    }

    //! Creates a unique lock for a locked mutex.
    //! Creates a unique lock for the given \p mutex. The constructor does
    //! not lock the mutex but assumes that it has already been locked
    //! by the caller.
    unique_lock(mutex_type& mutex, adopt_lock_t /*tag*/)
        : m_mutex(&mutex),
          m_locked(true)
    {
    }

    //! \todo Timed constructors are missing

    //! Destroys the unique lock.
    //! If the lock has an associated mutex and has locked this mutex, the
    //! mutex is unlocked.
    ~unique_lock()
    {
        if (m_locked)
            m_mutex->unlock();
    }

    //! Locks the associated mutex.
    void lock()
    {
        if (!m_mutex)
            ::weos::throw_exception(::weos::system_error(-1, cmsis_category()));//! \todo std::system_error);
        m_mutex->lock();
    }

    //! Returns a pointer to the associated mutex.
    //! Returns a pointer to the mutex to which this lock is tied. This may
    //! be a null-pointer, if no mutex has been supplied so far.
    mutex_type* mutex() const BOOST_NOEXCEPT
    {
        return m_mutex;
    }

    //! Checks if this lock owns a locked mutex.
    //! Returns \p true, if a mutex is tied to this lock and the lock has
    //! ownership of it.
    bool owns_lock() const BOOST_NOEXCEPT
    {
        return m_locked;
    }

    //! Releases the mutex without unlocking.
    //! Breaks the association of this lock and its mutex (which is returned
    //! by this function). The lock won't interact with the mutex any longer
    //! (it won't even unlock the mutex). Instead the responsibility is
    //! transfered to the caller.
    mutex_type* release() BOOST_NOEXCEPT
    {
        mutex_type* m = m_mutex;
        m_mutex = 0;
        m_locked = false;
        return m;
    }

    //! Unlocks the associated mutex.
    void unlock()
    {
        if (!m_locked)
            ::weos::throw_exception(::weos::system_error(-1, cmsis_category()));//! \todo std::system_error);
        m_mutex->unlock();
        m_locked = false;
    }

private:
    //! A pointer to the associated mutex.
    mutex_type* m_mutex;
    //! A flag indicating if the mutex has been locked.
    bool m_locked;
};

} // namespace weos

#endif // WEOS_KEIL_RL_RTX_MUTEX_HPP