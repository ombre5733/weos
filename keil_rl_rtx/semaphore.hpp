/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permittedo provided that the following conditions are met:

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

#ifndef WEOS_KEIL_RL_RTX_SEMAPHORE_HPP
#define WEOS_KEIL_RL_RTX_SEMAPHORE_HPP

#include "../config.hpp"
#include "chrono.hpp"
#include "error.hpp"

#include <boost/utility.hpp>

#include <cstdint>

namespace weos
{

class semaphore : boost::noncopyable
{
public:
    //! Creates a semaphore.
    //! Creates a semaphore with an initial number of \p value tokens.
    explicit semaphore(std::uint32_t value = 0)
    {
        WEOS_ASSERT(value <= 0xFFFF);
        os_sem_init(&m_id, value);
    }

    //! Destroys the semaphore.
    ~semaphore()
    {
    }

    //! Waits until a semaphore token is available.
    void wait()
    {
        OS_RESULT result = os_sem_wait(&m_id, 0xFFFF);
        if (result == OS_R_TMO)
            ::weos::throw_exception(::weos::system_error(osErrorOS, keil_rl_rtx_category()));
    }

    bool try_wait()
    {
        OS_RESULT result = os_sem_wait(&m_id, 0xFFFF);
        return result != OS_R_TMO; TODO: check this!
    }

    template <typename RepT, typename PeriodT>
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& d)
    {
        semaphore_try_waiter waiter(m_id);
        return chrono::detail::cmsis_wait<
                RepT, PeriodT, semaphore_try_waiter>::wait(d, waiter);
    }

    //! Releases a semaphore token.
    void post()
    {
        os_sem_send(&m_id);
    }

    //! Returns the numer of semaphore tokens.
    std::uint32_t value() const
    {
        return semaphoreControlBlockHeader()->numTokens;
    }

private:
    std::uint32_t m_cmsisSemaphoreControlBlock[2];
    osSemaphoreId m_id;

    // The header (first 32 bits) of the semaphore control block. The full
    // definition can be found in ../3rdparty/keil_cmsis_rtos/SRC/rt_TypeDef.h
    struct SemaphoreControlBlockHeader
    {
        std::uint8_t controlBlockType;
        std::uint8_t tokenMask;
        std::uint16_t numTokens;
    };

    const SemaphoreControlBlockHeader* semaphoreControlBlockHeader() const
    {
        return reinterpret_cast<const SemaphoreControlBlockHeader*>(
                    m_cmsisSemaphoreControlBlock);
    }

    // A helper to wait for a semaphore.
    struct semaphore_try_waiter
    {
        semaphore_try_waiter(osSemaphoreId id)
            : m_id(id)
        {
        }

        bool operator() (std::int32_t millisec) const
        {
            std::int32_t numTokens = osSemaphoreWait(m_id, millisec);
            if (numTokens < 0)
                ::weos::throw_exception(::weos::system_error(osErrorOS, keil_rl_rtx_category()));
            return numTokens != 0;
        }

    private:
        OS_SEM m_id;
    };
};

} // namespace weos

#endif // WEOS_KEIL_RL_RTX_SEMAPHORE_HPP
