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

#ifndef WEOS_MUTEX_HPP
#define WEOS_MUTEX_HPP

#include "config.hpp"

#if defined(WEOS_WRAP_CXX11)
#  include "cxx11/mutex.hpp"
#elif defined(WEOS_WRAP_KEIL_CMSIS_RTOS)
#  include "keil_cmsis_rtos/mutex.hpp"
#elif defined(WEOS_WRAP_KEIL_RL_RTX)
#  include "keil_rl_rtx/mutex.hpp"
#else
#  error "The OS wrapper has not been configured."
#endif

#include "chrono.hpp"
#include <boost/utility.hpp>

namespace weos
{

//! A null-mutex.
//! The null-mutex is a class which implements the Lockable concept but
//! does not block a thread. It can be used as a stub for a mutex in
//! single-threaded applications.
class null_mutex : boost::noncopyable
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

    template <typename RepT, typename PeriodT>
    bool try_lock_for(const chrono::duration<RepT, PeriodT>& d)
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
