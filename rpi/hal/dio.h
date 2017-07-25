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

/* VMX HAL Digital Input/Output (DIO) Interface */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_DigitalHandle VMXHAL_InitializeDIOPort(VMXHAL_PortHandle portHandle,
                                        VMXHAL_Bool input, int32_t* status);

VMXHAL_Bool VMXHAL_CheckDIOChannel(int32_t channel);

void VMXHAL_FreeDIOPort(VMXHAL_DigitalHandle dioPortHandle);

VMXHAL_DigitalPWMHandle VMXHAL_AllocateDigitalPWM(int32_t* status);

void VMXHAL_FreeDigitalPWM(VMXHAL_DigitalPWMHandle pwmGenerator, int32_t* status);

void VMXHAL_SetDigitalPWMRate(double rate, int32_t* status);

void VMXHAL_SetDigitalPWMDutyCycle(VMXHAL_DigitalPWMHandle pwmGenerator,
                                double dutyCycle, int32_t* status);

void VMXHAL_SetDigitalPWMOutputChannel(VMXHAL_DigitalPWMHandle pwmGenerator,
                                    int32_t channel, int32_t* status);

void VMXHAL_SetDIO(VMXHAL_DigitalHandle dioPortHandle, VMXHAL_Bool value,
                int32_t* status);

VMXHAL_Bool VMXHAL_GetDIO(VMXHAL_DigitalHandle dioPortHandle, int32_t* status);

VMXHAL_Bool VMXHAL_GetDIODirection(VMXHAL_DigitalHandle dioPortHandle, int32_t* status);

void VMXHAL_Pulse(VMXHAL_DigitalHandle dioPortHandle, double pulseLength,
               int32_t* status);

VMXHAL_Bool VMXHAL_IsPulsing(VMXHAL_DigitalHandle dioPortHandle, int32_t* status);

VMXHAL_Bool VMXHAL_IsAnyPulsing(int32_t* status);

void VMXHAL_SetFilterSelect(VMXHAL_DigitalHandle dioPortHandle, int32_t filterIndex,
                         int32_t* status);

int32_t VMXHAL_GetFilterSelect(VMXHAL_DigitalHandle dioPortHandle, int32_t* status);

void VMXHAL_SetFilterPeriod(int32_t filterIndex, int64_t value, int32_t* status);

int64_t VMXHAL_GetFilterPeriod(int32_t filterIndex, int32_t* status);

#ifdef __cplusplus
}
#endif

