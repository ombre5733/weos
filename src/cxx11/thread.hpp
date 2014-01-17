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

    static const int num_flags = 32;

    static const flags_type all_flags = std::numeric_limits<flags_type>::max();
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
        : std::thread(f, args...),
          m_data(detail::ThreadData::create(this->get_id()))
    {
    }

    thread(thread&& other)
        : m_data(other.m_data)
    {
        other.m_data.reset();
    }

    ~thread()
    {
        detail::ThreadData::remove(this->get_id());
    }

    thread& operator= (thread&& other)
    {
        if (this != &other)
        {
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
    data->signalCv.wait(lock, [data]{ return data->signalFlags != 0; });
    return data->signalFlags;
}

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
    data->signalCv.wait(
                lock,
                [data, flags]{ return (data->signalFlags & flags) == flags; });
    return data->signalFlags;
}

} // namespace this_thread

} // namespace weos

#endif // WEOS_CXX11_THREAD_HPP
