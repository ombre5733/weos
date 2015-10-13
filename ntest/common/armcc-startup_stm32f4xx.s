;*******************************************************************************
; WEOS - Wrapper for embedded operating systems
;
; Copyright (c) 2013-2015, Manuel Freiberger
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
; - Redistributions of source code must retain the above copyright notice, this
;   list of conditions and the following disclaimer.
; - Redistributions in binary form must reproduce the above copyright notice,
;   this list of conditions and the following disclaimer in the documentation
;   and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;*******************************************************************************

; ----======================================================================----
;     Stack and heap
; ----======================================================================----

; The size of the stack in bytes. Has to be a multiple of 8.
Stack_Size         EQU       0x00000800

; The size of the heap in bytes. Has to be a multiple of 8.
Heap_Size          EQU       0x00008000



                   AREA      STACK, ALIGN=3, NOINIT, READWRITE
Stack_Memory       SPACE     Stack_Size
__stack_top__



                   AREA      HEAP, ALIGN=3, NOINIT, READWRITE
__heap_base
Heap_Memory        SPACE     Heap_Size
__heap_limit


; ----======================================================================----
;     Interrupt vector table
; ----======================================================================----

                   PRESERVE8
                   THUMB

                   AREA      RESET, DATA, READONLY

                   ; ---- Cortex-M4 vectors ------------------------------------
__isr_vector       DCD       __stack_top__                 ; Initial stack-pointer value
                   DCD       Reset_Handler                 ; Reset Handler
                   DCD       NMI_Handler                   ; NMI Handler
                   DCD       HardFault_Handler             ; Hard Fault Handler
                   DCD       MemManage_Handler             ; MPU Fault Handler
                   DCD       BusFault_Handler              ; Bus Fault Handler
                   DCD       UsageFault_Handler            ; Usage Fault Handler
                   DCD       0
                   DCD       0
                   DCD       0
                   DCD       0
                   DCD       SVC_Handler                   ; SVCall Handler
                   DCD       DebugMon_Handler              ; Debug Monitor Handler
                   DCD       0
                   DCD       PendSV_Handler                ; PendSV Handler
                   DCD       SysTick_Handler               ; SysTick Handler

                   ; ---- External interrupts ----------------------------------
                   DCD       WWDG_IRQHandler               ; Window WatchDog
                   DCD       PVD_IRQHandler                ; PVD through EXTI Line detection
                   DCD       TAMP_STAMP_IRQHandler         ; Tamper and TimeStamps through the EXTI line
                   DCD       RTC_WKUP_IRQHandler           ; RTC Wakeup through the EXTI line
                   DCD       FLASH_IRQHandler              ; FLASH
                   DCD       RCC_IRQHandler                ; RCC
                   DCD       EXTI0_IRQHandler              ; EXTI Line0
                   DCD       EXTI1_IRQHandler              ; EXTI Line1
                   DCD       EXTI2_IRQHandler              ; EXTI Line2
                   DCD       EXTI3_IRQHandler              ; EXTI Line3
                   DCD       EXTI4_IRQHandler              ; EXTI Line4
                   DCD       DMA1_Stream0_IRQHandler       ; DMA1 Stream 0
                   DCD       DMA1_Stream1_IRQHandler       ; DMA1 Stream 1
                   DCD       DMA1_Stream2_IRQHandler       ; DMA1 Stream 2
                   DCD       DMA1_Stream3_IRQHandler       ; DMA1 Stream 3
                   DCD       DMA1_Stream4_IRQHandler       ; DMA1 Stream 4
                   DCD       DMA1_Stream5_IRQHandler       ; DMA1 Stream 5
                   DCD       DMA1_Stream6_IRQHandler       ; DMA1 Stream 6
                   DCD       ADC_IRQHandler                ; ADC1, ADC2 and ADC3s
                   DCD       CAN1_TX_IRQHandler            ; CAN1 TX
                   DCD       CAN1_RX0_IRQHandler           ; CAN1 RX0
                   DCD       CAN1_RX1_IRQHandler           ; CAN1 RX1
                   DCD       CAN1_SCE_IRQHandler           ; CAN1 SCE
                   DCD       EXTI9_5_IRQHandler            ; External Line[9:5]s
                   DCD       TIM1_BRK_TIM9_IRQHandler      ; TIM1 Break and TIM9
                   DCD       TIM1_UP_TIM10_IRQHandler      ; TIM1 Update and TIM10
                   DCD       TIM1_TRG_COM_TIM11_IRQHandler ; TIM1 Trigger and Commutation and TIM11
                   DCD       TIM1_CC_IRQHandler            ; TIM1 Capture Compare
                   DCD       TIM2_IRQHandler               ; TIM2
                   DCD       TIM3_IRQHandler               ; TIM3
                   DCD       TIM4_IRQHandler               ; TIM4
                   DCD       I2C1_EV_IRQHandler            ; I2C1 Event
                   DCD       I2C1_ER_IRQHandler            ; I2C1 Error
                   DCD       I2C2_EV_IRQHandler            ; I2C2 Event
                   DCD       I2C2_ER_IRQHandler            ; I2C2 Error
                   DCD       SPI1_IRQHandler               ; SPI1
                   DCD       SPI2_IRQHandler               ; SPI2
                   DCD       USART1_IRQHandler             ; USART1
                   DCD       USART2_IRQHandler             ; USART2
                   DCD       USART3_IRQHandler             ; USART3
                   DCD       EXTI15_10_IRQHandler          ; External Line[15:10]s
                   DCD       RTC_Alarm_IRQHandler          ; RTC Alarm (A and B) through EXTI Line
                   DCD       OTG_FS_WKUP_IRQHandler        ; USB OTG FS Wakeup through EXTI line
                   DCD       TIM8_BRK_TIM12_IRQHandler     ; TIM8 Break and TIM12
                   DCD       TIM8_UP_TIM13_IRQHandler      ; TIM8 Update and TIM13
                   DCD       TIM8_TRG_COM_TIM14_IRQHandler ; TIM8 Trigger and Commutation and TIM14
                   DCD       TIM8_CC_IRQHandler            ; TIM8 Capture Compare
                   DCD       DMA1_Stream7_IRQHandler       ; DMA1 Stream7
                   DCD       FSMC_IRQHandler               ; FSMC
                   DCD       SDIO_IRQHandler               ; SDIO
                   DCD       TIM5_IRQHandler               ; TIM5
                   DCD       SPI3_IRQHandler               ; SPI3
                   DCD       UART4_IRQHandler              ; UART4
                   DCD       UART5_IRQHandler              ; UART5
                   DCD       TIM6_DAC_IRQHandler           ; TIM6 and DAC1&2 underrun errors
                   DCD       TIM7_IRQHandler               ; TIM7
                   DCD       DMA2_Stream0_IRQHandler       ; DMA2 Stream 0
                   DCD       DMA2_Stream1_IRQHandler       ; DMA2 Stream 1
                   DCD       DMA2_Stream2_IRQHandler       ; DMA2 Stream 2
                   DCD       DMA2_Stream3_IRQHandler       ; DMA2 Stream 3
                   DCD       DMA2_Stream4_IRQHandler       ; DMA2 Stream 4
                   DCD       ETH_IRQHandler                ; Ethernet
                   DCD       ETH_WKUP_IRQHandler           ; Ethernet Wakeup through EXTI line
                   DCD       CAN2_TX_IRQHandler            ; CAN2 TX
                   DCD       CAN2_RX0_IRQHandler           ; CAN2 RX0
                   DCD       CAN2_RX1_IRQHandler           ; CAN2 RX1
                   DCD       CAN2_SCE_IRQHandler           ; CAN2 SCE
                   DCD       OTG_FS_IRQHandler             ; USB OTG FS
                   DCD       DMA2_Stream5_IRQHandler       ; DMA2 Stream 5
                   DCD       DMA2_Stream6_IRQHandler       ; DMA2 Stream 6
                   DCD       DMA2_Stream7_IRQHandler       ; DMA2 Stream 7
                   DCD       USART6_IRQHandler             ; USART6
                   DCD       I2C3_EV_IRQHandler            ; I2C3 event
                   DCD       I2C3_ER_IRQHandler            ; I2C3 error
                   DCD       OTG_HS_EP1_OUT_IRQHandler     ; USB OTG HS End Point 1 Out
                   DCD       OTG_HS_EP1_IN_IRQHandler      ; USB OTG HS End Point 1 In
                   DCD       OTG_HS_WKUP_IRQHandler        ; USB OTG HS Wakeup through EXTI
                   DCD       OTG_HS_IRQHandler             ; USB OTG HS
                   DCD       DCMI_IRQHandler               ; DCMI
                   DCD       CRYP_IRQHandler               ; CRYP crypto
                   DCD       HASH_RNG_IRQHandler           ; Hash and Rng
                   DCD       FPU_IRQHandler                ; FPU


; ----======================================================================----
;     Reset handler
; ----======================================================================----

                   AREA      |.text|, CODE, READONLY

Reset_Handler      PROC
                   EXPORT    Reset_Handler                 [WEAK]
                   IMPORT    SystemInit
                   IMPORT    __main

                   ; Call the system initialization function.
                   LDR       R0, =SystemInit
                   BLX       R0
                   ; Call the application's entry point.
                   LDR       R0, =__main
                   BX        R0
                   ENDP


; ----======================================================================----
;     Cortex-M4 exception handler stubs.
;     These handlers are declared weak and can be overridden easily. The
;     default implementation is an infinite loop.
; ----======================================================================----

                   ; This macro defines a default handler named handlerName.
                   ; The implementation is simply an infinite loop.
                   MACRO
$handlerName       defineIrqHandler
$handlerName       PROC
                   EXPORT    $handlerName                  [WEAK]
                   B         .
                   ENDP
                   MEND


NMI_Handler        defineIrqHandler
HardFault_Handler  defineIrqHandler
MemManage_Handler  defineIrqHandler
BusFault_Handler   defineIrqHandler
UsageFault_Handler defineIrqHandler
SVC_Handler        defineIrqHandler
DebugMon_Handler   defineIrqHandler
PendSV_Handler     defineIrqHandler
SysTick_Handler    defineIrqHandler


; ----======================================================================----
;     Interrupt handler stubs.
;     These handlers are declared weak and can be overridden easily. The
;     default implementation is an infinite loop.
; ----======================================================================----

Default_Handler    defineIrqHandler

                   EXPORT    WWDG_IRQHandler               [WEAK]
                   EXPORT    PVD_IRQHandler                [WEAK]
                   EXPORT    TAMP_STAMP_IRQHandler         [WEAK]
                   EXPORT    RTC_WKUP_IRQHandler           [WEAK]
                   EXPORT    FLASH_IRQHandler              [WEAK]
                   EXPORT    RCC_IRQHandler                [WEAK]
                   EXPORT    EXTI0_IRQHandler              [WEAK]
                   EXPORT    EXTI1_IRQHandler              [WEAK]
                   EXPORT    EXTI2_IRQHandler              [WEAK]
                   EXPORT    EXTI3_IRQHandler              [WEAK]
                   EXPORT    EXTI4_IRQHandler              [WEAK]
                   EXPORT    DMA1_Stream0_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream1_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream2_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream3_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream4_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream5_IRQHandler       [WEAK]
                   EXPORT    DMA1_Stream6_IRQHandler       [WEAK]
                   EXPORT    ADC_IRQHandler                [WEAK]
                   EXPORT    CAN1_TX_IRQHandler            [WEAK]
                   EXPORT    CAN1_RX0_IRQHandler           [WEAK]
                   EXPORT    CAN1_RX1_IRQHandler           [WEAK]
                   EXPORT    CAN1_SCE_IRQHandler           [WEAK]
                   EXPORT    EXTI9_5_IRQHandler            [WEAK]
                   EXPORT    TIM1_BRK_TIM9_IRQHandler      [WEAK]
                   EXPORT    TIM1_UP_TIM10_IRQHandler      [WEAK]
                   EXPORT    TIM1_TRG_COM_TIM11_IRQHandler [WEAK]
                   EXPORT    TIM1_CC_IRQHandler            [WEAK]
                   EXPORT    TIM2_IRQHandler               [WEAK]
                   EXPORT    TIM3_IRQHandler               [WEAK]
                   EXPORT    TIM4_IRQHandler               [WEAK]
                   EXPORT    I2C1_EV_IRQHandler            [WEAK]
                   EXPORT    I2C1_ER_IRQHandler            [WEAK]
                   EXPORT    I2C2_EV_IRQHandler            [WEAK]
                   EXPORT    I2C2_ER_IRQHandler            [WEAK]
                   EXPORT    SPI1_IRQHandler               [WEAK]
                   EXPORT    SPI2_IRQHandler               [WEAK]
                   EXPORT    USART1_IRQHandler             [WEAK]
                   EXPORT    USART2_IRQHandler             [WEAK]
                   EXPORT    USART3_IRQHandler             [WEAK]
                   EXPORT    EXTI15_10_IRQHandler          [WEAK]
                   EXPORT    RTC_Alarm_IRQHandler          [WEAK]
                   EXPORT    OTG_FS_WKUP_IRQHandler        [WEAK]
                   EXPORT    TIM8_BRK_TIM12_IRQHandler     [WEAK]
                   EXPORT    TIM8_UP_TIM13_IRQHandler      [WEAK]
                   EXPORT    TIM8_TRG_COM_TIM14_IRQHandler [WEAK]
                   EXPORT    TIM8_CC_IRQHandler            [WEAK]
                   EXPORT    DMA1_Stream7_IRQHandler       [WEAK]
                   EXPORT    FSMC_IRQHandler               [WEAK]
                   EXPORT    SDIO_IRQHandler               [WEAK]
                   EXPORT    TIM5_IRQHandler               [WEAK]
                   EXPORT    SPI3_IRQHandler               [WEAK]
                   EXPORT    UART4_IRQHandler              [WEAK]
                   EXPORT    UART5_IRQHandler              [WEAK]
                   EXPORT    TIM6_DAC_IRQHandler           [WEAK]
                   EXPORT    TIM7_IRQHandler               [WEAK]
                   EXPORT    DMA2_Stream0_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream1_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream2_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream3_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream4_IRQHandler       [WEAK]
                   EXPORT    ETH_IRQHandler                [WEAK]
                   EXPORT    ETH_WKUP_IRQHandler           [WEAK]
                   EXPORT    CAN2_TX_IRQHandler            [WEAK]
                   EXPORT    CAN2_RX0_IRQHandler           [WEAK]
                   EXPORT    CAN2_RX1_IRQHandler           [WEAK]
                   EXPORT    CAN2_SCE_IRQHandler           [WEAK]
                   EXPORT    OTG_FS_IRQHandler             [WEAK]
                   EXPORT    DMA2_Stream5_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream6_IRQHandler       [WEAK]
                   EXPORT    DMA2_Stream7_IRQHandler       [WEAK]
                   EXPORT    USART6_IRQHandler             [WEAK]
                   EXPORT    I2C3_EV_IRQHandler            [WEAK]
                   EXPORT    I2C3_ER_IRQHandler            [WEAK]
                   EXPORT    OTG_HS_EP1_OUT_IRQHandler     [WEAK]
                   EXPORT    OTG_HS_EP1_IN_IRQHandler      [WEAK]
                   EXPORT    OTG_HS_WKUP_IRQHandler        [WEAK]
                   EXPORT    OTG_HS_IRQHandler             [WEAK]
                   EXPORT    DCMI_IRQHandler               [WEAK]
                   EXPORT    CRYP_IRQHandler               [WEAK]
                   EXPORT    HASH_RNG_IRQHandler           [WEAK]
                   EXPORT    FPU_IRQHandler                [WEAK]

WWDG_IRQHandler                 EQU       Default_Handler
PVD_IRQHandler                  EQU       Default_Handler
TAMP_STAMP_IRQHandler           EQU       Default_Handler
RTC_WKUP_IRQHandler             EQU       Default_Handler
FLASH_IRQHandler                EQU       Default_Handler
RCC_IRQHandler                  EQU       Default_Handler
EXTI0_IRQHandler                EQU       Default_Handler
EXTI1_IRQHandler                EQU       Default_Handler
EXTI2_IRQHandler                EQU       Default_Handler
EXTI3_IRQHandler                EQU       Default_Handler
EXTI4_IRQHandler                EQU       Default_Handler
DMA1_Stream0_IRQHandler         EQU       Default_Handler
DMA1_Stream1_IRQHandler         EQU       Default_Handler
DMA1_Stream2_IRQHandler         EQU       Default_Handler
DMA1_Stream3_IRQHandler         EQU       Default_Handler
DMA1_Stream4_IRQHandler         EQU       Default_Handler
DMA1_Stream5_IRQHandler         EQU       Default_Handler
DMA1_Stream6_IRQHandler         EQU       Default_Handler
ADC_IRQHandler                  EQU       Default_Handler
CAN1_TX_IRQHandler              EQU       Default_Handler
CAN1_RX0_IRQHandler             EQU       Default_Handler
CAN1_RX1_IRQHandler             EQU       Default_Handler
CAN1_SCE_IRQHandler             EQU       Default_Handler
EXTI9_5_IRQHandler              EQU       Default_Handler
TIM1_BRK_TIM9_IRQHandler        EQU       Default_Handler
TIM1_UP_TIM10_IRQHandler        EQU       Default_Handler
TIM1_TRG_COM_TIM11_IRQHandler   EQU       Default_Handler
TIM1_CC_IRQHandler              EQU       Default_Handler
TIM2_IRQHandler                 EQU       Default_Handler
TIM3_IRQHandler                 EQU       Default_Handler
TIM4_IRQHandler                 EQU       Default_Handler
I2C1_EV_IRQHandler              EQU       Default_Handler
I2C1_ER_IRQHandler              EQU       Default_Handler
I2C2_EV_IRQHandler              EQU       Default_Handler
I2C2_ER_IRQHandler              EQU       Default_Handler
SPI1_IRQHandler                 EQU       Default_Handler
SPI2_IRQHandler                 EQU       Default_Handler
USART1_IRQHandler               EQU       Default_Handler
USART2_IRQHandler               EQU       Default_Handler
USART3_IRQHandler               EQU       Default_Handler
EXTI15_10_IRQHandler            EQU       Default_Handler
RTC_Alarm_IRQHandler            EQU       Default_Handler
OTG_FS_WKUP_IRQHandler          EQU       Default_Handler
TIM8_BRK_TIM12_IRQHandler       EQU       Default_Handler
TIM8_UP_TIM13_IRQHandler        EQU       Default_Handler
TIM8_TRG_COM_TIM14_IRQHandler   EQU       Default_Handler
TIM8_CC_IRQHandler              EQU       Default_Handler
DMA1_Stream7_IRQHandler         EQU       Default_Handler
FSMC_IRQHandler                 EQU       Default_Handler
SDIO_IRQHandler                 EQU       Default_Handler
TIM5_IRQHandler                 EQU       Default_Handler
SPI3_IRQHandler                 EQU       Default_Handler
UART4_IRQHandler                EQU       Default_Handler
UART5_IRQHandler                EQU       Default_Handler
TIM6_DAC_IRQHandler             EQU       Default_Handler
TIM7_IRQHandler                 EQU       Default_Handler
DMA2_Stream0_IRQHandler         EQU       Default_Handler
DMA2_Stream1_IRQHandler         EQU       Default_Handler
DMA2_Stream2_IRQHandler         EQU       Default_Handler
DMA2_Stream3_IRQHandler         EQU       Default_Handler
DMA2_Stream4_IRQHandler         EQU       Default_Handler
ETH_IRQHandler                  EQU       Default_Handler
ETH_WKUP_IRQHandler             EQU       Default_Handler
CAN2_TX_IRQHandler              EQU       Default_Handler
CAN2_RX0_IRQHandler             EQU       Default_Handler
CAN2_RX1_IRQHandler             EQU       Default_Handler
CAN2_SCE_IRQHandler             EQU       Default_Handler
OTG_FS_IRQHandler               EQU       Default_Handler
DMA2_Stream5_IRQHandler         EQU       Default_Handler
DMA2_Stream6_IRQHandler         EQU       Default_Handler
DMA2_Stream7_IRQHandler         EQU       Default_Handler
USART6_IRQHandler               EQU       Default_Handler
I2C3_EV_IRQHandler              EQU       Default_Handler
I2C3_ER_IRQHandler              EQU       Default_Handler
OTG_HS_EP1_OUT_IRQHandler       EQU       Default_Handler
OTG_HS_EP1_IN_IRQHandler        EQU       Default_Handler
OTG_HS_WKUP_IRQHandler          EQU       Default_Handler
OTG_HS_IRQHandler               EQU       Default_Handler
DCMI_IRQHandler                 EQU       Default_Handler
CRYP_IRQHandler                 EQU       Default_Handler
HASH_RNG_IRQHandler             EQU       Default_Handler
FPU_IRQHandler                  EQU       Default_Handler

                   ALIGN


; ----======================================================================----
;     Stack and heap configuration
; ----======================================================================----

                   IMPORT    __use_two_region_memory
                   EXPORT    __user_initial_stackheap

; Returns the stack and heap address bounds
; TODO: Replace by __user_setup_stackheap()
__user_initial_stackheap
                   LDR       R0, =Heap_Memory
                   LDR       R1, =(Stack_Memory + Stack_Size)
                   LDR       R2, =(Heap_Memory + Heap_Size)
                   LDR       R3, =Stack_Memory
                   BX        LR

                   ALIGN

                   END
