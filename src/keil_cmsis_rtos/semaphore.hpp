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

#ifndef WEOS_KEIL_CMSIS_RTOS_SEMAPHORE_HPP
#define WEOS_KEIL_CMSIS_RTOS_SEMAPHORE_HPP

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
        : m_id(0)
    {
        // Keil's RTOS wants a zero'ed control block type for initialization.
        m_cmsisSemaphoreControlBlock[0] = 0;
        osSemaphoreDef_t semaphoreDef = { m_cmsisSemaphoreControlBlock };
        m_id = osSemaphoreCreate(&semaphoreDef, value);
        if (m_id == 0)
        {
            ::weos::throw_exception(::weos::system_error(
                                        osErrorOS, cmsis_category()));
        }
    }

    //! Destroys the semaphore.
    ~semaphore()
    {
        if (m_id)
            osSemaphoreDelete(m_id);
    }

    //! Releases a semaphore token.
    void post()
    {
        osStatus status = osSemaphoreRelease(m_id);
        if (status != osOK)
        {
            ::weos::throw_exception(::weos::system_error(
                                        status, cmsis_category()));
        }
    }

    //! Waits until a semaphore token is available.
    void wait()
    {
        std::int32_t numTokens = osSemaphoreWait(m_id, osWaitForever);
        if (numTokens <= 0)
        {
            ::weos::throw_exception(::weos::system_error(
                                        osErrorOS, cmsis_category()));
        }
    }

    //! Tries to acquire a semaphore token.
    //! Tries to acquire a semaphore token and returns \p true upon success.
    //! If no token is available, the calling thread is not blocked and
    //! \p false is returned.
    bool try_wait()
    {
        std::int32_t numTokens = osSemaphoreWait(m_id, 0);
        if (numTokens < 0)
        {
            ::weos::throw_exception(::weos::system_error(
                                        osErrorOS, cmsis_category()));
        }
        return numTokens != 0;
    }

    //! Tries to acquire a semaphore token within a timeout.
    //! Tries for a timeout period \p d to acquire a semaphore token and returns
    //! \p true upon success or \p false in case of a timeout.
    template <typename RepT, typename PeriodT>
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& d)
    {
        try_waiter waiter(m_id);
        return chrono::detail::cmsis_wait<
                RepT, PeriodT, try_waiter>::wait(d, waiter);
    }

    //! Returns the numer of semaphore tokens.
    value_type value() const
    {
        return semaphoreControlBlockHeader()->numTokens;
    }

private:
    // Just enough memory to hold a CMSIS semaphore. The typedef can be found
    // in ${Keil-CMSIS-RTOS}/SRC/rt_TypeDef.h.
    std::uint32_t m_cmsisSemaphoreControlBlock[2];
    osSemaphoreId m_id;

    semaphore(const semaphore&);
    const semaphore& operator= (const semaphore&);

    // The header (first 32 bits) of the semaphore control block. The full
    // definition can be found in ${Keil-CMSIS-RTOS}/SRC/rt_TypeDef.h.
    struct SemaphoreControlBlockHeader
    {
        std::uint8_t controlBlockType;
        std::uint8_t tokenMask;
        std::uint16_t numTokens;
    };

    const SemaphoreControlBlockHeader* semaphoreControlBlockHeader() const
    {
        return reinterpret_cast<const SemaphoreControlBlockHeader*>(
                    &m_cmsisSemaphoreControlBlock);
    }

    // A helper to wait for a semaphore.
    struct try_waiter
    {
        try_waiter(osSemaphoreId& id)
            : m_id(id)
        {
        }

        // Waits up to \p millisec milliseconds for a semaphore token. Returns
        // \p true, if a token has been acquired and no further waiting is
        // necessary.
        bool operator() (std::int32_t millisec) const
        {
            std::int32_t numTokens = osSemaphoreWait(m_id, millisec);
            if (numTokens < 0)
            {
                ::weos::throw_exception(::weos::system_error(
                                            osErrorOS, cmsis_category()));
            }

            return numTokens != 0;
        }

    private:
        // The semaphore from which a token shall be acquired.
        osSemaphoreId& m_id;
    };
};

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_SEMAPHORE_HPP
