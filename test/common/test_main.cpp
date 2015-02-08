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

#include "stm32f4xx.h"
#include "system_stm32f4xx.h"

#include "cmsis_os.h"

#include "gtest/gtest.h"

#include <stdio.h>
#include <exception>

#define GREEN_LED    12
#define ORANGE_LED   13
#define RED_LED      14
#define BLUE_LED     15

void initUart()
{
    const int BAUDRATE = 115200;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    // Set the RX pin (PD09) to alternate function 7.
    GPIOD->PUPDR   &= ~((uint32_t)3 << (2 * 9));
    GPIOD->PUPDR   |=   (uint32_t)0 << (2 * 9);
    GPIOD->MODER   &= ~((uint32_t)3 << (2 * 9));
    GPIOD->MODER   |=   (uint32_t)2 << (2 * 9);
    GPIOD->AFR[1]  &= ~((uint32_t)0xF << (4 * (9 % 8)));
    GPIOD->AFR[1]  |=   (uint32_t)7   << (4 * (9 % 8));

    // Set the TX pin (PD08) to alternate function 7. The pull-up is enabled,
    // otherwise a wrong first byte is transmitted.
    GPIOD->PUPDR   &= ~((uint32_t)3 << (2 * 8));
    GPIOD->PUPDR   |=   (uint32_t)1 << (2 * 8);
    GPIOD->MODER   &= ~((uint32_t)3 << (2 * 8));
    GPIOD->MODER   |=   (uint32_t)2 << (2 * 8);
    GPIOD->OTYPER  &= ~((uint32_t)1 << 8);
    GPIOD->OSPEEDR &= ~((uint32_t)3 << (2 * 8));
    GPIOD->OSPEEDR |=   (uint32_t)2 << (2 * 8);
    GPIOD->AFR[1]  &= ~((uint32_t)0xF << (4 * (8 % 8)));
    GPIOD->AFR[1]  |=   (uint32_t)7   << (4 * (8 % 8));

    // Setup the USART.
    USART3->BRR = SYSTEM_SLOW_PERIPHERAL_CLOCK / BAUDRATE;
    USART3->CR1 = 0;
    USART3->CR2 = 0;
    USART3->CR3 = 0;
    USART3->CR1 |= USART_CR1_RE | USART_CR1_TE;
    USART3->CR1 |= USART_CR1_UE;

    printf("\n\nUART initialized\n");
}

extern "C" void putChar(char ch)
{
    while ((USART3->SR & USART_SR_TXE) == 0);
    USART3->DR = ch;
}

extern "C" void SystemInit(void)
{
    System_InitializeClock();

    //! \todo FPU settings should go into the peripheral description file.
    // Enable the FPU coprocessor.
    //SCB->CPACR = uint32_t(0xF) << 20;
    *((volatile uint32_t*)0xE000ED88) = uint32_t(0xF) << 20;
    // Trap on unaligned memory accesses.
    //SCB->CCR |= uint32_t(8);
    *((volatile uint32_t*)0xE000ED14) = uint32_t(8);

    initUart();
}

namespace weos
{
void throw_exception(const std::exception& e)
{
    printf("Exception: '%s'\n", e.what());
    while (1);
}
} // namespace weos

extern "C" void runTests(const void* arg)
{
    uint32_t led;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER   |= 1 << (2 * GREEN_LED);
    GPIOD->OSPEEDR |= 1 << (2 * GREEN_LED);
    GPIOD->MODER   |= 1 << (2 * ORANGE_LED);
    GPIOD->OSPEEDR |= 1 << (2 * ORANGE_LED);
    GPIOD->MODER   |= 1 << (2 * RED_LED);
    GPIOD->OSPEEDR |= 1 << (2 * RED_LED);
    GPIOD->MODER   |= 1 << (2 * BLUE_LED);
    GPIOD->OSPEEDR |= 1 << (2 * BLUE_LED);

    // Clear all LEDs.
    GPIOD->BRR = (1 << GREEN_LED);
    GPIOD->BRR = (1 << ORANGE_LED);
    GPIOD->BRR = (1 << RED_LED);
    GPIOD->BRR = (1 << BLUE_LED);

    // LED startup sequence.
    GPIOD->BSR = (1 << GREEN_LED);
    osDelay(125);
    GPIOD->BSR = (1 << ORANGE_LED);
    osDelay(125);
    GPIOD->BSR = (1 << RED_LED);
    osDelay(125);
    GPIOD->BSR = (1 << BLUE_LED);
    osDelay(125);
    GPIOD->BRR = (1 << GREEN_LED);
    GPIOD->BRR = (1 << ORANGE_LED);
    GPIOD->BRR = (1 << RED_LED);
    GPIOD->BRR = (1 << BLUE_LED);

    // Execute the tests.
    GPIOD->BSR = (1 << BLUE_LED);
    if (RUN_ALL_TESTS())
        led = (1 << GREEN_LED);
    else
        led = (1 << RED_LED);

    // Clear all LEDs.
    GPIOD->BRR = (1 << GREEN_LED);
    GPIOD->BRR = (1 << ORANGE_LED);
    GPIOD->BRR = (1 << RED_LED);
    GPIOD->BRR = (1 << BLUE_LED);

    // Blink either with the green or the red LED depending on the outcome.
    while (1)
    {
        GPIOD->BSR = led;
        osDelay(500);
        GPIOD->BRR = led;
        osDelay(500);
    }
}

void showError()
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER   |= 1 << (2 * GREEN_LED);
    GPIOD->OSPEEDR |= 1 << (2 * GREEN_LED);
    GPIOD->MODER   |= 1 << (2 * ORANGE_LED);
    GPIOD->OSPEEDR |= 1 << (2 * ORANGE_LED);
    GPIOD->MODER   |= 1 << (2 * RED_LED);
    GPIOD->OSPEEDR |= 1 << (2 * RED_LED);
    GPIOD->MODER   |= 1 << (2 * BLUE_LED);
    GPIOD->OSPEEDR |= 1 << (2 * BLUE_LED);
    for (int i=0; i<3; ++i)
    {
        GPIOD->BSR = 1 << RED_LED;
        osDelay(1000);
        GPIOD->BRR = 1 << RED_LED;
        osDelay(1000);
    }
    while (1);
}

int main()
{
    printf("main() started\n");

#if 0

    runTests(0);
    while (1);

#else

    osThreadDef_t testRunnerThread = {&runTests, osPriorityNormal, 1, 0};
    if (osKernelInitialize() != osOK
        || osKernelStart() != osOK
        || osThreadCreate(&testRunnerThread, 0) == NULL)
    {
        showError();
    }

    while (1)
    {
        osDelay(10000);
    }

#endif
}

// This handler is invoked when a pure virtual method is called. The default
// implementation outputs useful debug information which unfortunately pulls
// in a lot of code (e.g. fputs).
void __cxa_pure_virtual()
{
    while (1);
}

// Provide a new terminate handler which is called when an uncaught exception
// is encountered.
namespace __gnu_cxx
{
void __verbose_terminate_handler()
{
    while (1);
}
} // namespace __gnu_cxx
