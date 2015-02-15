/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2015, Manuel Freiberger
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

#include "core.hpp"

#include "../chrono.hpp"
#include "../system_error.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace detail
{
// Just enough memory to hold a CMSIS semaphore. This is the type to which
// the macro osSemaphoreDef() defined in <cmsis_os.h> expands.
struct SemaphoreControlBlock
{
    // The first 32-bit of a semaphore. The typedef can be found
    // in ${Keil-CMSIS-RTOS}/SRC/rt_TypeDef.h.
    struct Header
    {
        volatile std::uint8_t controlBlockType;
        volatile std::uint8_t tokenMask;
        volatile std::uint16_t numTokens;
    };

    std::uint32_t _[2];

    inline
    std::int16_t numTokens() const
    {
        return reinterpret_cast<const Header&>(_[0]).numTokens;
    }
};

} // namespace detail


//! \brief A semaphore.
class semaphore
{
public:
    //! The counter type used for the semaphore.
    typedef std::uint16_t value_type;

    //! \brief Creates a semaphore.
    //!
    //! Creates a semaphore with an initial number of \p value tokens.
    explicit semaphore(value_type value = 0);

    //! \brief Destroys a semaphore.
    ~semaphore();

    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    //! \brief Releases a semaphore token.
    //!
    //! Increases the semaphore's value by one.
    //! \note It is undefined behaviour to post() a semaphore which is already
    //! full.
    void post();

    //! \brief Waits until a semaphore token is available.
    //!
    //! Blocks the calling thread until the semaphore's value is non-zero.
    //! Then the semaphore is decreased by one and the thread returns.
    void wait();

    //! \brief Tries to acquire a semaphore token.
    //!
    //! Tries to acquire a semaphore token and returns \p true upon success.
    //! If no token is available, the calling thread is not blocked and
    //! \p false is returned.
    bool try_wait();

    //! \cond
    //! Tries to acquire a semaphore token within a timeout.
    //!
    //! This is an overload for the case when the timeout is given in
    //! milliseconds.
    bool try_wait_for(chrono::milliseconds ms);
    //! \endcond

    //! \brief Tries to acquire a semaphore token within a timeout.
    //!
    //! Tries to acquire a semaphore token within the given \p timeout. The
    //! return value is \p true if a token could be acquired.
    template <typename RepT, typename PeriodT>
    inline
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& timeout)
    {
        using namespace chrono;

        milliseconds converted = duration_cast<milliseconds>(timeout);
        if (converted < timeout)
            ++converted;

        return try_wait_for(converted);
    }

    //! \brief Tries to acquire token up to a time point.
    //!
    //! Tries to acquire a semaphore token up to the given \p time point. The
    //! return value is \p true, if a token could be acquired before the
    //! timeout.
    template <typename ClockT, typename DurationT>
    bool try_wait_until(const chrono::time_point<ClockT, DurationT>& time)
    {
        typedef typename WEOS_NAMESPACE::common_type<
                             typename ClockT::duration,
                             DurationT>::type difference_type;
        typedef chrono::detail::internal_time_cast<difference_type> caster;

        while (true)
        {
            typename caster::type millisecs
                    = caster::convert_and_clip(time - ClockT::now());

            std::int32_t result = osSemaphoreWait(m_id, millisecs);
            if (result > 0)
                return true;

            if (result < 0)
            {
                WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                        "semaphore::try_wait_until failed");
            }

            if (millisecs == 0)
                return false;
        }
    }

    //! Returns the numer of semaphore tokens.
    value_type value() const;

private:
    //! The native semaphore.
    detail::SemaphoreControlBlock m_controlBlock;
    //! The native semaphore handle.
    osSemaphoreId m_id;
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_SEMAPHORE_HPP
