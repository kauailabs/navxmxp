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

/* VMX HAL Interrupt Interface */

#pragma once

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VMXHAL_InterruptHandlerFunction)(uint32_t interruptAssertedMask,
                                             void* param);


VMXHAL_InterruptHandle VMXHAL_InitializeInterrupts(VMXHAL_Bool watcher, int32_t* status);

void VMXHAL_CleanInterrupts(VMXHAL_InterruptHandle interruptHandle, int32_t* status);

int64_t VMXHAL_WaitForInterrupt(VMXHAL_InterruptHandle interruptHandle,
                             double timeout, VMXHAL_Bool ignorePrevious,
                             int32_t* status);

void VMXHAL_EnableInterrupts(VMXHAL_InterruptHandle interruptHandle, int32_t* status);

void VMXHAL_DisableInterrupts(VMXHAL_InterruptHandle interruptHandle,
                           int32_t* status);

double VMXHAL_ReadInterruptRisingTimestamp(VMXHAL_InterruptHandle interruptHandle,
                                        int32_t* status);

double VMXHAL_ReadInterruptFallingTimestamp(VMXHAL_InterruptHandle interruptHandle,
                                         int32_t* status);

void VMXHAL_RequestInterrupts(VMXHAL_InterruptHandle interruptHandle,
                           VMXHAL_Handle digitalSourceHandle,
                           VMXHAL_AnalogTriggerType analogTriggerType,
                           int32_t* status);

void VMXHAL_AttachInterruptHandler(VMXHAL_InterruptHandle interruptHandle,
                                VMXHAL_InterruptHandlerFunction handler,
                                void* param, int32_t* status);

void VMXHAL_AttachInterruptHandlerThreaded(VMXHAL_InterruptHandle interruptHandle,
                                        VMXHAL_InterruptHandlerFunction handler,
                                        void* param, int32_t* status);

void VMXHAL_SetInterruptUpSourceEdge(VMXHAL_InterruptHandle interruptHandle,
                                  VMXHAL_Bool risingEdge, HAL_Bool fallingEdge,
                                  int32_t* status);

#ifdef __cplusplus
}
#endif

