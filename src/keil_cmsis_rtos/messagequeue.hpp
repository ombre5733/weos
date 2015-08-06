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

#ifndef WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP
#define WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP

#include "core.hpp"

#include "../chrono.hpp"
#include "../semaphore.hpp"
#include "../memorypool.hpp"
#include "../system_error.hpp"
#include "../type_traits.hpp"
#include "../utility.hpp"

#include <cstdint>


WEOS_BEGIN_NAMESPACE

namespace weos_detail
{

template <typename TType, std::size_t TQueueSize>
class SmallMessageQueue
{
    static_assert(sizeof(TType) <= sizeof(std::uint32_t),
                  "Implementation limits element size to 32 bit.");
    static_assert(alignment_of<TType>::value <= alignment_of<std::uint32_t>::value,
                  "The type's alignment is too large.");
    static_assert(is_trivially_copyable<TType>::value,
                  "Type must be bit-wise copyable.");

    static_assert(TQueueSize > 0, "The queue size must be non-zero.");

    using align_tag = alignment_of<TType>;

public:
    typedef TType value_type;

    SmallMessageQueue()
        : m_id(0)
    {
        // Keil's CMSIS RTOS wants a zero'ed control block type for
        // initialization.
        m_queueData[0] = 0;
        osMessageQDef_t queueDef = { TQueueSize, m_queueData };
        m_id = osMessageCreate(&queueDef, NULL);
        if (m_id == 0)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::osErrorOS,
                                    "message_queue::message_queue failed");
    }

    SmallMessageQueue(const SmallMessageQueue&) = delete;
    SmallMessageQueue& operator=(const SmallMessageQueue&) = delete;

    value_type receive()
    {
        osEvent result = osMessageGet(m_id, osWaitForever);
        if (result.status != osEventMessage)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "message_queue::receive failed");

        return *reinterpret_cast<value_type*>(&result.value.v);
    }

    bool try_receive(value_type& value)
    {
        osEvent result = osMessageGet(m_id, 0);
        if (result.status == osOK)
        {
            return false;
        }
        else if (result.status != osEventMessage)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(result.status),
                                    "message_queue::try_receive failed");
        }

        value = *reinterpret_cast<value_type*>(&result.value.v);
        return true;
    }

    void send(value_type value)
    {
        std::uint32_t datum = toUint32(value, align_tag());
        osStatus status = osMessagePut(m_id, datum, osWaitForever);
        if (status != osOK)
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(status),
                                    "message_queue::send failed");
    }

    bool try_send(value_type value)
    {
        std::uint32_t datum = toUint32(value, align_tag());
        osStatus status = osMessagePut(m_id, datum, 0);
        if (status == osOK)
            return true;

        if (   status != osErrorTimeoutResource
            && status != osErrorResource)
        {
            WEOS_THROW_SYSTEM_ERROR(cmsis_error::cmsis_error_t(status),
                                    "message_queue::try_send failed");
        }

        return false;
    }

private:
    inline
    std::uint32_t toUint32(value_type value, integral_constant<std::size_t, 1>)
    {
        std::uint32_t datum;
        for (std::size_t i = 0; i < sizeof(value_type); ++i)
            reinterpret_cast<char*>(&datum)[i] = reinterpret_cast<char*>(&value)[i];
        return datum;
    }

    inline
    std::uint32_t toUint32(value_type value, integral_constant<std::size_t, 2>)
    {
        std::uint32_t datum;
        for (std::size_t i = 0; i < sizeof(value_type) / sizeof(std::uint16_t); ++i)
            reinterpret_cast<std::uint16_t*>(&datum)[i] = reinterpret_cast<std::uint16_t*>(&value)[i];
        return datum;
    }

    inline
    std::uint32_t toUint32(value_type value, integral_constant<std::size_t, 4>)
    {
        return *reinterpret_cast<std::uint32_t*>(&value);
    }

    //! The storage for the message queue.
    std::uint32_t m_queueData[4 + TQueueSize];
    //! The id of the message queue.
    osMessageQId m_id;
};

template <typename TType, std::size_t TQueueSize>
class LargeMessageQueue
{
public:
    typedef TType value_type;

    LargeMessageQueue()
        : m_numAvailable(TQueueSize)
    {
    }

    value_type receive()
    {
        void* mem = m_pointerQueue.receive();
        WEOS_ASSERT(mem != nullptr);
        value_type temp(weos::move(*static_cast<value_type*>(mem)));
        static_cast<value_type*>(mem)->~TType();
        m_memoryPool.free(mem);
        m_numAvailable.post();
        return temp;
    }

    bool try_receive(value_type& value)
    {
        void* mem;
        bool result = m_pointerQueue.try_receive(mem);
        if (result)
        {
            WEOS_ASSERT(mem != nullptr);
            value = weos::move(*static_cast<value_type*>(mem));
            static_cast<value_type*>(mem)->~TType();
            m_memoryPool.free(mem);
            m_numAvailable.post();
            return true;
        }
        else
        {
            return false;
        }
    }

    void send(const value_type& element)
    {
        m_numAvailable.wait();
        void* mem = m_memoryPool.try_allocate();
        WEOS_ASSERT(mem != nullptr);
        // TODO: unique_ptr
        new (mem) value_type(element);
        m_pointerQueue.send(mem);
    }

    bool try_send(const value_type& element)
    {
        // TODO: Make this work in an interrupt context.
        if (!m_numAvailable.try_wait())
            return;
        void* mem = m_memoryPool.try_allocate();
        WEOS_ASSERT(mem != nullptr);
        // TODO: unique_ptr
        new (mem) value_type(element);
        bool result = m_pointerQueue.try_send(mem);
        WEOS_ASSERT(result);
        (void)result;
    }

#if 0
    atomic<int> m_numAwailable;
    sempahore m_trigger;

    value_type _receive()
    {
        void* mem = m_pointerQueue.receive();
        WEOS_ASSERT(mem != nullptr);
        value_type temp(weos::move(*static_cast<value_type*>(mem)));
        static_cast<value_type*>(mem)->~TType();
        m_memoryPool.free(mem);
        if (++m_numAvailable <= 0)
            m_trigger.post();
        return temp;
    }

    void _send(const value_type& element)
    {
        if (--m_numAwailable < 0)
            m_trigger.wait();
        void* mem = m_memoryPool.try_allocate();
        WEOS_ASSERT(mem != nullptr);
        // TODO: unique_ptr
        new (mem) value_type(element);
        m_pointerQueue.send(mem);
    }

    bool _try_send(const value_type& element)
    {
        int available = m_numAwailable;
        while (available > 0 && !m_numAwailable.compare_exchange_weak(available, available - 1))
        {
        }
        if (available <= 0)
            return false;

        void* mem = m_memoryPool.try_allocate();
        WEOS_ASSERT(mem != nullptr);
        // TODO: unique_ptr
        new (mem) value_type(element);
        bool result = m_pointerQueue.try_send(mem);
        WEOS_ASSERT(result);
        (void)result;
    }
#endif

private:
    semaphore m_numAvailable;
    shared_memory_pool<TType, TQueueSize> m_memoryPool;
    SmallMessageQueue<void*, TQueueSize> m_pointerQueue;
};

template <typename TType, std::size_t TQueueSize>
struct select_message_queue_implementation
{
    static const bool is_small = sizeof(TType) <= sizeof(std::uint32_t);
    static const bool has_small_alignment = alignment_of<TType>::value <= alignment_of<std::uint32_t>::value;
    static const bool can_be_copied = is_trivially_copyable<TType>::value;

    typedef typename conditional<is_small && has_small_alignment && can_be_copied,
                                 SmallMessageQueue<TType, TQueueSize>,
                                 LargeMessageQueue<TType, TQueueSize>>::type type;
};

} // namespace weos_detail

//! A message queue.
//! The message_queue is an object to pass elements from one thread to another
//! in a thread-safe manner. The object statically holds the necessary memory.
template <typename TType, std::size_t TQueueSize>
class message_queue
        : public weos_detail::select_message_queue_implementation<TType, TQueueSize>::type
{
public:
    //! The type of the elements transfered via this message queue.
    typedef TType value_type;

    //! \brief Creates a message queue.
    //!
    //! Creates an empty message queue.
    message_queue()
    {
    }

    message_queue(const message_queue&) = delete;
    message_queue& operator=(const message_queue&) = delete;

    //! \brief Returns the capacity.
    //!
    //! Returns the maximum number of elements which the queue can hold.
    std::size_t capacity() const
    {
        return TQueueSize;
    }

    //! \brief Receives an element from the queue.
    //!
    //! Returns the first element from the message queue. If the queue is
    //! empty, the calling thread is blocked until an element is added.
    // value_type receive();

    //! \brief Tries to receive an element from the queue.
    //!
    //! Tries to receive an element from the message queue.
    //! The element is returned together with a boolean, which is set if the
    //! queue was non-empty. If the queue was empty, the boolean is reset and
    //! the returned element is default-constructed.
    // bool try_receive(value_type& value);

#if 0
    //! Tries to receive an element from the queue.
    //! Tries to receive an element from the message queue within the timeout
    //! duration \p d.
    //! The element is returned together with a boolean, which is set if the
    //! queue was non-empty. If the queue was empty, the boolean is reset and
    //! the returned element is default-constructed.
    template <typename RepT, typename PeriodT>
    std::pair<bool, value_type> try_receive_for(
            const chrono::duration<RepT, PeriodT>& d)
    {
        try_receiver receiver(m_id);
        if (chrono::detail::cmsis_wait<
                RepT, PeriodT, try_receiver>::wait(d, receiver))
        {
            value_type element;
            std::memcpy(&element, &receiver.datum(), sizeof(value_type));
            return std::pair<bool, value_type>(true, element);
        }

        return std::pair<bool, value_type>(false, value_type());
    }
#endif

    //! \brief Sends an element via the queue.
    //!
    //! Sends the \p element by appending it at the end of the message queue.
    //! If the queue is full, the calling thread is blocked until space becomes
    //! available.
    //!
    //! \note This method may be called in an interrupt context.
    // void send(const value_type& element);

    //! \brief Tries to send an element via the queue.
    //!
    //! Tries to send the \p element via the queue. If no space was available,
    //! \p false is returned. Otherwise the method returns \p true. The
    //! calling thread is never blocked.
    // bool try_send(const value_type& element);

#if 0
    //! Tries to send an element via the queue.
    //! Tries to send the given \p element via the queue and returns \p true
    //! if successful. If there is no space available within the
    //! duration \p d, the operation is aborted an \p false is returned.
    template <typename RepT, typename PeriodT>
    bool try_send_for(value_type element,
                      const chrono::duration<RepT, PeriodT>& d)
    {
        std::uint32_t datum = 0;
        std::memcpy(&datum, &element, sizeof(value_type));
        try_sender sender(m_id, datum);
        return chrono::detail::cmsis_wait<
                RepT, PeriodT, try_sender>::wait(d, sender);
    }
#endif
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_MESSAGEQUEUE_HPP
