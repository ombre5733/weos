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

#ifndef CMSIS_THREADS_H
#define CMSIS_THREADS_H

//#include <time.h>

//! An enumeration of return codes.
//! - thrd_success: Indicates that the requested operation succeeded.
//! - thrd_error: Indicates that the requested operation failed.
//! - thrd_busy: Indicates that an operation failed because the resource
//!   which has been tested is already in use.
//! - thrd_nomem: Indicates that an operation failed because it was not able
//!   to allocate memory.
//! - thrd_timedout: Is returned by a timed wait if the function returned
//!   reaching a timeout without acquiring the resource.
enum
{
    thrd_success,
    thrd_error,
    thrd_busy,
    thrd_nomem,
    thrd_timedout
};

// ----=====================================================================----
//     Mutex
// ----=====================================================================----

//! An enumeration of mutex types.
//! - mtx_plain: A mutex which is neither recursive nor does it support timeout.
//! - mtx_recursive: A mutex which is recursive but does not support timeout.
//! - mtx_timed: Can be OR'ed with mtx_plain or mtx_recursive to create a mutex
//!   with support for timeout.
enum
{
    mtx_plain     = 0x01,
    mtx_recursive = 0x02,
    mtx_timed     = 0x04
};

//! A mutex.
//! A mutex is a data structure to prevent concurrent access to shared
//! rssources. Before a thread enters a critical section, it must lock
//! the mutex via mtx_lock(). When the thread leaves the critical section,
//! it must unlock the mutex using mtx_unlock(). Thus, locking and unlocking
//! of a mutex must happen in the same thread.
typedef struct
{
    osMutexDef(m_mutexDef);
    osMutexId m_id;
} mtx_t;

//! Destroys a mutex.
//! Destroys the mutex \p mtx and releases associated resources. It is an
//! error to destroy a mutex which is blocked by a thread.
void mtx_destroy(mtx_t* mtx);

//! Initializes a mutex.
//! Initializes the mutex \p mtx to be of type \p type. The \p type must be
//! one of the following:
//! - mtx_plain: creates a non-recursive mutex
//! - mtx_timed: creates a non-recursive mutex with support for timeout
//! - mtx_plain | mtx_recursive: creates a recursive mutex
//! - mtx_timed | mtx_recursive: creates a recursive mutex with support for
//!   timeout
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int mtx_init(mtx_t* mtx, int type);

//! Locks a mutex.
//! Blocks the current thread until the mutex \p mtx has been locked by it.
//! It is an error to lock a non-recursive mutex (i.e. a mutex created without
//! mtx_recursive) if the calling thread has already locked the mutex.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
//!
//! \sa mtx_trylock()
int mtx_lock(mtx_t* mtx);

int mtx_timedlock(mtx_t* restrict mtx, const struct timespec* restrict ts);

//! Tests and locks a mutex if it is available.
//! If the mutex \p mtx is available, it is locked by the calling thread. If
//! \p mtx is already locked, the function returns without blocking.
//!
//! The function returns thrd_success if the mutex has been locked by the
//! caller. If the mutex has not been available, thrd_busy is returned. If an
//! error occurred, thrd_error is returned.
int mtx_trylock(mtx_t* mtx);

//! Unlocks a mutex.
//! Unlocks the mutex \p mtx which must have been locked previously by the
//! calling thread.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int mtx_unlock(mtx_t* mtx);

// ----=====================================================================----
//     Condition variable
// ----=====================================================================----

//! A condition variable.
//! A condition variable provides a synchronization mechanism to block one
//! or multiple threads until an event occurs. The event may be
//! - a signal sent from another thread,
//! - the expiration of a timeout or
//! - a spurious wakeup.
typedef struct
{
    cnd_node_t* queue;
    mtx_t queueMutex;
} cnd_t;

//! Send a broadcast via a condition variable.
//! Unblocks all of the threads which block on the condition variable \p cond.
//! If no thread blocks on the condition variable when a signal is sent, the
//! function does nothing and returns with success.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int cnd_broadcast(cnd_t* cond);

int cnd_destroy(cnd_t* cond);

//! Initializes a condition variable.
//! Initializes the condition variable \p cond.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int cnd_init(cnd_t* cond);

//! Send a signal via a condition variable.
//! Unblocks one of the threads which block on the condition variable \p cond.
//! If no thread blocks on the condition variable when a signal is sent, the
//! function does nothing and returns with success.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int cnd_signal(cnd_t* cond);

//! Blocks on a condition variable after unlocking a mutex.
//!
//! Unlocks the mutex \p mtx and blocks on the condition variable \p cond.
//! The mutex must have been locked by the calling thread.
//! Upon a successful return, \p mtx will have been locked again by the
//! calling thread.
//!
//! Unlocking \p mtx and blocking on \p cond happens atomically. This means
//! it is not possible for another thread to lock \p mtx and call
//! cnd_signal() or cnd_broadcast() on \p cond before the current thread
//! blocks on \p cond. In other words, if another thread obtains the
//! mutex \p mtx and sends a signal via \p cond, the current thread will
//! always see this signal.
//!
//! Note: Even if the current thread blocks successfully on the condition
//! variable \p cond, it is possible that the thread gets woken up (this
//! function returns) without receiving a signal from another thread. This is
//! called a spurious wakeup. The caller code has to be able to deal with such
//! "false positive" notifications.
//!
//! The function returns thrd_success on success and thrd_error otherwise.
int cnd_wait(cnd_t* cond, mtx_t* mtx);

// ----=====================================================================----
//     Thread
// ----=====================================================================----

typedef osThreadId thrd_t;
typedef int (*thread_start_t)(void*);

#endif // CMSIS_THREADS_H
