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

#ifndef WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP
#define WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP

#include "../config.hpp"
#include "chrono.hpp"
#include "error.hpp"

#include <boost/config.hpp>
#include <boost/static_assert.hpp>

#include <cstdint>
#include <cstring>

namespace weos
{

//! A message queue.
//! The message_queue is an object to pass elements from one thread to another
//! in a thread-safe manner. The object statically holds the necessary memory.
template <typename TypeT, unsigned QueueSizeT>
class message_queue
{
    BOOST_STATIC_ASSERT(sizeof(TypeT) <= 4);
    BOOST_STATIC_ASSERT(QueueSizeT > 0);

public:
    typedef TypeT element_type;

    //! Creates a message queue.
    message_queue()
        : m_id(0)
    {
        // Keil's CMSIS RTOS wants a zero'ed control block type for
        // initialization.
        m_queueData[0] = 0;
        osMessageQDef_t queueDef = { QueueSizeT, m_queueData };
        m_id = osMessageCreate(&queueDef, NULL);
        if (m_id == 0)
        {
            ::weos::throw_exception(weos::system_error(
                                        osErrorOS, cmsis_category()));
        }
    }

    //! Gets an element from the queue.
    //! Returns the first element from the message queue. If the queue is
    //! empty, the calling thread is blocked until an element is added.
    element_type get()
    {
        osEvent result = osMessageGet(m_id, osWaitForever);
        if (result.status != osOK)
        {
            ::weos::throw_exception(weos::system_error(
                                        result.status, cmsis_category()));
        }
        element_type element;
        std::memcpy(&element, &result.value.p, sizeof(element_type));
        return element;
    }

    std::pair<bool, element_type> try_get();

    template <typename RepT, typename PeriodT>
    std::pair<bool, element_type> try_get_for(
            const chrono::duration<RepT, PeriodT>& d);

    //! Puts an element into the queue.
    //! Puts the \p element at the end of the message queue. If the queue
    //! is full, the calling thread is blocked until an element is taken
    //! away from it.
    void put(element_type element)
    {
        std::uint32_t datum = 0;
        std::memcpy(&datum, &element, sizeof(element_type));
        osStatus status = osMessagePut(m_id, datum, osWaitForever);
        if (status != osOK)
        {
            ::weos::throw_exception(weos::system_error(
                                        status, cmsis_category()));
        }
    }

    bool try_put(element_type element);

    template <typename RepT, typename PeriodT>
    bool try_put_for(element_type element,
                     const chrono::duration<RepT, PeriodT>& d);

private:
    //! The storage for the message queue.
    std::uint32_t m_queueData[4 + QueueSizeT];
    //! The id of the message queue.
    osMessageQId m_id;

    // A helper to wait for the message queue.
    struct try_getter
    {
        try_getter(osMessageQId& id)
            : m_id(id)
        {
        }

        element_type& element()
        {
            return m_element;
        }

        // Waits up to \p millisec milliseconds for getting an element from
        // the message queue. Returns \p true, if an element has been acquired
        // and no further waiting is necessary.
        bool operator() (std::int32_t millisec) const
        {
            //! \todo Missing implementation
            return false;
        }

    private:
        osMessageQId& m_id;
        element_type m_element;
    };
};

} // namespace weos

#endif // WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP
