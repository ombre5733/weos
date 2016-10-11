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

#include "_tq.hpp"
#include "cmsis_error.hpp"
#include "_svc_indirection.hpp"

#include WEOS_CMSIS_CORE_CMX_INCLUDE

using namespace std;


extern "C"
osPriority svcThreadGetPriority(osThreadId thread_id);

extern "C"
osThreadId svcThreadGetId(void);


extern "C"
int weos_tq_notify(void* q_, uint32_t a_) noexcept
{
    using namespace WEOS_NAMESPACE::weos_detail;

    _tq& q = *static_cast<_tq*>(q_);
    if (a_)
    {
        _tq::_t* i = q.m_h.exchange(nullptr);
        while (i)
        {
            uintptr_t iv = i->m_v;
            i->m_v = iv | uintptr_t(3);
            i->m_s.post();
            i = reinterpret_cast<_tq::_t*>(iv & ~uintptr_t(3));
        }
        return 0;
    }
    else
    {
        _tq::_t* i = q.m_h.load();
        uintptr_t iv;
        _tq::_t* in;
        do
        {
            if (!i)
                return 0;
            iv = i->m_v;
            in = reinterpret_cast<_tq::_t*>(iv & ~uintptr_t(3));
        } while (!q.m_h.compare_exchange_weak(i, in));
        i->m_v = iv | uintptr_t(3);
        i->m_s.post();
        return 0;
    }
}

extern "C"
int weos_tq_link(void* q_, void* t_) noexcept
{
    using namespace WEOS_NAMESPACE::weos_detail;

    _tq* q = static_cast<_tq*>(q_);
    _tq::_t* t = static_cast<_tq::_t*>(t_);
    osPriority p = t->m_p = svcThreadGetPriority(svcThreadGetId());
    if (p == osPriorityError)
        return 1;
    t->m_p = p;

    _tq::_t* i = nullptr;
    for (;;)
    {
        if (!i)
        {
            while (!q->m_h.compare_exchange_weak(i, t) && !i) {}
            if (!i)
                return 0;
        }

        uintptr_t iv = i->m_v.load();
        if (iv & 2)
        {
            i = nullptr;
            continue;
        }

        _tq::_t* in = reinterpret_cast<_tq::_t*>(iv & ~uintptr_t(3));
        if (in && in->m_p >= p)
        {
            i = in;
            continue;
        }

        t->m_v = uintptr_t(in);
        while (!i->m_v.compare_exchange_weak(iv, uintptr_t(t) | (iv & uintptr_t(3)))
               && ((iv & 2) == 0))
        {
        }
        if (iv & 2)
        {
            i = nullptr;
            continue;
        }

        return 0;
    }
}

extern "C"
int weos_tq_unlink(void* q_, void* t_) noexcept
{
    using namespace WEOS_NAMESPACE::weos_detail;

    _tq& q = *static_cast<_tq*>(q_);
    _tq::_t* t = static_cast<_tq::_t*>(t_);
    _tq::_t* tn = reinterpret_cast<_tq::_t*>(t->m_v & ~uintptr_t(3));

    _tq::_t* i = t;
    while (!q.m_h.compare_exchange_weak(i, tn) && i == t) {}
    if (i == t || !i)
    {
        t->m_v |= uintptr_t(2);
        return 0;
    }

    for (;;)
    {
        uintptr_t iv = i->m_v.load();
        _tq::_t* in = reinterpret_cast<_tq::_t*>(iv & ~uintptr_t(3));
        if (!in
            || (in == t && i->m_v.compare_exchange_weak(iv, uintptr_t(tn) | (iv & uintptr_t(3)))))
        {
            t->m_v |= uintptr_t(2);
            return 0;
        }
        if (in != t)
            i = in;
    }
}


SVC_2(weos_tq_notify, int,   void*, uint32_t)
SVC_2(weos_tq_link,   int,   void*, void*)
SVC_2(weos_tq_unlink, int,   void*, void*)



WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

_tq::_t::_t(_tq& q)
    : m_tq(q),
      m_v(0)
{
    if (__get_IPSR() != 0U)
    {
        WEOS_THROW_SYSTEM_ERROR(WEOS_NAMESPACE::cmsis_error::cmsis_error_t(osErrorISR),
                                "not allowed in ISR");
    }

    weos_tq_link_indirect(&m_tq, this);
}

bool _tq::_t::unlink() noexcept
{
    uintptr_t v = m_v.load();
    if (v & 2)
        return v & 1;

    weos_tq_unlink_indirect(&m_tq, this);
    return m_v.load() & 1;
}

void _tq::notify_one() noexcept
{
    if (m_h.load() == nullptr)
        return;

    if (__get_IPSR() != 0U)
        weos_tq_notify(this, 0);
    else
        weos_tq_notify_indirect(this, 0);
}

void _tq::notify_all() noexcept
{
    if (m_h.load() == nullptr)
        return;

    if (__get_IPSR() != 0U)
        weos_tq_notify(this, 1);
    else
        weos_tq_notify_indirect(this, 1);
}

} // namespace weos_detail

WEOS_END_NAMESPACE
