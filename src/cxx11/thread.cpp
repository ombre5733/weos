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

namespace
{

std::mutex g_idToDataMutex;
std::map<std::thread::id, std::shared_ptr<ThreadData>> g_idToData;

} // anonymous namespace

std::shared_ptr<ThreadData> ThreadData::create(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(g_idToDataMutex);
    std::shared_ptr<ThreadData> data = std::make_shared<ThreadData>();
    g_idToData[id] = data;
    return data;
}

std::shared_ptr<ThreadData> ThreadData::find(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(g_idToDataMutex);
    auto iter = g_idToData.find(id);
    if (iter != g_idToData.end())
        return iter->second;
    else
        return std::shared_ptr<ThreadData>();
}

void ThreadData::remove(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(g_idToDataMutex);
    auto iter = g_idToData.find(id);
    if (iter != g_idToData.end())
        g_idToData.erase(iter);
}

} // namespace detail
} // namespace weos
