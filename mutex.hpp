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

#ifndef WEOS_MUTEX_HPP
#define WEOS_MUTEX_HPP

#include "config.hpp"

#if defined(WEOS_WRAP_CXX11)
#  include "cxx11/mutex.hpp"
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#  include "keil_cmsis_rtos/mutex.hpp"
#else
#  error "The OS wrapper has not been configured."
#endif

#include <boost/utility.hpp>

namespace weos
{

//! A null-mutex.
//! The null-mutex is a class which implements the Lockable concept but
//! does not block a thread. It can be used as a stub for a mutex in
//! single-threaded applications.
class null_mutex: boost::noncopyable
{
public:
    //! Locks the null-mutex.
    void lock()
    {
    }

    //! Tries to lock the null-mutex which always succeeds.
    bool try_lock()
    {
        return true;
    }

    //! Unlocks the null-mutex.
    void unlock()
    {
    }
};

} // namespace weos

#endif // WEOS_MUTEX_HPP
