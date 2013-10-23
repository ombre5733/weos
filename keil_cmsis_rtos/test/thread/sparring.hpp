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

#ifndef WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP
#define WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP

#include "../../thread.hpp"

struct SparringData
{
    enum Action
    {
        None,
        WaitForAnySignal,
        WaitForSignalMask,
        TryWaitForSignal,
        TryWaitForSignalFor,
        Terminate
    };

    SparringData()
        : action(None),
          busy(false),
          caughtMask(0),
          waitMask(0),
          sparringStarted(false)
    {
    }

    volatile Action action;
    volatile bool busy;
    volatile std::uint32_t caughtMask;
    volatile std::uint32_t waitMask;
    volatile bool sparringStarted;
};

void sparring(void* arg)
{
    SparringData* data = static_cast<SparringData*>(arg);
    data->sparringStarted = true;

    while (1)
    {
        if (data->action == SparringData::None)
        {
            osDelay(1);
            continue;
        }
        else if (data->action == SparringData::Terminate)
            break;

        data->busy = true;
        switch (data->action)
        {
            case SparringData::WaitForAnySignal:
                data->caughtMask = weos::this_thread::wait_for_signal(weos::any_signal);
                break;
            case SparringData::WaitForSignalMask:
                data->caughtMask = weos::this_thread::wait_for_signal(data->waitMask);
                break;
            default:
                break;
        }
        data->busy = false;
        data->action = SparringData::None;
    }
}

#endif // WEOS_KEIL_CMSIS_RTOS_TEST_SPARRING_HPP
