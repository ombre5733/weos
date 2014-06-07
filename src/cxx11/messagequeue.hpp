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

#ifndef WEOS_CXX11_MESSAGEQUEUE_HPP
#define WEOS_CXX11_MESSAGEQUEUE_HPP

#include "../config.hpp"
#include "chrono.hpp"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <utility>

namespace weos
{

//! A message queue.
//! The message_queue is an object to pass elements from one thread to another
//! in a thread-safe manner. The type of the element which are transfered is
//! defined by the template parameter \p TypeT. The maximum size of the queue
//! has to be passed in \p QueueSizeT.
template <typename TypeT, std::size_t QueueSizeT>
class message_queue
{
    //! \todo Removed the size check for now because 64-bit pointers must
    //! work, too.
    // For compatibility we limit the size of a message to the size of uint32_t.
    //static_assert(sizeof(TypeT) <= 4, "The element size is too large.");

    static_assert(QueueSizeT > 0, "The queue size must be nonzero.");

public:
    //! The type of the elements transfered via this message queue.
    typedef TypeT element_type;

    //! Returns the capacity.
    //! Returns the maximum number of elements which the queue can hold.
    std::size_t capacity() const
    {
        return QueueSizeT;
    }

    //! Receives an element from the queue.
    //! Returns the first element from the message queue. If the queue is
    //! empty, the calling thread is blocked until an element is added.
    element_type receive()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_cv_receive.wait(lock);
        }

        element_type element = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        m_cv_send.notify_one();

        return element;
    }

    //! Tries to receive an element from the queue.
    //! Tries to receive an element from the message queue.
    //! The element is returned together with a boolean, which is set if the
    //! queue was non-empty. If the queue was empty, the boolean is reset and
    //! the returned element is default-constructed.
    std::pair<bool, element_type> try_receive()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty())
        {
            return std::pair<bool, element_type>(false, element_type());
        }

        element_type element = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        m_cv_send.notify_one();

        return std::pair<bool, element_type>(true, element);
    }

    //! Tries to receive an element from the queue.
    //! Tries to receive an element from the message queue within the timeout
    //! duration \p d.
    //! The element is returned together with a boolean, which is set if the
    //! queue was non-empty. If the queue was empty, the boolean is reset and
    //! the returned element is default-constructed.
    template <typename RepT, typename PeriodT>
    std::pair<bool, element_type> try_receive_for(
            const chrono::duration<RepT, PeriodT>& d)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // Note: If we spuriously wakeup, we should not wait again for
            // the same time because then we wait too long.
            if (m_cv_receive.wait(lock, d) == std::cv_status::timeout)
            {
                if (m_queue.empty())
                    return std::pair<bool, element_type>(false, element_type());
                break;
            }
        }

        element_type element = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        m_cv_send.notify_one();

        return std::pair<bool, element_type>(true, element);
    }

    //! Sends an element via the queue.
    //! Sends the \p element by appending it at the end of the message queue.
    //! If the queue is full, the calling thread is blocked until space becomes
    //! available.
    void send(element_type element)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (isFull())
        {
            m_cv_send.wait(lock);
        }

        m_queue.push_back(element);
        lock.unlock();
        m_cv_receive.notify_one();
    }

    //! Tries to send an element via the queue.
    //! Tries to send the \p element via the queue. If no space was available,
    //! \p false is returned. Otherwise the method returns \p true. The
    //! calling thread is never blocked.
    bool try_send(element_type element)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (isFull())
            return false;

        m_queue.push_back(element);
        lock.unlock();
        m_cv_receive.notify_one();

        return true;
    }

    //! Tries to send an element via the queue.
    //! Tries to send the given \p element via the queue and returns \p true
    //! if successful. If there is no space available within the
    //! duration \p d, the operation is aborted an \p false is returned.
    template <typename RepT, typename PeriodT>
    bool try_send_for(element_type element,
                      const chrono::duration<RepT, PeriodT>& d)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (isFull())
        {
            // Note: If we spuriously wakeup, we should not wait again for
            // the same time because then we wait too long.
            if (m_cv_send.wait_for(lock, d) == std::cv_status::timeout)
            {
                if (isFull())
                    return false;
                break;
            }
        }

        m_queue.push_back(element);
        lock.unlock();
        m_cv_receive.notify_one();

        return true;
    }

private:
    //! A mutex to protect the queue.
    std::mutex m_mutex;
    //! The queue to transfer the data.
    std::deque<element_type> m_queue;
    //! This condition variable is triggered whenever something is added to
    //! the queue (i.e. we can receive from it).
    std::condition_variable m_cv_receive;
    //! This condition variable is triggered whenever seomthing is taken from
    //! the queue (i.e. we can send via it).
    std::condition_variable m_cv_send;

    //! Checks if the queue is full.
    bool isFull() const
    {
        return m_queue.size() >= QueueSizeT;
    }
};

} // namespace weos

#endif // WEOS_CXX11_MESSAGEQUEUE_HPP
