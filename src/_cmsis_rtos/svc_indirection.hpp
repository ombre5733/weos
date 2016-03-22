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

#ifndef WEOS_CMSIS_RTOS_SVC_INDIRECTION_HPP
#define WEOS_CMSIS_RTOS_SVC_INDIRECTION_HPP

#if defined(__CC_ARM)

#define SVC_1(fun, retType, A0)                                                \
    __svc_indirect(0)                                                          \
    extern retType fun##_svc(retType (*)(A0), A0);                             \
                                                                               \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0)                                              \
    {                                                                          \
        return fun##_svc(&fun, a0);                                            \
    }

#define SVC_2(fun, retType, A0, A1)                                            \
    __svc_indirect(0)                                                          \
    extern retType fun##_svc(retType (*)(A0, A1), A0, A1);                     \
                                                                               \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0, A1 a1)                                       \
    {                                                                          \
        return fun##_svc(&fun, a0, a1);                                        \
    }

#define SVC_4(fun, retType, A0, A1, A2, A3)                                    \
    __svc_indirect(0)                                                          \
    extern retType fun##_svc(retType (*)(A0, A1, A2, A3), A0, A1, A2, A3);     \
                                                                               \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0, A1 a1, A2 a2, A3 a3)                         \
    {                                                                          \
        return fun##_svc(&fun, a0, a1, a2, a3);                                \
    }

#elif defined(__GNUC__)

#define SVC_Args1(A0)                                                          \
    register A0  arg0 __asm("r0") = a0;                                        \
    register int arg1 __asm("r1");                                             \
    register int arg2 __asm("r2");                                             \
    register int arg3 __asm("r3");

#define SVC_Args2(A0, A1)                                                      \
    register A0  arg0 __asm("r0") = a0;                                        \
    register A1  arg1 __asm("r1") = a1;                                        \
    register int arg2 __asm("r2");                                             \
    register int arg3 __asm("r3");

#define SVC_Args4(A0, A1, A2, A3)                                              \
    register A0 arg0 __asm("r0") = a0;                                         \
    register A1 arg1 __asm("r1") = a1;                                         \
    register A2 arg2 __asm("r2") = a2;                                         \
    register A3 arg3 __asm("r3") = a3;

#define SVC_Call(fun)                                                          \
    __asm volatile(                                                            \
        "ldr r12,=" #fun "\n\t"                                                \
        "svc 0"                                                                \
        : "=r" (arg0), "=r" (arg1), "=r" (arg2), "=r" (arg3)                   \
        :  "r" (arg0),  "r" (arg1),  "r" (arg2),  "r" (arg3)                   \
        : "r12", "lr", "cc");

#define SVC_1(fun, retType, A0)                                                \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0)                                              \
    {                                                                          \
        SVC_Args1(A0)                                                          \
        SVC_Call(fun)                                                          \
        return (retType)arg0;                                                  \
    }

#define SVC_2(fun, retType, A0, A1)                                            \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0, A1 a1)                                       \
    {                                                                          \
        SVC_Args2(A0, A1)                                                      \
        SVC_Call(fun)                                                          \
        return (retType)arg0;                                                  \
    }

#define SVC_4(fun, retType, A0, A1, A2, A3)                                    \
    __attribute__((always_inline)) static inline                               \
    retType fun##_indirect(A0 a0, A1 a1, A2 a2, A3 a3)                         \
    {                                                                          \
        SVC_Args4(A0, A1, A2, A3)                                              \
        SVC_Call(fun)                                                          \
        return (retType)arg0;                                                  \
    }

#else
  #error "Compiler not supported"
#endif

#endif // WEOS_CMSIS_RTOS_SVC_INDIRECTION_HPP
