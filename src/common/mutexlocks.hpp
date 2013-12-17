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

#ifndef WEOS_COMMON_MUTEXLOCKS_HPP
#define WEOS_COMMON_MUTEXLOCKS_HPP

#include "../config.hpp"

#include <boost/utility.hpp>

namespace weos
{

struct defer_lock_t {};
struct try_to_lock_t {};
struct adopt_lock_t {};

BOOST_CONSTEXPR_OR_CONST defer_lock_t defer_lock = defer_lock_t();
BOOST_CONSTEXPR_OR_CONST try_to_lock_t try_to_lock = try_to_lock_t();
BOOST_CONSTEXPR_OR_CONST adopt_lock_t adopt_lock = adopt_lock_t();

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

#endif // WEOS_COMMON_MUTEXLOCKS_HPP
