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

#include "core.hpp"

#include "chrono.hpp"
#include "system_error.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

class semaphore
{
public:
    //! The counter type used for the semaphore.
    typedef std::uint16_t value_type;

    //! \brief Creates a semaphore.
    //!
    //! Creates a semaphore with an initial number of \p value tokens.
    explicit semaphore(value_type value = 0)
    {
        os_sem_init(&m_semaphore, value);
    }

    //! \brief Releases a semaphore token.
    //!
    //! Increases the semaphore's value by one.
    //! \note It is undefined behaviour to post() a semaphore which is already
    //! full.
    void post()
    {
        os_sem_send(&m_semaphore);
    }

    //! \brief Waits until a semaphore token is available.
    //!
    //! Blocks the calling thread until the semaphore's value is non-zero.
    //! Then the semaphore is decreased by one and the thread returns.
    void wait()
    {
        OS_RESULT result = os_sem_wait(&m_semaphore, 0xFFFF);
        if (result == OS_R_TMO)
        {
            ::weos::throw_exception(::weos::system_error(
                                        result, keil_rl_rtx_category()));
        }
    }

    //! \brief Tries to acquire a semaphore token.
    //!
    //! Tries to acquire a semaphore token and returns \p true upon success.
    //! If no token is available, the calling thread is not blocked and
    //! \p false is returned.
    bool try_wait()
    {
        OS_RESULT result = os_sem_wait(&m_semaphore, 0);
        return result != OS_R_TMO;
    }

    //! \brief Tries to acquire a semaphore token within a timeout.
    //!
    //! Tries for a timeout period \p d to acquire a semaphore token and returns
    //! \p true upon success or \p false in case of a timeout.
    template <typename RepT, typename PeriodT>
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& d)
    {
        return try_wait_until(chrono::monotonic_clock::now() + d);
    }

    //! \brief Tries to acquire token up to a time point.
    //!
    //! Tries to acquire a semaphore token up to the given \p timePoint. The
    //! return value is \p true, if a token could be acquired before the
    //! timeout.
    template <typename ClockT, typename DurationT>
    bool try_wait_until(const chrono::time_point<ClockT, DurationT>& timePoint)
    {
        typedef typename DurationT::rep rep_t;
        do
        {
            rep_t ticks = chrono::duration_cast<chrono::milliseconds>(
                              timePoint - ClockT::now());
            if (ticks < 0)
                ticks = 0;
            else if (ticks > 0xFFFE)
                ticks = 0xFFFE;

            OS_RESULT result = os_sem_wait(&m_semaphore, ticks);
            if (result != OS_R_TMO)
                return true;
        } while (ticks > 0);

        return false;
    }

    //! \brief Returns the numer of semaphore tokens.
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
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_RL_RTX_SEMAPHORE_HPP
