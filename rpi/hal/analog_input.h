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

/* VMX HAL Analog Input Interface */

#pragma once

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
VMXHAL_AnalogInputHandle VMXHAL_InitializeAnalogInputPort(VMXHAL_PortHandle portHandle,
                                                    int32_t* status);
void VMXHAL_FreeAnalogInputPort(VMXHAL_AnalogInputHandle analogPortHandle);
VMXHAL_Bool VMXHAL_CheckAnalogModule(int32_t module);
VMXHAL_Bool VMXHAL_CheckAnalogInputChannel(int32_t channel);

void VMXHAL_SetAnalogSampleRate(double samplesPerSecond, int32_t* status);
double VMXHAL_GetAnalogSampleRate(int32_t* status);
void VMXHAL_SetAnalogAverageBits(VMXHAL_AnalogInputHandle analogPortHandle,
                              int32_t bits, int32_t* status);
int32_t VMXHAL_GetAnalogAverageBits(VMXHAL_AnalogInputHandle analogPortHandle,
                                 int32_t* status);
void VMXHAL_SetAnalogOversampleBits(VMXHAL_AnalogInputHandle analogPortHandle,
                                 int32_t bits, int32_t* status);
int32_t VMXHAL_GetAnalogOversampleBits(VMXHAL_AnalogInputHandle analogPortHandle,
                                    int32_t* status);
int32_t VMXHAL_GetAnalogValue(VMXHAL_AnalogInputHandle analogPortHandle,
                           int32_t* status);
int32_t VMXHAL_GetAnalogAverageValue(VMXHAL_AnalogInputHandle analogPortHandle,
                                  int32_t* status);
int32_t VMXHAL_GetAnalogVoltsToValue(VMXHAL_AnalogInputHandle analogPortHandle,
                                  double voltage, int32_t* status);
double VMXHAL_GetAnalogVoltage(VMXHAL_AnalogInputHandle analogPortHandle,
                            int32_t* status);
double VMXHAL_GetAnalogAverageVoltage(VMXHAL_AnalogInputHandle analogPortHandle,
                                   int32_t* status);
int32_t VMXHAL_GetAnalogLSBWeight(VMXHAL_AnalogInputHandle analogPortHandle,
                               int32_t* status);
int32_t VMXHAL_GetAnalogOffset(VMXHAL_AnalogInputHandle analogPortHandle,
                            int32_t* status);

#ifdef __cplusplus
}
#endif

