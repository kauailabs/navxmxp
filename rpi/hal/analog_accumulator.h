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

/* VMX HAL Analog Accumulator (Oversample/Average) Interface */

#pragma once

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_Bool VMXHAL_IsAccumulatorChannel(VMXHAL_AnalogInputHandle analogPortHandle,
                                  int32_t* status);

/* Todo: Add interface for setting initial value? */

void VMXHAL_InitAccumulator(VMXHAL_AnalogInputHandle analogPortHandle,
                         int32_t* status);

void VMXHAL_ResetAccumulator(VMXHAL_AnalogInputHandle analogPortHandle,
                          int32_t* status);

/* The center value is subtracted from each sample before the sample is applied
 * to the accumulator. Note that the accumulator is after the oversample and
 * averaging engine in the pipeline so oversampling will affect the appropriate
 * value for this parameter.
 **/

void VMXHAL_SetAccumulatorCenter(VMXHAL_AnalogInputHandle analogPortHandle,
                              int32_t center, int32_t* status);

/* The raw value deadband around the center point where the accumulator will
 * treat the sample as 0.
 **/

void VMXHAL_SetAccumulatorDeadband(VMXHAL_AnalogInputHandle analogPortHandle,
                                int32_t deadband, int32_t* status);

int64_t VMXHAL_GetAccumulatorValue(VMXHAL_AnalogInputHandle analogPortHandle,
                                int32_t* status);

/* The number of samples that have been added to the accumulator since the
 * last reset.
 **/

int64_t VMXHAL_GetAccumulatorCount(VMXHAL_AnalogInputHandle analogPortHandle,
                                int32_t* status);

void VMXHAL_GetAccumulatorOutput(VMXHAL_AnalogInputHandle analogPortHandle,
                              int64_t* value, int64_t* count, int32_t* status);

#ifdef __cplusplus
}
#endif

