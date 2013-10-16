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

#ifndef WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
#define WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP

#include "../config.hpp"
#include "chrono.hpp"
#include "error.hpp"
#include "../common/mutexlocks.hpp"

#include <boost/config.hpp>

#include <cstdint>

namespace weos
{

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
        : m_id(0)
    {
        // Keil's RTOS wants a zero'ed control block type for initialization.
        m_cmsisMutexControlBlock[0] = 0;
        osMutexDef_t mutexDef = { m_cmsisMutexControlBlock };
        m_id = osMutexCreate(&mutexDef);
        if (m_id == 0)
            ::weos::throw_exception(weos::system_error(osErrorOS, cmsis_category()));
    }

    // Destroys the mutex.
    ~basic_mutex()
    {
        if (m_id)
            osMutexDelete(m_id);
    }

    // Locks the mutex. Calls post_lock_check() after a successful lock.
    void lock()
    {
        osStatus status = osMutexWait(m_id, osWaitForever);
        if (status != osOK)
        {
            ::weos::throw_exception(weos::system_error(status, cmsis_category()));
        }
        derived()->post_lock_check(mutexControlBlockHeader());
    }

    // Tries to lock the mutex. If successful, returns the result of calling
    // post_try_lock_correction().
    bool try_lock()
    {
        osStatus status = osMutexWait(m_id, 0);
        if (status == osOK)
        {
            return derived()->post_try_lock_correction(
                        m_id, mutexControlBlockHeader());
        }

        if (   status != osErrorTimeoutResource
            && status != osErrorResource)
        {
            ::weos::throw_exception(weos::system_error(status, cmsis_category()));
        }

        return false;
    }

    void unlock()
    {
        osStatus status = osMutexRelease(m_id);
        // Just check the return code but do not throw because unlock is
        // called from the destructor of lock_guard, for example.
        //! \todo I think, we can throw exceptions, too.
        WEOS_ASSERT(status == osOK);
    }

protected:
    std::uint32_t m_cmsisMutexControlBlock[3];
    osMutexId m_id;

    MutexControlBlockHeader* mutexControlBlockHeader()
    {
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

    bool post_try_lock_correction(osMutexId /*id*/,
                                  MutexControlBlockHeader* /*mucb*/)
    {
        return true;
    }
};

// A helper to lock a mutex.
struct mutex_try_locker
{
    mutex_try_locker(osMutexId id)
        : m_id(id)
    {
    }

    // Tries to lock a mutex up to \p millisec milliseconds. If a mutex
    // has been locked, the method returns \p true to signal that no further
    // waiting is necessary.
    bool operator() (std::int32_t millisec) const
    {
        osStatus status = osMutexWait(m_id, millisec);
        if (status == osOK)
            return true;

        if (   status != osErrorTimeoutResource
            && status != osErrorResource)
        {
            ::weos::throw_exception(weos::system_error(status, cmsis_category()));
        }

        return false;
    }

private:
    osMutexId m_id;
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
                        base::m_id, base::mutexControlBlockHeader());
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

//! This adapter turns a recursive mutex into a non-recursive one.
template <typename BaseT>
class nonrecursive_adapter : public BaseT
{
public:
    void post_lock_check(MutexControlBlockHeader* mucb)
    {
        WEOS_ASSERT(mucb->nestingLevel == 1);
    }

    bool post_try_lock_correction(osMutexId id, MutexControlBlockHeader* mucb)
    {
        if (mucb->nestingLevel == 1)
            return true;

        WEOS_ASSERT(mucb->nestingLevel == 2);
        osStatus status = osMutexRelease(id);
        WEOS_ASSERT(status == osOK);
        return false;
    }
};

} // namespace detail

//! A plain mutex.
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

//! A plain mutex with timeout.
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

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_MUTEX_HPP
