/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
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

#ifndef WEOS_KEIL_CMSIS_RTOS_EXCEPTION_HPP
#define WEOS_KEIL_CMSIS_RTOS_EXCEPTION_HPP

#include "core.hpp"

#include "error.hpp"
#include "../objectpool.hpp"


WEOS_BEGIN_NAMESPACE

class error_code_node
{
public:
    error_code_node(error_code code)
        : m_code(code),
          m_next(0)
    {
    }

    void ref();
    void deref();

private:
    typedef object_pool<error_code_node, WEOS_NUM_ERROR_CODE_NODES, mutex> pool_t;

    static pool_t& pool();

    error_code m_code;
    error_code_node* m_next;
};

class exception
{
public:
    void addErrorCode(error_code code);

private:
    error_code_node* m_errorCodeList;
};

WEOS_END_NAMESPACE

#endif // WEOS_KEIL_CMSIS_RTOS_EXCEPTION_HPP
