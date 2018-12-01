/*
 * Simple microseconds delay routine, utilizingARM's DWT
 * (Data Watchpoint and Trace Unit) and HAL library.
 */

#include "stm32f4xx_hal.h"          // change to whatever MCU you use
#include "DWTDelay.h"

void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * Delay routine itself.
 * Time is in microseconds (1/1000000th of a second), not to be
 * confused with millisecond (1/1000th).
 *
 * @param uint32_t us  Number of microseconds to delay for
 */
void DWT_Delay(uint32_t us) // microseconds
{
  int32_t targetTick = DWT->CYCCNT + us * (SystemCoreClock/1000000);
  while (DWT->CYCCNT <= targetTick);
}
