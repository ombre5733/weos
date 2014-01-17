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

/* A startup script for the STM32F4xx microcontroller.

   The startup script references the following symbols which are defined in
   the linker script:
   __etext        ... The start of the initialized data in the FLASH (the source for copying).
   __data_start__ ... The start of the initialized data in the RAM (the target for copying).
   __data_end__   ... The end of the initialized data in the RAM.
   __bss_start__  ... The start of the uninitialized data section in the RAM.
   __bss_end__    ... The end of the uninitialized data section in the RAM.
   __stack        ... The initial value for the stack pointer.
*/

    .syntax unified
    .arch armv7-m

/*--========================================================================----
    Stack and heap
----========================================================================--*/

/* The size of the stack in bytes. Has to be a multiple of 8. */
    .set    Stack_Size, 0x00000800

/* The size of the heap in bytes. Has to be a multiple of 8. */
    .set    Heap_Size,  0x00008000


    .section .stack
    .align 3
    .globl    __stack_top__
    .globl    __stack_limit__
__stack_limit__:
    .if       Stack_Size
    .space    Stack_Size
    .endif
    .size __stack_limit__, . - __stack_limit__
__stack_top__:
    .size __stack_top__, . - __stack_top__


    .section .heap
    .align 3
    .globl    __HeapBase
    .globl    __heap_end__
__HeapBase:
    .if       Heap_Size
    .space    Heap_Size
    .endif
    .size __HeapBase, . - __HeapBase
__heap_end__:
    .size __heap_end__, . - __heap_end__

/*--========================================================================----
    Interrupt vector table
----========================================================================--*/

    .section .isr_vector
    .align 2
    .globl  __isr_vector
__isr_vector:
    /* ---- Cortex-M4 vectors --------------------------------------------------------------- */
    .word    __stack_top__                     /* Initial stack-pointer value                 */
    .word    Reset_Handler                     /* Reset handler                               */
    .word    NMI_Handler                       /* NMI handler                                 */
    .word    HardFault_Handler                 /* Hard fault handler                          */
    .word    MemManage_Handler                 /* Memory management fault handler             */
    .word    BusFault_Handler                  /* Bus fault handler                           */
    .word    UsageFault_Handler                /* Usage fault handler                         */
    .word    0
    .word    0
    .word    0
    .word    0
    .word    SVC_Handler                       /* SVCall handler                              */
    .word    DebugMon_Handler                  /* Debug monitor handler                       */
    .word    0
    .word    PendSV_Handler                    /* PendSV handler                              */
    .word    SysTick_Handler                   /* SysTick handler                             */

    /* ---- External interrupts ------------------------------------------------------------- */
    .word    WWDG_IRQHandler                   /* Window Watch-dog                            */
    .word    PVD_IRQHandler                    /* PVD through EXTI Line detection             */
    .word    TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
    .word    RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line            */
    .word    FLASH_IRQHandler                  /* FLASH                                       */
    .word    RCC_IRQHandler                    /* RCC                                         */
    .word    EXTI0_IRQHandler                  /* EXTI Line0                                  */
    .word    EXTI1_IRQHandler                  /* EXTI Line1                                  */
    .word    EXTI2_IRQHandler                  /* EXTI Line2                                  */
    .word    EXTI3_IRQHandler                  /* EXTI Line3                                  */
    .word    EXTI4_IRQHandler                  /* EXTI Line4                                  */
    .word    DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                               */
    .word    DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                               */
    .word    DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                               */
    .word    DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                               */
    .word    DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                               */
    .word    DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                               */
    .word    DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                               */
    .word    ADC_IRQHandler                    /* ADC1, ADC2 and ADC3                         */
    .word    CAN1_TX_IRQHandler                /* CAN1 TX                                     */
    .word    CAN1_RX0_IRQHandler               /* CAN1 RX0                                    */
    .word    CAN1_RX1_IRQHandler               /* CAN1 RX1                                    */
    .word    CAN1_SCE_IRQHandler               /* CAN1 SCE                                    */
    .word    EXTI9_5_IRQHandler                /* External Line[9:5]                          */
    .word    TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9                         */
    .word    TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10                       */
    .word    TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11      */
    .word    TIM1_CC_IRQHandler                /* TIM1 Capture Compare                        */
    .word    TIM2_IRQHandler                   /* TIM2                                        */
    .word    TIM3_IRQHandler                   /* TIM3                                        */
    .word    TIM4_IRQHandler                   /* TIM4                                        */
    .word    I2C1_EV_IRQHandler                /* I2C1 Event                                  */
    .word    I2C1_ER_IRQHandler                /* I2C1 Error                                  */
    .word    I2C2_EV_IRQHandler                /* I2C2 Event                                  */
    .word    I2C2_ER_IRQHandler                /* I2C2 Error                                  */
    .word    SPI1_IRQHandler                   /* SPI1                                        */
    .word    SPI2_IRQHandler                   /* SPI2                                        */
    .word    USART1_IRQHandler                 /* USART1                                      */
    .word    USART2_IRQHandler                 /* USART2                                      */
    .word    USART3_IRQHandler                 /* USART3                                      */
    .word    EXTI15_10_IRQHandler              /* External Line[15:10]                        */
    .word    RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line       */
    .word    OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line         */
    .word    TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12                        */
    .word    TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13                       */
    .word    TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14      */
    .word    TIM8_CC_IRQHandler                /* TIM8 Capture Compare                        */
    .word    DMA1_Stream7_IRQHandler           /* DMA1 Stream7                                */
    .word    FSMC_IRQHandler                   /* FSMC                                        */
    .word    SDIO_IRQHandler                   /* SDIO                                        */
    .word    TIM5_IRQHandler                   /* TIM5                                        */
    .word    SPI3_IRQHandler                   /* SPI3                                        */
    .word    UART4_IRQHandler                  /* UART4                                       */
    .word    UART5_IRQHandler                  /* UART5                                       */
    .word    TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors             */
    .word    TIM7_IRQHandler                   /* TIM7                                        */
    .word    DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                               */
    .word    DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                               */
    .word    DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                               */
    .word    DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                               */
    .word    DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                               */
    .word    ETH_IRQHandler                    /* Ethernet                                    */
    .word    ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line           */
    .word    CAN2_TX_IRQHandler                /* CAN2 TX                                     */
    .word    CAN2_RX0_IRQHandler               /* CAN2 RX0                                    */
    .word    CAN2_RX1_IRQHandler               /* CAN2 RX1                                    */
    .word    CAN2_SCE_IRQHandler               /* CAN2 SCE                                    */
    .word    OTG_FS_IRQHandler                 /* USB OTG FS                                  */
    .word    DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                               */
    .word    DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                               */
    .word    DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                               */
    .word    USART6_IRQHandler                 /* USART6                                      */
    .word    I2C3_EV_IRQHandler                /* I2C3 event                                  */
    .word    I2C3_ER_IRQHandler                /* I2C3 error                                  */
    .word    OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out                  */
    .word    OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In                   */
    .word    OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI              */
    .word    OTG_HS_IRQHandler                 /* USB OTG HS                                  */
    .word    DCMI_IRQHandler                   /* DCMI                                        */
    .word    CRYP_IRQHandler                   /* CRYP crypto                                 */
    .word    HASH_RNG_IRQHandler               /* Hash and Rng                                */
    .word    FPU_IRQHandler                    /* FPU                                         */

    .size  __isr_vector, . - __isr_vector

/*--========================================================================----
    Reset handler
----========================================================================--*/

    .text
    .thumb
    .thumb_func
    .align 2
    .globl   Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:

    /* The init-values for the initialized data section .data has to be copied
       from the FLASH to the RAM, i.e.
       __data_start__[i] = __etext[i]   for i = 0 .. (__data_end__ - __data_start__) */
copyIniData:
    ldr     r1, =__etext          @ Load the start address of .data in the FLASH.
    ldr     r2, =__data_start__   @ Load the start address of .data in the RAM.
    ldr     r3, =__data_end__     @ Load the end address of .data in the RAM.
    subs    r3, r2
    ble     doneIniData
copyLoop:
    subs    r3, #4
    ldr     r0, [r1, r3]
    str     r0, [r2, r3]
    bgt     copyLoop
doneIniData:

    /* Zero the .bss section. */
zeroBss:
    ldr     r1, =__bss_start__    @ Load the start address of .bss.
    ldr     r2, =__bss_end__      @ Load the end address of .bess.
    movs    r0, 0
zeroLoop:
    cmp     r1, r2
    itt     lt
    strlt   r0, [r1], #4
    blt     zeroLoop

    /* Call the system intitialization function. */
#ifndef __NO_SYSTEM_INIT
    ldr     r0, =SystemInit
    blx     r0
#endif
    /* Call the application's entry point. */
    ldr     r0, =_start
    bx      r0
    /* Here the assembler can put data for immediate 32bit loads. */
    .pool

    .size Reset_Handler, . - Reset_Handler

/*--========================================================================----
    Cortex-M4 exception handler stubs.
    These handlers are declared weak and can be overridden easily. The
    default implementation is an infinite loop.
----========================================================================--*/

    /* This macro defines a default handler named handlerName.
       The implementation is simply an infinite loop.
     */
    .macro    defineIrqHandler    handlerName
    .align 1
    .thumb_func
    .weak    \handlerName
    .type    \handlerName, %function
\handlerName :
    b    .
    .size \handlerName, . - \handlerName
    .endm

defineIrqHandler    NMI_Handler
defineIrqHandler    HardFault_Handler
defineIrqHandler    MemManage_Handler
defineIrqHandler    BusFault_Handler
defineIrqHandler    UsageFault_Handler
defineIrqHandler    SVC_Handler
defineIrqHandler    DebugMon_Handler
defineIrqHandler    PendSV_Handler
defineIrqHandler    SysTick_Handler

/*--========================================================================----
    Interrupt handler stubs.
    These handlers are declared weak and can be overridden easily. The
    default implementation is an infinite loop.
----========================================================================--*/

defineIrqHandler    Default_Handler

    .weak       WWDG_IRQHandler
    .set        WWDG_IRQHandler, Default_Handler

    .weak       PVD_IRQHandler
    .set        PVD_IRQHandler, Default_Handler

    .weak       TAMP_STAMP_IRQHandler
    .set        TAMP_STAMP_IRQHandler, Default_Handler

    .weak       RTC_WKUP_IRQHandler
    .set        RTC_WKUP_IRQHandler, Default_Handler

    .weak       FLASH_IRQHandler
    .set        FLASH_IRQHandler, Default_Handler

    .weak       RCC_IRQHandler
    .set        RCC_IRQHandler, Default_Handler

    .weak       EXTI0_IRQHandler
    .set        EXTI0_IRQHandler, Default_Handler

    .weak       EXTI1_IRQHandler
    .set        EXTI1_IRQHandler, Default_Handler

    .weak       EXTI2_IRQHandler
    .set        EXTI2_IRQHandler, Default_Handler

    .weak       EXTI3_IRQHandler
    .set        EXTI3_IRQHandler, Default_Handler

    .weak       EXTI4_IRQHandler
    .set        EXTI4_IRQHandler, Default_Handler

    .weak       DMA1_Stream0_IRQHandler
    .set        DMA1_Stream0_IRQHandler, Default_Handler

    .weak       DMA1_Stream1_IRQHandler
    .set        DMA1_Stream1_IRQHandler, Default_Handler

    .weak       DMA1_Stream2_IRQHandler
    .set        DMA1_Stream2_IRQHandler, Default_Handler

    .weak       DMA1_Stream3_IRQHandler
    .set        DMA1_Stream3_IRQHandler, Default_Handler

    .weak       DMA1_Stream4_IRQHandler
    .set        DMA1_Stream4_IRQHandler, Default_Handler

    .weak       DMA1_Stream5_IRQHandler
    .set        DMA1_Stream5_IRQHandler, Default_Handler

    .weak       DMA1_Stream6_IRQHandler
    .set        DMA1_Stream6_IRQHandler, Default_Handler

    .weak       ADC_IRQHandler
    .set        ADC_IRQHandler, Default_Handler

    .weak       CAN1_TX_IRQHandler
    .set        CAN1_TX_IRQHandler, Default_Handler

    .weak       CAN1_RX0_IRQHandler
    .set        CAN1_RX0_IRQHandler, Default_Handler

    .weak       CAN1_RX1_IRQHandler
    .set        CAN1_RX1_IRQHandler, Default_Handler

    .weak       CAN1_SCE_IRQHandler
    .set        CAN1_SCE_IRQHandler, Default_Handler

    .weak       EXTI9_5_IRQHandler
    .set        EXTI9_5_IRQHandler, Default_Handler

    .weak       TIM1_BRK_TIM9_IRQHandler
    .set        TIM1_BRK_TIM9_IRQHandler, Default_Handler

    .weak       TIM1_UP_TIM10_IRQHandler
    .set        TIM1_UP_TIM10_IRQHandler, Default_Handler

    .weak       TIM1_TRG_COM_TIM11_IRQHandler
    .set        TIM1_TRG_COM_TIM11_IRQHandler, Default_Handler

    .weak       TIM1_CC_IRQHandler
    .set        TIM1_CC_IRQHandler, Default_Handler

    .weak       TIM2_IRQHandler
    .set        TIM2_IRQHandler, Default_Handler

    .weak       TIM3_IRQHandler
    .set        TIM3_IRQHandler, Default_Handler

    .weak       TIM4_IRQHandler
    .set        TIM4_IRQHandler, Default_Handler

    .weak       I2C1_EV_IRQHandler
    .set        I2C1_EV_IRQHandler, Default_Handler

    .weak       I2C1_ER_IRQHandler
    .set        I2C1_ER_IRQHandler, Default_Handler

    .weak       I2C2_EV_IRQHandler
    .set        I2C2_EV_IRQHandler, Default_Handler

    .weak       I2C2_ER_IRQHandler
    .set        I2C2_ER_IRQHandler, Default_Handler

    .weak       SPI1_IRQHandler
    .set        SPI1_IRQHandler, Default_Handler

    .weak       SPI2_IRQHandler
    .set        SPI2_IRQHandler, Default_Handler

    .weak       USART1_IRQHandler
    .set        USART1_IRQHandler, Default_Handler

    .weak       USART2_IRQHandler
    .set        USART2_IRQHandler, Default_Handler

    .weak       USART3_IRQHandler
    .set        USART3_IRQHandler, Default_Handler

    .weak       EXTI15_10_IRQHandler
    .set        EXTI15_10_IRQHandler, Default_Handler

    .weak       RTC_Alarm_IRQHandler
    .set        RTC_Alarm_IRQHandler, Default_Handler

    .weak       OTG_FS_WKUP_IRQHandler
    .set        OTG_FS_WKUP_IRQHandler, Default_Handler

    .weak       TIM8_BRK_TIM12_IRQHandler
    .set        TIM8_BRK_TIM12_IRQHandler, Default_Handler

    .weak       TIM8_UP_TIM13_IRQHandler
    .set        TIM8_UP_TIM13_IRQHandler, Default_Handler

    .weak       TIM8_TRG_COM_TIM14_IRQHandler
    .set        TIM8_TRG_COM_TIM14_IRQHandler, Default_Handler

    .weak       TIM8_CC_IRQHandler
    .set        TIM8_CC_IRQHandler, Default_Handler

    .weak       DMA1_Stream7_IRQHandler
    .set        DMA1_Stream7_IRQHandler, Default_Handler

    .weak       FSMC_IRQHandler
    .set        FSMC_IRQHandler, Default_Handler

    .weak       SDIO_IRQHandler
    .set        SDIO_IRQHandler, Default_Handler

    .weak       TIM5_IRQHandler
    .set        TIM5_IRQHandler, Default_Handler

    .weak       SPI3_IRQHandler
    .set        SPI3_IRQHandler, Default_Handler

    .weak       UART4_IRQHandler
    .set        UART4_IRQHandler, Default_Handler

    .weak       UART5_IRQHandler
    .set        UART5_IRQHandler, Default_Handler

    .weak       TIM6_DAC_IRQHandler
    .set        TIM6_DAC_IRQHandler, Default_Handler

    .weak       TIM7_IRQHandler
    .set        TIM7_IRQHandler, Default_Handler

    .weak       DMA2_Stream0_IRQHandler
    .set        DMA2_Stream0_IRQHandler, Default_Handler

    .weak       DMA2_Stream1_IRQHandler
    .set        DMA2_Stream1_IRQHandler, Default_Handler

    .weak       DMA2_Stream2_IRQHandler
    .set        DMA2_Stream2_IRQHandler, Default_Handler

    .weak       DMA2_Stream3_IRQHandler
    .set        DMA2_Stream3_IRQHandler, Default_Handler

    .weak       DMA2_Stream4_IRQHandler
    .set        DMA2_Stream4_IRQHandler, Default_Handler

    .weak       ETH_IRQHandler
    .set        ETH_IRQHandler, Default_Handler

    .weak       ETH_WKUP_IRQHandler
    .set        ETH_WKUP_IRQHandler, Default_Handler

    .weak       CAN2_TX_IRQHandler
    .set        CAN2_TX_IRQHandler, Default_Handler

    .weak       CAN2_RX0_IRQHandler
    .set        CAN2_RX0_IRQHandler, Default_Handler

    .weak       CAN2_RX1_IRQHandler
    .set        CAN2_RX1_IRQHandler, Default_Handler

    .weak       CAN2_SCE_IRQHandler
    .set        CAN2_SCE_IRQHandler, Default_Handler

    .weak       OTG_FS_IRQHandler
    .set        OTG_FS_IRQHandler, Default_Handler

    .weak       DMA2_Stream5_IRQHandler
    .set        DMA2_Stream5_IRQHandler, Default_Handler

    .weak       DMA2_Stream6_IRQHandler
    .set        DMA2_Stream6_IRQHandler, Default_Handler

    .weak       DMA2_Stream7_IRQHandler
    .set        DMA2_Stream7_IRQHandler, Default_Handler

    .weak       USART6_IRQHandler
    .set        USART6_IRQHandler, Default_Handler

    .weak       I2C3_EV_IRQHandler
    .set        I2C3_EV_IRQHandler, Default_Handler

    .weak       I2C3_ER_IRQHandler
    .set        I2C3_ER_IRQHandler, Default_Handler

    .weak       OTG_HS_EP1_OUT_IRQHandler
    .set        OTG_HS_EP1_OUT_IRQHandler, Default_Handler

    .weak       OTG_HS_EP1_IN_IRQHandler
    .set        OTG_HS_EP1_IN_IRQHandler, Default_Handler

    .weak       OTG_HS_WKUP_IRQHandler
    .set        OTG_HS_WKUP_IRQHandler, Default_Handler

    .weak       OTG_HS_IRQHandler
    .set        OTG_HS_IRQHandler, Default_Handler

    .weak       DCMI_IRQHandler
    .set        DCMI_IRQHandler, Default_Handler

    .weak       CRYP_IRQHandler
    .set        CRYP_IRQHandler, Default_Handler

    .weak       HASH_RNG_IRQHandler
    .set        HASH_RNG_IRQHandler, Default_Handler

    .weak       FPU_IRQHandler
    .set        FPU_IRQHandler, Default_Handler

    .end
