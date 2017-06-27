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

/* VMX HAL Power Interface */

#pragma once

#include <stdint.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

double VMXHAL_GetVinVoltage(int32_t* status);
double VMXHAL_GetUserVoltage5V(int32_t* status);
VMXHAL_Bool VMXHAL_GetUserActive5V(int32_t* status);
int32_t VMXHAL_GetUserCurrentFaults5V(int32_t* status);
double VMXHAL_GetUserVoltage3V3(int32_t* status);
VMXHAL_Bool VMXHAL_GetUserActive3V3(int32_t* status);
int32_t VMXHAL_GetUserCurrentFaults3V3(int32_t* status);
#ifdef __cplusplus
}
#endif
