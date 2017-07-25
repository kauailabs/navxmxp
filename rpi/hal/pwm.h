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

/* Allocates a VMX PWM "DigitalHandle" for the channel specified PortHandle.     */
/* Returns VMXHAL_kInvalidHandle upon error (and status != 0), otherwise         */
/* if successful returns the VMXHAL_DigitalHandle representing the PWM Resource. */
VMXHAL_DigitalHandle VMXHAL_InitializePWMPort(VMXHAL_PortHandle portHandle,
                                        int32_t* status);
/* Releases the previously-allocated PWM Port Handle.  If PWM is active on this  */
/* PWM, it is stopped.  status is 0 upon success, or non-zero in case of error.  */
void VMXHAL_FreePWMPort(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);

/* Returns !0 if the channel is a valid VMX Channel Number for PWM, 0 otherwise. */
VMXHAL_Bool VMXHAL_CheckPWMChannel(int32_t channel);

/* maxPwm:          The maximum scaled PWM value for forward direction. */
/* deadbandMaxPwm:  The minimum scaled PWM value for forward direction. */
/* centerPWM:       The scaled PWM value that represents idle.          */
/* deadbandMinPwm:  The minimum scaled PWM value for reverse direction. */
/* minPwm:          The maximum scaled PWM value for reverse direction. */
/* NOTE:  All values are "scaled", falling within a range of -1.0       */
/* (minPwm) to 1.0 (maxPwm).                                            */
void VMXHAL_SetPWMConfig(VMXHAL_DigitalHandle pwmPortHandle, double maxPwm,
                      double deadbandMaxPwm, double centerPwm,
                      double deadbandMinPwm, double minPwm, int32_t* status);

/* All values correspond to those used in VMXHAL_SetPWMConfig, except that */
/* these are "raw" values (0-255) rather than the scaled value.            */
void VMXHAL_SetPWMConfigRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t maxPwm,
                         int32_t deadbandMaxPwm, int32_t centerPwm,
                         int32_t deadbandMinPwm, int32_t minPwm,
                         int32_t* status);
void VMXHAL_GetPWMConfigRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t* maxPwm,
                         int32_t* deadbandMaxPwm, int32_t* centerPwm,
                         int32_t* deadbandMinPwm, int32_t* minPwm,
                         int32_t* status);
/* If eliminateDeadband == 0, the deadband min/max Pwm values are ignored. */
void VMXHAL_SetPWMEliminateDeadband(VMXHAL_DigitalHandle pwmPortHandle,
                                 VMXHAL_Bool eliminateDeadband, int32_t* status);
VMXHAL_Bool VMXHAL_GetPWMEliminateDeadband(VMXHAL_DigitalHandle pwmPortHandle,
                                     int32_t* status);

/* Sets a PWM channel to the desired value.  The values range from 0 to 255 and
 * the period is controlled by the PWM Period and MinHigh registers.
 */
void VMXHAL_SetPWMRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t value,
                   int32_t* status);
/* This is intended to be used by speed controllers and other standard PWM devices. */
/* Speed is a value from -1.0 to 1.0, representing the logical scale between */
/* the minPwm (-1.0) to the centerPwm (0) to the maxPwm (1.0). */
void VMXHAL_SetPWMSpeed(VMXHAL_DigitalHandle pwmPortHandle, double speed,
                     int32_t* status);
/* This is intended to be used by servos.  Rather than communicating a rate, the */
/* Position represents a displacement (typically 0 degrees) rather than a rate. */
void VMXHAL_SetPWMPosition(VMXHAL_DigitalHandle pwmPortHandle, double position,
                        int32_t* status);
void VMXHAL_SetPWMDisabled(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
int32_t VMXHAL_GetPWMRaw(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
double VMXHAL_GetPWMSpeed(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
/* Get a position value from a PWM channel.  The values range from 0 to 1. */
double VMXHAL_GetPWMPosition(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);
void VMXHAL_LatchPWMZero(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status);

/* Set how often the PWM signal is squelched, thus scaling the period. */
/* This is used to slow down the PWM Signal (5ms is the default period) */
/* for use with older devices (e.g., Victor, Talon) which cannot handle fast PWM rates. */
/* 3:  Squelch 3 out of 4 outputs */
/* 1:  Squelch 1 out of 2 outputs */
/* 0:  Don't squelch any outputs */
void VMXHAL_SetPWMPeriodScale(VMXHAL_DigitalHandle pwmPortHandle, int32_t squelchMask,
                           int32_t* status);
/* Get the loop timing of the PWM system. */
int32_t VMXHAL_GetLoopTiming(int32_t* status);

#ifdef __cplusplus
}
#endif

