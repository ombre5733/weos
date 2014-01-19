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

#ifndef WEOS_CXX11_THREAD_HPP
#define WEOS_CXX11_THREAD_HPP

#include "../config.hpp"
#include "chrono.hpp"

#include <condition_variable>
#include <cstdint>
#include <limits>
#include <mutex>
#include <thread>

namespace weos
{

//! Traits for thread signals.
struct signal_traits
{
    typedef std::uint32_t flags_type;

    inline
    static const int num_flags()
    {
        return 32;
    }

    inline
    static const flags_type all_flags()
    {
        return std::numeric_limits<flags_type>::max();
    }
};


namespace detail
{

struct ThreadData
{
    std::mutex signalMutex;
    signal_traits::flags_type signalFlags;
    std::condition_variable signalCv;

    static std::shared_ptr<ThreadData> create(std::thread::id id);
    static std::shared_ptr<ThreadData> find(std::thread::id id);
    static void remove(std::thread::id id);
};

} // namespace detail

class thread : public std::thread
{
public:
    thread()
    {
    }

    template <typename TFunction, typename... TArgs>
    explicit thread(TFunction&& f, TArgs&&... args)
        : std::thread(std::forward<TFunction>(f),
                      std::forward<TArgs>(args)...),
          m_data(detail::ThreadData::create(this->get_id()))
    {
    }

    thread(const thread&) = delete;

    thread(thread&& other)
        : m_data(other.m_data)
    {
        other.m_data.reset();
    }

    ~thread()
    {
        detail::ThreadData::remove(this->get_id());
    }

    thread& operator= (thread&) = delete;

    thread& operator= (thread&& other)
    {
        if (this != &other)
        {
            std::thread::operator= (std::move(other));
            m_data = other.m_data;
            other.m_data.reset();
        }
        return *this;
    }

    //! Clears a set of signals.
    //! Clears the signals which are specified by the \p flags.
    inline
    void clear_signals(signal_traits::flags_type flags)
    {
        if (!joinable())
        {
            //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }

        std::lock_guard<std::mutex> lock(m_data->signalMutex);
        m_data->signalFlags &= ~flags;
        m_data->signalCv.notify_one();
    }

    //! Sets a set of signals.
    //! Sets the signals which are specified by the \p flags.
    inline
    void set_signals(signal_traits::flags_type flags)
    {
        if (!joinable())
        {
            //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
        }

        std::lock_guard<std::mutex> lock(m_data->signalMutex);
        m_data->signalFlags |= flags;
        m_data->signalCv.notify_one();
    }

private:
    std::shared_ptr<detail::ThreadData> m_data;
};

namespace this_thread
{
using std::this_thread::get_id;
using std::this_thread::sleep_for;
using std::this_thread::sleep_until;
using std::this_thread::yield;

//! Waits for any signal.
//! Blocks the current thread until one or more signal flags have been set,
//! returns these flags and resets them.
inline
signal_traits::flags_type wait_for_any_signal()
{
    std::shared_ptr<detail::ThreadData> data
            = detail::ThreadData::find(get_id());
    if (!data)
    {
        //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }

    std::unique_lock<std::mutex> lock(data->signalMutex);
    if (data->signalFlags == 0)
        data->signalCv.wait(lock, [data]{ return data->signalFlags != 0; });
    signal_traits::flags_type temp = data->signalFlags;
    data->signalFlags = 0;
    return temp;
}

//! Checks if any signal has arrived.
//! Checks if one or more signal flags have been set for the current thread,
//! returns these flags and resets them. If no signal is set, zero
//! is returned.
inline
signal_traits::flags_type try_wait_for_any_signal()
{
    std::shared_ptr<detail::ThreadData> data
            = detail::ThreadData::find(get_id());
    if (!data)
    {
        //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }

    std::unique_lock<std::mutex> lock(data->signalMutex);
    signal_traits::flags_type temp = data->signalFlags;
    data->signalFlags = 0;
    return temp;
}

// try_wait_for_any_signal_for()

//! Waits for a set of signals.
//! Blocks the current thread until all signal flags selected by \p flags have
//! been set, returns these flags and resets them. Signal flags which are
//! not selected by \p flags are not reset.
inline
signal_traits::flags_type wait_for_all_signals(signal_traits::flags_type flags)
{
    std::shared_ptr<detail::ThreadData> data
            = detail::ThreadData::find(get_id());
    if (!data)
    {
        //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }

    std::unique_lock<std::mutex> lock(data->signalMutex);
    if ((data->signalFlags & flags) != flags)
    {
        data->signalCv.wait(
                    lock,
                    [data, flags]{ return (data->signalFlags & flags)
                                          == flags; });
    }
    signal_traits::flags_type temp = (data->signalFlags & flags) == flags
                                     ? flags : 0;
    data->signalFlags &= ~temp;
    return temp;
}

//! Checks if a set of signals has been set.
//! Checks if all signal flags selected by \p flags have been set, returns
//! these flags and resets them. Signal flags which are not selected
//! through \p flags are not reset.
//! If not all signal flags specified by \p flags are set, zero is returned
//! and no flag is reset.
inline
signal_traits::flags_type try_wait_for_all_signals(
        signal_traits::flags_type flags)
{
    std::shared_ptr<detail::ThreadData> data
            = detail::ThreadData::find(get_id());
    if (!data)
    {
        //::weos::throw_exception(system_error(-1, cmsis_category())); //! \todo Use correct value
    }

    std::unique_lock<std::mutex> lock(data->signalMutex);
    signal_traits::flags_type temp = (data->signalFlags & flags) == flags
                                     ? flags : 0;
    data->signalFlags &= ~temp;
    return temp;
}

} // namespace this_thread

} // namespace weos

#endif // WEOS_CXX11_THREAD_HPP
