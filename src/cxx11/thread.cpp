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

#include "thread.hpp"

#include <map>

namespace weos
{
namespace detail
{

ThreadDataManager& ThreadDataManager::instance()
{
    static ThreadDataManager manager;
    return manager;
}

void ThreadDataManager::add(std::thread::id id, ThreadData* data)
{
    std::lock_guard<std::mutex> lock(m_idToDataMutex);
    WEOS_ASSERT(m_idToData.find(id) == m_idToData.cend());
    m_idToData[id] = data;
}

ThreadData* ThreadDataManager::find(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_idToDataMutex);
    auto iter = m_idToData.find(id);
    if (iter != m_idToData.end())
        return iter->second;
    else
        return nullptr;
}

void ThreadDataManager::remove(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_idToDataMutex);
    auto iter = m_idToData.find(id);
    if (iter != m_idToData.end())
        m_idToData.erase(iter);
}

} // namespace detail
} // namespace weos
