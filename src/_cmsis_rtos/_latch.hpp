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

#ifndef WEOS_CMSIS_RTOS_LATCH_HPP
#define WEOS_CMSIS_RTOS_LATCH_HPP

#include "_core.hpp"
#include "_tq.hpp"
#include "../atomic.hpp"

#include <cstddef>


WEOS_BEGIN_NAMESPACE

//! A latch.
//!
//! A latch is a downward counter which is initialized upon creation.
//! Threads may block on this latch and will be released once the counter
//! decrements to zero. A latch cannot be reset.
class latch
{
public:
    //! Creates a latch whose counter is set to \p count (>= 0).
    explicit
    latch(std::ptrdiff_t count)
        : m_count(count)
    {
    }

    latch(const latch&) = delete;
    latch(latch&&) = delete;

    latch& operator=(const latch&) = delete;
    latch& operator=(latch&&) = delete;

    //! Destroys the latch.
    //!
    //! \note The destructor may only be called if no thread blocks on
    //! the latch or if the latch counter has reached zero.
    //!
    //! \note The caller is responsible to ensure that no thread calls
    //! wait() after one thread has invoked the destructor.
    ~latch()
    {
        m_tq.notify_all();
    }

    //! Decrements the counter by one. If it is non-zero then, the thread
    //! blocks until the latch is triggered.
    void count_down_and_wait()
    {
        weos_detail::_tq::_t t(m_tq);
        if (--m_count > 0)
            t.wait();
    }

    //! Decrements the counter by \p n (0 <= n <= counter).
    //!
    //! \note This method may be called in an interrupt context.
    void count_down(std::ptrdiff_t n)
    {
        std::ptrdiff_t c = m_count.load();
        std::ptrdiff_t d;
        do
        {
            d = c - n;
        } while (!m_count.compare_exchange_weak(c, d));
        if (d == 0)
            m_tq.notify_all();
    }

    //! Returns \p true, if the counter has reached zero.
    bool is_ready() const noexcept
    {
        return m_count == 0;
    }

    //! Blocks the calling thread until the counter has reached zero. If
    //! the value is already zero, this is a no-op.
    void wait() const
    {
        weos_detail::_tq::_t t(m_tq);
        if (m_count > 0)
            t.wait();
    }

private:
    atomic<std::ptrdiff_t> m_count;
    mutable weos_detail::_tq m_tq;
};

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_LATCH_HPP
