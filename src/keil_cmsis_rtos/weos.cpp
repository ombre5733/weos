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

#include "core.hpp"

#include "_tq.cpp"
#include "condition_variable.cpp"
#include "chrono.cpp"
#include "mutex.cpp"
#include "semaphore.cpp"
#include "system_error.cpp"
#include "thread.cpp"


WEOS_BEGIN_NAMESPACE

class Weos
{
public:
    Weos();
    ~Weos();

    Weos(const Weos&) = delete;
    Weos& operator=(const Weos&) = delete;

private:
    thread m_precisionTimeReader;
};

Weos::Weos()
    : m_precisionTimeReader(
          thread_attributes().setPriority(thread::attributes::priority::low)
                             .setStack(precisionTimeReaderStack),
          &readPrecisionTimePeriodically)
{
}

Weos::~Weos()
{
    m_precisionTimeReader.set_signals(1);
    m_precisionTimeReader.join();
}

WEOS_END_NAMESPACE
