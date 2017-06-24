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

/* VMX HAL Data Types */

#pragma once

#include <stdint.h>

#define VMXHAL_kInvalidHandle 0

typedef int32_t VMXHAL_Handle;

typedef VMXHAL_Handle VMXHAL_PortHandle;

typedef VMXHAL_Handle VMXHAL_AnalogInputHandle;

typedef VMXHAL_Handle VMXHAL_AnalogTriggerHandle;

typedef VMXHAL_Handle VMXHAL_CounterHandle;

typedef VMXHAL_Handle VMXHAL_DigitalHandle;

typedef VMXHAL_Handle VMXHAL_DigitalPWMHandle;

typedef VMXHAL_Handle VMXHAL_EncoderHandle;

typedef VMXHAL_Handle VMXHAL_InterruptHandle;

typedef VMXHAL_Handle VMXHAL_NotifierHandle;

typedef int32_t VMXHAL_Bool;

/* Note:  the following singular objects do not have a handle:
 *
 * CAN
 * IMU
 * I2C
 * SerialPort
 * SPI
 */
