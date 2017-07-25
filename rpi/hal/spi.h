/* ============================================
VMX HAL source code is placed under the MIT license
Copyright (c) 2017 Kauai Labs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
 */

/* VMX HAL SPI Port Interface */

#pragma once

#include "types.h"

typedef enum {
  VMXHAL_SPI_kRpi = 0,
} VMXHAL_SPIPort;

#ifdef __cplusplus
extern "C" {
#endif

/* SPI Port */

void VMXHAL_InitializeSPI(VMXHAL_SPIPort port, int32_t* status);
int32_t VMXHAL_TransactionSPI(VMXHAL_SPIPort port, uint8_t* dataToSend,
                           uint8_t* dataReceived, int32_t size);
int32_t VMXHAL_WriteSPI(VMXHAL_SPIPort port, uint8_t* dataToSend, int32_t sendSize);
int32_t VMXHAL_ReadSPI(VMXHAL_SPIPort port, uint8_t* buffer, int32_t count);
void VMXHAL_CloseSPI(VMXHAL_SPIPort port);
void VMXHAL_SetSPISpeed(VMXHAL_SPIPort port, int32_t speed);
void VMXHAL_SetSPIOpts(VMXHAL_SPIPort port, VMXHAL_Bool msbFirst,
                    VMXHAL_Bool sampleOnTrailing, VMXHAL_Bool clkIdleHigh);
void VMXHAL_SetSPIChipSelectActiveHigh(VMXHAL_SPIPort port, int32_t* status);
void VMXHAL_SetSPIChipSelectActiveLow(VMXHAL_SPIPort port, int32_t* status);
int32_t VMXHAL_GetSPIHandle(VMXHAL_SPIPort port);
void VMXHAL_SetSPIHandle(VMXHAL_SPIPort port, int32_t handle);

/* SPI Accumulator */

void VMXHAL_InitSPIAccumulator(VMXHAL_SPIPort port, int32_t period, int32_t cmd,
                            int32_t xferSize, int32_t validMask,
                            int32_t validValue, int32_t dataShift,
                            int32_t dataSize, VMXHAL_Bool isSigned,
                            VMXHAL_Bool bigEndian, int32_t* status);
void VMXHAL_FreeSPIAccumulator(VMXHAL_SPIPort port, int32_t* status);
void VMXHAL_ResetSPIAccumulator(VMXHAL_SPIPort port, int32_t* status);
void VMXHAL_SetSPIAccumulatorCenter(VMXHAL_SPIPort port, int32_t center,
                                 int32_t* status);
void VMXHAL_SetSPIAccumulatorDeadband(VMXHAL_SPIPort port, int32_t deadband,
                                   int32_t* status);
int32_t VMXHAL_GetSPIAccumulatorLastValue(VMXHAL_SPIPort port, int32_t* status);
int64_t VMXHAL_GetSPIAccumulatorValue(VMXHAL_SPIPort port, int32_t* status);
int64_t VMXHAL_GetSPIAccumulatorCount(VMXHAL_SPIPort port, int32_t* status);
double VMXHAL_GetSPIAccumulatorAverage(VMXHAL_SPIPort port, int32_t* status);
void VMXHAL_GetSPIAccumulatorOutput(VMXHAL_SPIPort port, int64_t* value,
                                 int64_t* count, int32_t* status);

#ifdef __cplusplus
}
#endif

