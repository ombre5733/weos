/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#ifndef CMSIS_MUTEX_HPP
#define CMSIS_MUTEX_HPP

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>

namespace osl
{

//! A recursive mutex with support for timeout.
class recursive_timed_mutex : boost::noncopyable
{
public:
    //! Creates a recursive mutex with support for timeout.
    recursive_timed_mutex()
    {
        m_id = osMutexCreate(osMutex(m_mutexDef));
        if (m_id == 0)
            throw ::boost::throw_exception(std::system_error());
    }

    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //!
    //! \sa try_lock()
    void lock()
    {
        osStatus status = osMutexWait(m_id, osWaitForever);
        if (status != osOK)
            throw ::boost::throw_exception(std::system_error());
    }

    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns without blocking with a value of \p false.
    bool try_lock()
    {
        osStatus status = osMutexWait(m_id, 0);
        if (status == osOK)
            return true;
        else
            return false;
    }

    //bool try_lock_for(duration);

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock()
    {
        osStatus status = osMutexRelease(m_id);
        if (status != osOK)
            throw -1; // or don't throw --> this could be called from a destructor?
    }

private:
    osMutexDef(m_mutexDef);
    osMutexId m_id;
};

struct defer_lock_t {};
struct try_to_lock_t {};
struct adopt_lock_t {};

//! A lock guard for RAII-style mutex locking.
template <class Mutex>
class lock_guard : boost::noncopyable
{
public:
    typedef Mutex mutex_type;

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
    lock_guard(mutex_type& mutex, adopt_lock_t adopt_tag)
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

template <class Mutex>
class unique_lock
{
public:
    typedef Mutex mutex_type;

    unique_lock();
    explicit unique_lock(mutex_type& mutex);

    ~unique_lock();

    bool owns_lock() const;

private:
    mutex_type& m_mutex;
};

} // namespace osl

#endif // CMSIS_MUTEX_HPP
