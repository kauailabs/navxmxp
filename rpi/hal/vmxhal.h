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

/* VMX HAL Top-level Header */

#pragma once

#include "analog_accumulator.h"
#include "analog_input.h"
#include "analog_trigger.h"
#include "constants.h"
#include "counter.h"
#include "dio.h"
#include "errors.h"
#include "i2c.h"
#include "interrupts.h"
#include "notifier.h"
#include "pwm.h"
#include "ports.h"
#include "power.h"
#include "spi.h"
#include "serial_port.h"

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* VMXHAL_GetErrorMessage(int32_t code);

int32_t VMXHAL_GetFirmwareVersion(int32_t* status);
int64_t VMXHAL_GetFirmwareRevision(int32_t* status);

typedef enum { VMXHAL_VMX, VMXHAL_Mock } VMXHAL_RuntimeType;

VMXHAL_RuntimeType VMXHAL_GetRuntimeType();

VMXHAL_Bool VMXHAL_GetSystemActive(int32_t* status);
VMXHAL_Bool VMXHAL_GetBrownedOut(int32_t* status);

void VMXHAL_BaseInitialize(int32_t* status);

uint64_t VMXHAL_GetMicrocontrollerTime(int32_t* status); /* Todo:  Granularity? */

int32_t VMXHAL_Initialize(int32_t mode);

int32_t VMXHAL_Deinitialize();

VMXHAL_PortHandle VMXHAL_GetPort(int32_t channel);
VMXHAL_PortHandle VMXHAL_GetPortWithModule(int32_t module, int32_t channel);

// ifdef's definition is to allow for default parameters in C++.
#ifdef __cplusplus
int64_t VMXHAL_Report(int32_t resource, int32_t instanceNumber,
                   int32_t context = 0, const char* feature = nullptr);
#else
int64_t VMXHAL_Report(int32_t resource, int32_t instanceNumber, int32_t context,
                   const char* feature);
#endif

#ifdef __cplusplus
}
#endif

