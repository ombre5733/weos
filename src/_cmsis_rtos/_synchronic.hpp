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

#ifndef WEOS_CMSIS_RTOS_SYNCHRONIC_HPP
#define WEOS_CMSIS_RTOS_SYNCHRONIC_HPP

#include "_core.hpp"
#include "_tq.hpp"
#include "../atomic.hpp"
#include "../chrono.hpp"


WEOS_BEGIN_NAMESPACE

enum notify_hint
{
    notify_all,
    notify_one
};

enum expect_hint
{
    expect_urgent,
    expect_delay
};


template <typename T>
class synchronic
{
public:
    using atomic_type = std::atomic<T>;

    synchronic() = default;
    ~synchronic() = default;

    synchronic(const synchronic&) = delete;
    synchronic& operator=(const synchronic&) = delete;

    //! Updates the content of \p object to \p value using the provided
    //! memory order. Depending on \p hint, a notification to either one or
    //! all threads blocking on the \p object.
    void notify(atomic_type& object, T value,
                std::memory_order order = std::memory_order_seq_cst,
                notify_hint hint = notify_all) noexcept
    {
        object.store(value, order);
        if (hint == notify_all)
            m_tq.notify_all();
        else
            m_tq.notify_one();
    }

    template <typename F>
    void notify(atomic_type& /*object*/, F&& func, notify_hint hint = notify_all)
    {
        func();
        if (hint == notify_all)
            m_tq.notify_all();
        else
            m_tq.notify_one();
    }

    //! Blocks the current thread until the content of \p object equals
    //! \p desired. Loading the current object value happens with the
    //! provided memory \p order. The caller can signal via \p hint if an
    //! timely or a delayed update is expected.
    void expect(const atomic_type& object, T desired,
                std::memory_order order = std::memory_order_seq_cst,
                expect_hint /*hint*/ = expect_urgent) const noexcept
    {
        for (;;)
        {
            WEOS_NAMESPACE::weos_detail::_tq::_t t(m_tq);
            if (object.load(order) == desired)
                return;
            else
                t.wait();
        }
    }

    //! Blocks the current thread until \p pred() returns \p true. The caller
    //! can signal via \p hint if an timely or a delayed update is expected.
    template <typename F>
    void expect(const atomic_type& /*object*/, F&& pred,
                expect_hint /*hint*/ = expect_urgent) const
    {
        for (;;)
        {
            WEOS_NAMESPACE::weos_detail::_tq::_t t(m_tq);
            if (pred())
                return;
            else
                t.wait();
        }
    }

    //! Blocks the current thread until the content of \p object does not
    //! equal \p current. Loading the current object value happens with the
    //! provided memory \p order. The caller can signal via \p hint if an
    //! timely or a delayed update is expected.
    void expect_update(const atomic_type& object, T current,
                       std::memory_order order = std::memory_order_seq_cst,
                       expect_hint /*hint*/ = expect_urgent) const noexcept
    {
        for (;;)
        {
            WEOS_NAMESPACE::weos_detail::_tq::_t t(m_tq);
            if (object.load(order) != current)
                return;
            else
                t.wait();
        }
    }

    template <typename TRep, typename TPeriod>
    void expect_update_for(const atomic_type& object, T current,
                           std::chrono::duration<TRep, TPeriod> const& relTime,
                           expect_hint hint = expect_urgent) const;

    template <typename TClock, typename TDuration>
    void expect_update_until(const atomic_type& object, T current,
                             std::chrono::time_point<TClock, TDuration> const& absTime,
                             expect_hint hint = expect_urgent) const;

private:
    mutable WEOS_NAMESPACE::weos_detail::_tq m_tq;
};

WEOS_END_NAMESPACE

#endif // WEOS_CMSIS_RTOS_SYNCHRONIC_HPP
