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

/* VMX HAL Analog Trigger Interface */

#pragma once

#include <stdint.h>
#include "types.h"

typedef enum {
  VMXHAL_Trigger_kInWindow = 0,
  VMXHAL_Trigger_kState = 1,
  VMXHAL_Trigger_kRisingPulse = 2,
  VMXHAL_Trigger_kFallingPulse = 3
} VMXHAL_AnalogTriggerType;

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_AnalogTriggerHandle VMXHAL_InitializeAnalogTrigger(
    VMXHAL_AnalogInputHandle portHandle, int32_t* index, int32_t* status);

void VMXHAL_CleanAnalogTrigger(VMXHAL_AnalogTriggerHandle analogTriggerHandle,
                            int32_t* status);

void VMXHAL_SetAnalogTriggerLimitsRaw(VMXHAL_AnalogTriggerHandle analogTriggerHandle,
                                   int32_t lower, int32_t upper,
                                   int32_t* status);

void VMXHAL_SetAnalogTriggerLimitsVoltage(
    VMXHAL_AnalogTriggerHandle analogTriggerHandle, double lower, double upper,
    int32_t* status);

void VMXHAL_SetAnalogTriggerAveraged(VMXHAL_AnalogTriggerHandle analogTriggerHandle,
                                  VMXHAL_Bool useAveragedValue, int32_t* status);

void VMXHAL_SetAnalogTriggerFiltered(VMXHAL_AnalogTriggerHandle analogTriggerHandle,
                                  VMXHAL_Bool useFilteredValue, int32_t* status);

VMXHAL_Bool VMXHAL_GetAnalogTriggerInWindow(
    VMXHAL_AnalogTriggerHandle analogTriggerHandle, int32_t* status);

VMXHAL_Bool VMXHAL_GetAnalogTriggerTriggerState(
    VMXHAL_AnalogTriggerHandle analogTriggerHandle, int32_t* status);

VMXHAL_Bool VMXHAL_GetAnalogTriggerOutput(VMXHAL_AnalogTriggerHandle analogTriggerHandle,
                                    VMXHAL_AnalogTriggerType type,
                                    int32_t* status);

#ifdef __cplusplus
}
#endif

