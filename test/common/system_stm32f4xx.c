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

#include "stm32f4xx.h"
#include "system_stm32f4xx.h"

// The APB1 divider must be one of {1, 2, 4, 8, 16}.
#if SYSTEM_SLOW_PERIPHERAL_DIVIDER < 1 || SYSTEM_SLOW_PERIPHERAL_DIVIDER > 16  \
    || (SYSTEM_SLOW_PERIPHERAL_DIVIDER & (SYSTEM_SLOW_PERIPHERAL_DIVIDER - 1)) != 0
#  error Wrong SYSTEM_SLOW_PERIPHERAL_DIVIDER
#endif

// The APB2 divider must be one of {1, 2, 4, 8, 16}.
#if SYSTEM_FAST_PERIPHERAL_DIVIDER < 1 || SYSTEM_FAST_PERIPHERAL_DIVIDER > 16  \
    || (SYSTEM_FAST_PERIPHERAL_DIVIDER & (SYSTEM_FAST_PERIPHERAL_DIVIDER - 1)) != 0
#  error Wrong SYSTEM_FAST_PERIPHERAL_DIVIDER
#endif

// f_VCO = f_HSE / M * N
// f_SYSCLK = f_VCO / P
// f_USB = f_SDIO = f_RNG = f_VCO / Q
//
// Constraints:
// 1 MHz <= f_HSE / M <= 2 MHz (2 MHz is preferable to reduce jitter)
// 64 MHz <= f_VCO <= 432 MHz
// f_USB = 48 MHz
// 2 <= M <= 63
// 63 <= N <= 432
// P in {2, 4, 6, 8}
// 2 <= Q <= 15
//
// f_VCO = lcm(f_USB, f_SYSCLK) = lcm(48 MHz, f_SYSCLK)

#if (SYSTEM_EXTERNAL_CLOCK_FREQUENCY / 2000000) * 2000000 == SYSTEM_EXTERNAL_CLOCK_FREQUENCY
#  define VCO_INPUT_FREQUENCY   2000000
#else
#  if (SYSTEM_EXTERNAL_CLOCK_FREQUENCY / 1000000) * 1000000 != SYSTEM_EXTERNAL_CLOCK_FREQUENCY
#    error SYSTEM_EXTERNAL_CLOCK_FREQUENCY is not a multiple of 1 MHz.
#  endif
#  define VCO_INPUT_FREQUENCY   1000000
#endif
#define PLL_M   (SYSTEM_EXTERNAL_CLOCK_FREQUENCY / VCO_INPUT_FREQUENCY)

#if SYSTEM_CLOCK_FREQUENCY == 168000000
// f_VCO = 336 MHz
#  define PLL_N   (336000000 * PLL_M / SYSTEM_EXTERNAL_CLOCK_FREQUENCY)
#  define PLL_P   2
#  define PLL_Q   7
#else
#  error This SYSTEM_CLOCK_FREQUENCY is not supported.
#endif

// Sanity checks for the PLL configuration.
#if PLL_M < 2 || PLL_M > 63
#  error Wrong PLL_M.
#endif
#if PLL_N < 63 || PLL_N > 432
#  error Wrong PLL_N.
#endif
#if PLL_P != 2 && PLL_P != 4 && PLL_P != 6 && PLL_P != 8
#  error Wrong PLL_P.
#endif
#if PLL_Q < 2 || PLL_Q > 15
#  error Wrong PLL_Q.
#endif

void System_InitializeClock(void)
{
    uint32_t hseReadyCounter = 0;

    // Enable the high-speed internal (HSI) clock and wait until it is stable.
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Switch to the internal clock.
    RCC->CFGR = 0;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    // Disable all clocks except the HSI clock.
    RCC->CR = 0x00000081;
    RCC->CIR = 0;

    // Enable the high-speed external (HSE) clock and wait until it is stable.
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)
           && hseReadyCounter < SYSTEM_HSE_READY_TIMEOUT)
    {
        ++hseReadyCounter;
    }

    if (!(RCC->CR & RCC_CR_HSERDY))
    {
        // The HSE has not stabilized within the timeout.
        while(1); //! \todo Throw exception.
    }

    // AHB clock frequencies above 144 MHz require the voltage output scaling
    // mode 1.
    if (SYSTEM_CLOCK_FREQUENCY > 144000000)
    {
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_VOS;
    }

    // Set the dividers for the AHB busses.
    switch (SYSTEM_SLOW_PERIPHERAL_DIVIDER)
    {
        case  1: RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;  break;
        case  2: RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;  break;
        case  4: RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;  break;
        case  8: RCC->CFGR |= RCC_CFGR_PPRE1_DIV8;  break;
        case 16: RCC->CFGR |= RCC_CFGR_PPRE1_DIV16; break;
    }
    switch (SYSTEM_FAST_PERIPHERAL_DIVIDER)
    {
        case  1: RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  break;
        case  2: RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;  break;
        case  4: RCC->CFGR |= RCC_CFGR_PPRE2_DIV4;  break;
        case  8: RCC->CFGR |= RCC_CFGR_PPRE2_DIV8;  break;
        case 16: RCC->CFGR |= RCC_CFGR_PPRE2_DIV16; break;
    }

    // Configure the PLL.
    RCC->PLLCFGR =   (uint32_t)PLL_M
                   | ((uint32_t)PLL_N << 6)
                   | (((uint32_t)PLL_P / 2 - 1) << 16)
                   | RCC_PLLCFGR_PLLSRC_HSE
                   | ((uint32_t)PLL_Q << 24);
    // Enable the main PLL and wait until it is locked.
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Configure the number of waitstates of the FLASH.
    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | SYSTEM_FLASH_WAITSTATES;

    // Switch the system clock to the PLL and wait until it is ready.
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
