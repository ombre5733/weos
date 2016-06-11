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

#ifndef WEOS_CXX11_SEMAPHORE_HPP
#define WEOS_CXX11_SEMAPHORE_HPP

#include "_core.hpp"

#include "../chrono.hpp"
#include "../condition_variable.hpp"
#include "../mutex.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

//! \brief A semaphore.
class semaphore
{
public:
    //! The type of the native mutex handle.
    using native_handle_type = semaphore*;

    //! The counter type used for the semaphore.
    using value_type = std::uint16_t;

    //! \brief Creates a semaphore.
    //!
    //! Creates a semaphore with an initial number of \p value tokens.
    /*constexpr*/ explicit // TODO: make constexpr
    semaphore(value_type value = 0) noexcept
        : m_value{value}
    {
    }

    //! \brief Destroys a semaphore.
    ~semaphore();

    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    //! \brief Releases a semaphore token.
    //!
    //! Increases the semaphore's value by one.
    //! \note It is undefined behaviour to post() a semaphore which is already
    //! full.
    //!
    //! \note This method may be called in an interrupt context.
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

    //! \brief Tries to acquire a semaphore token within a timeout.
    //!
    //! Tries to acquire a semaphore token within the given \p timeout. The
    //! return value is \p true if a token could be acquired.
    template <typename RepT, typename PeriodT>
    inline
    bool try_wait_for(const chrono::duration<RepT, PeriodT>& timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_conditionVariable.wait_for(lock, timeout,
                                         [this] { return m_value != 0; }))
        {
            --m_value;
            return true;
        }
        return false;
    }

    //! \brief Tries to acquire token up to a time point.
    //!
    //! Tries to acquire a semaphore token up to the given \p time point. The
    //! return value is \p true, if a token could be acquired before the
    //! timeout.
    template <typename ClockT, typename DurationT>
    inline
    bool try_wait_until(const chrono::time_point<ClockT, DurationT>& time)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_conditionVariable.wait_until(lock, time,
                                           [this] { return m_value != 0; }))
        {
            --m_value;
            return true;
        }
        return false;
    }

    //! Returns the numer of semaphore tokens.
    value_type value() const;

    //! Returns a native semaphore handle.
    native_handle_type native_handle()
    {
        return this;
    }

private:
    value_type m_value;
    mutable std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
};

WEOS_END_NAMESPACE

#endif // WEOS_CXX11_SEMAPHORE_HPP
