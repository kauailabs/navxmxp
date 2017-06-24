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

/* VMX HAL Counter Interface */

#pragma once

#include <stdint.h>

enum VMXHAL_Counter_Mode : int32_t {
  VMXHAL_Counter_kTwoPulse = 0,
  VMXHAL_Counter_kSemiperiod = 1,
  VMXHAL_Counter_kPulseLength = 2,
  VMXHAL_Counter_kExternalDirection = 3
};

/* Notes:
 * VMX has 6 hardware counters:
 * - 4 full-capability counters (all modes), indexes 0-3
 * - 2 semi-capabiility counters (Semiperiod/External Direction), indexes 4-5
 */

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_CounterHandle VMXHAL_InitializeCounter(VMXHAL_Counter_Mode mode, int32_t* index,
                                        int32_t* status);

void VMXHAL_FreeCounter(VMXHAL_CounterHandle counterHandle, int32_t* status);

void VMXHAL_SetCounterAverageSize(VMXHAL_CounterHandle counterHandle, int32_t size,
                               int32_t* status);

void VMXHAL_SetCounterUpSource(VMXHAL_CounterHandle counterHandle,
                            VMXHAL_Handle digitalSourceHandle,
                            VMXHAL_AnalogTriggerType analogTriggerType,
                            int32_t* status);

void VMXHAL_SetCounterUpSourceEdge(VMXHAL_CounterHandle counterHandle,
                                VMXHAL_Bool risingEdge, VMXHAL_Bool fallingEdge,
                                int32_t* status);

void VMXHAL_ClearCounterUpSource(VMXHAL_CounterHandle counterHandle, int32_t* status);

void VMXHAL_SetCounterDownSource(VMXHAL_CounterHandle counterHandle,
                              VMXHAL_Handle digitalSourceHandle,
                              VMXHAL_AnalogTriggerType analogTriggerType,
                              int32_t* status);

void VMXHAL_SetCounterDownSourceEdge(VMXHAL_CounterHandle counterHandle,
                                  VMXHAL_Bool risingEdge, VMXHAL_Bool fallingEdge,
                                  int32_t* status);

void VMXHAL_ClearCounterDownSource(VMXHAL_CounterHandle counterHandle,
                                int32_t* status);

void VMXHAL_SetCounterUpDownMode(VMXHAL_CounterHandle counterHandle, int32_t* status);

void VMXHAL_SetCounterExternalDirectionMode(VMXHAL_CounterHandle counterHandle,
                                         int32_t* status);

void VMXHAL_SetCounterSemiPeriodMode(VMXHAL_CounterHandle counterHandle,
                                  VMXHAL_Bool highSemiPeriod, int32_t* status);

void VMXHAL_SetCounterPulseLengthMode(VMXHAL_CounterHandle counterHandle,
                                   double threshold, int32_t* status);

int32_t VMXHAL_GetCounterSamplesToAverage(VMXHAL_CounterHandle counterHandle,
                                       int32_t* status);

void VMXHAL_SetCounterSamplesToAverage(VMXHAL_CounterHandle counterHandle,
                                    int32_t samplesToAverage, int32_t* status);

void VMXHAL_ResetCounter(VMXHAL_CounterHandle counterHandle, int32_t* status);

int32_t VMXHAL_GetCounter(VMXHAL_CounterHandle counterHandle, int32_t* status);

double VMXHAL_GetCounterPeriod(VMXHAL_CounterHandle counterHandle, int32_t* status);

void VMXHAL_SetCounterMaxPeriod(VMXHAL_CounterHandle counterHandle, double maxPeriod,
                             int32_t* status);

void VMXHAL_SetCounterUpdateWhenEmpty(VMXHAL_CounterHandle counterHandle,
                                   VMXHAL_Bool enabled, int32_t* status);

VMXHAL_Bool VMXHAL_GetCounterStopped(VMXHAL_CounterHandle counterHandle,
                               int32_t* status);

VMXHAL_Bool VMXHAL_GetCounterDirection(VMXHAL_CounterHandle counterHandle,
                                 int32_t* status);

void VMXHAL_SetCounterReverseDirection(VMXHAL_CounterHandle counterHandle,
                                    VMXHAL_Bool reverseDirection, int32_t* status);

#ifdef __cplusplus
}
#endif

