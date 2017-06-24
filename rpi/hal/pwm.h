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

/* VMX HAL PWM Interface */

#pragma once

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_DigitalHandle VMXHAL_InitializePWMPort(VMXHAL_PortHandle portHandle,
                                        int32_t* status);
void VMXHAL_FreePWMPort(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);

VMXHAL_Bool VMXHAL_CheckPWMChannel(int32_t channel);

void VMXHAL_SetPWMConfig(VMXHAL_DigitalHandle pwmPortHandle, double maxPwm,
                      double deadbandMaxPwm, double centerPwm,
                      double deadbandMinPwm, double minPwm, int32_t* status);
void VMXHAL_SetPWMConfigRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t maxPwm,
                         int32_t deadbandMaxPwm, int32_t centerPwm,
                         int32_t deadbandMinPwm, int32_t minPwm,
                         int32_t* status);
void VMXHAL_GetPWMConfigRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t* maxPwm,
                         int32_t* deadbandMaxPwm, int32_t* centerPwm,
                         int32_t* deadbandMinPwm, int32_t* minPwm,
                         int32_t* status);
void VMXHAL_SetPWMEliminateDeadband(VMXHAL_DigitalHandle pwmPortHandle,
                                 VMXHAL_Bool eliminateDeadband, int32_t* status);
VMXHAL_Bool VMXHAL_GetPWMEliminateDeadband(VMXHAL_DigitalHandle pwmPortHandle,
                                     int32_t* status);
void VMXHAL_SetPWMRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t value,
                   int32_t* status);
void VMXHAL_SetPWMSpeed(VMXHAL_DigitalHandle pwmPortHandle, double speed,
                     int32_t* status);
void VMXHAL_SetPWMPosition(VMXHAL_DigitalHandle pwmPortHandle, double position,
                        int32_t* status);
void VMXHAL_SetPWMDisabled(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
int32_t VMXHAL_GetPWMRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
double VMXHAL_GetPWMSpeed(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
double VMXHAL_GetPWMPosition(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
void VMXHAL_LatchPWMZero(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
void VMXHAL_SetPWMPeriodScale(VMXHAL_DigitalHandle pwmPortHandle, int32_t squelchMask,
                           int32_t* status);
int32_t VMXHAL_GetLoopTiming(int32_t* status);

#ifdef __cplusplus
}
#endif

