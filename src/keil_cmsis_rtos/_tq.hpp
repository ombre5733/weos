/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2016, Manuel Freiberger
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

#ifndef WEOS_KEIL_CMSIS_RTOS_TQ_HPP
#define WEOS_KEIL_CMSIS_RTOS_TQ_HPP

#ifndef WEOS_CONFIG_HPP
    #error "Do not include this file directly."
#endif // WEOS_CONFIG_HPP


#include "../atomic.hpp"
#include "../chrono.hpp"
#include "../semaphore.hpp"

#include <cstddef>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

struct _tq
{
    struct _t
    {
        _t(_tq& q);

        ~_t()
        {
            unlink();
        }

        _t(const _t&) = delete;
        _t& operator=(const _t&) = delete;

        bool unlink() noexcept;

        explicit
        operator bool() const noexcept
        {
            return m_v.load() & 1;
        }

        void wait()
        {
            m_s.wait();
        }

        template <typename TRep, typename TPeriod>
        inline
        bool wait_for(const chrono::duration<TRep, TPeriod>& timeout)
        {
            return m_s.try_wait_for(timeout);
        }

        template <typename TClock, typename TDuration>
        inline
        bool wait_until(const chrono::time_point<TClock, TDuration>& time)
        {
            return m_s.try_wait_until(time);
        }

        _tq& m_tq;
        semaphore m_s;
        atomic<std::uintptr_t> m_v;
        osPriority m_p;
    };



    _tq() = default;

    _tq(const _tq&) = delete;
    _tq& operator=(const _tq&) = delete;

    void notify_one() noexcept;
    void notify_all() noexcept;



    atomic<_t*> m_h{nullptr};
};

} // namespace weos_detail

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_TQ_HPP
