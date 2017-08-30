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

/* VMX HAL Quadrature Encoder Interface */

#pragma once

#include <stdint.h>
#include <types.h>

enum VMXHAL_EncoderIndexingType : int32_t {
  VMXHAL_kResetWhileHigh,
  VMXHAL_kResetWhileLow,
  VMXHAL_kResetOnFallingEdge,
  VMXHAL_kResetOnRisingEdge
};

enum VMXHAL_EncoderEncodingType : int32_t {
  VMXHAL_Encoder_k1X,
  VMXHAL_Encoder_k2X,
  VMXHAL_Encoder_k4X
};

#ifdef __cplusplus
extern "C" {
#endif

VMXHAL_EncoderHandle VMXHAL_InitializeEncoder(
    VMXHAL_Handle digitalSourceHandleA, VMXHAL_AnalogTriggerType analogTriggerTypeA,
    VMXHAL_Handle digitalSourceHandleB, VMXHAL_AnalogTriggerType analogTriggerTypeB,
    VMXHAL_Bool reverseDirection, VMXHAL_EncoderEncodingType encodingType,
    int32_t* status);
void VMXHAL_FreeEncoder(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
int32_t VMXHAL_GetEncoder(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
int32_t VMXHAL_GetEncoderRaw(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
int32_t VMXHAL_GetEncoderEncodingScale(VMXHAL_EncoderHandle encoderHandle,
                                    int32_t* status);
void VMXHAL_ResetEncoder(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
double VMXHAL_GetEncoderPeriod(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
void VMXHAL_SetEncoderMaxPeriod(VMXHAL_EncoderHandle encoderHandle, double maxPeriod,
                             int32_t* status);
VMXHAL_Bool VMXHAL_GetEncoderStopped(VMXHAL_EncoderHandle encoderHandle,
                               int32_t* status);
VMXHAL_Bool VMXHAL_GetEncoderDirection(VMXHAL_EncoderHandle encoderHandle,
                                 int32_t* status);
double VMXHAL_GetEncoderDistance(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
double VMXHAL_GetEncoderRate(VMXHAL_EncoderHandle encoderHandle, int32_t* status);
void VMXHAL_SetEncoderMinRate(VMXHAL_EncoderHandle encoderHandle, double minRate,
                           int32_t* status);
void VMXHAL_SetEncoderDistancePerPulse(VMXHAL_EncoderHandle encoderHandle,
                                    double distancePerPulse, int32_t* status);
void VMXHAL_SetEncoderReverseDirection(VMXHAL_EncoderHandle encoderHandle,
                                    VMXHAL_Bool reverseDirection, int32_t* status);
void VMXHAL_SetEncoderSamplesToAverage(VMXHAL_EncoderHandle encoderHandle,
                                    int32_t samplesToAverage, int32_t* status);
int32_t VMXHAL_GetEncoderSamplesToAverage(VMXHAL_EncoderHandle encoderHandle,
                                       int32_t* status);

void VMXHAL_SetEncoderIndexSource(VMXHAL_EncoderHandle encoderHandle,
                               VMXHAL_Handle digitalSourceHandle,
                               VMXHAL_AnalogTriggerType analogTriggerType,
                               VMXHAL_EncoderIndexingType type, int32_t* status);

int32_t VMXHAL_GetEncoderFPGAIndex(VMXHAL_EncoderHandle encoderHandle,
                                int32_t* status);

double VMXHAL_GetEncoderDecodingScaleFactor(VMXHAL_EncoderHandle encoderHandle,
                                         int32_t* status);

double VMXHAL_GetEncoderDistancePerPulse(VMXHAL_EncoderHandle encoderHandle,
                                      int32_t* status);

VMXHAL_EncoderEncodingType VMXHAL_GetEncoderEncodingType(
    VMXHAL_EncoderHandle encoderHandle, int32_t* status);

#ifdef __cplusplus
}
#endif

