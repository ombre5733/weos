/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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
#include "system_error.hpp"

#include <cstdint>

namespace weos
{

class semaphore
{
public:
    //! The counter type used for the semaphore.
    typedef std::uint16_t value_type;

    //! Creates a semaphore.
    //! Creates a semaphore with an initial number of \p value tokens.
    explicit semaphore(value_type value = 0)
    {
        os_sem_init(&m_semaphore, value);
    }

    //! Releases a semaphore token.
    void post()
    {
        os_sem_send(&m_semaphore);
    }

    //! Waits until a semaphore token is available.
    void wait()
    {
        OS_RESULT result = os_sem_wait(&m_semaphore, 0xFFFF);
        if (result == OS_R_TMO)
        {
            ::weos::throw_exception(::weos::system_error(
                                        result, keil_rl_rtx_category()));
        }
    }

    //! Tries to acquire a semaphore token.
    //! Tries to acquire a semaphore token and returns \p true upon success.
    //! If no token is available, the calling thread is not blocked and
    //! \p false is returned.
    bool try_wait()
    {
        OS_RESULT result = os_sem_wait(&m_semaphore, 0);
        return result != OS_R_TMO;
    }

    //! Tries to acquire a semaphore token within a timeout.
    //! Tries for a timeout period \p d to acquire a semaphore token and returns
    //! \p true upon success or \p false in case of a timeout.
    template <typename RepT, typename PeriodT>
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& d)
    {
        try_waiter waiter(m_semaphore);
        return chrono::detail::cmsis_wait<
                RepT, PeriodT, try_waiter>::wait(d, waiter);
    }

    //! Returns the numer of semaphore tokens.
    value_type value() const
    {
        return semaphoreControlBlockHeader()->numTokens;
    }

private:
    //! The underlying RL RTX semaphore.
    OS_SEM m_semaphore;

    semaphore(const semaphore&);
    const semaphore& operator= (const semaphore&);

    // The header (first 32 bits) of the semaphore control block. The full
    // definition can be found in $/RL/RTX/SRC/rt_TypeDef.h.
    struct SemaphoreControlBlockHeader
    {
        std::uint8_t controlBlockType;
        std::uint16_t numTokens;
        std::uint8_t unused;
    };

    const SemaphoreControlBlockHeader* semaphoreControlBlockHeader() const
    {
        return reinterpret_cast<const SemaphoreControlBlockHeader*>(
                    &m_semaphore);
    }

    // A helper to wait for a semaphore.
    struct try_waiter
    {
        try_waiter(OS_SEM& semaphore)
            : m_semaphore(semaphore)
        {
        }

        // Waits up to \p ticks system ticks for a semaphore token. Returns
        // \p true, if a token has been acquired and no further waiting is
        // necessary.
        bool operator() (std::int32_t ticks) const
        {
            WEOS_ASSERT(ticks < 0xFFFF);
            OS_RESULT result = os_sem_wait(&m_semaphore, ticks);
            return result != OS_R_TMO;
        }

    private:
        OS_SEM& m_semaphore;
    };
};

} // namespace weos

#endif // WEOS_KEIL_RL_RTX_SEMAPHORE_HPP
