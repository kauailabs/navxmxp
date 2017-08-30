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

/* VMX HAL Thread Interface */

#pragma once

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#define VMXThreadHandle const pthread_t*

int32_t VMXHAL_GetThreadPriority(VMXThreadHandle handle,
									VMXHAL_Bool* isRealTime,
									int32_t* status);

int32_t VMXHAL_GetCurrentThreadPriority(VMXHAL_Bool* isRealTime,
										int32_t* status);

VMXHAL_Bool VMXHAL_SetThreadPriority(VMXThreadHandle handle,
										VMXHAL_Bool realTime,
										int32_t priority,
										int32_t* status);

VMXHAL_Bool VMXHAL_SetCurrentThreadPriority(VMXHAL_Bool realTime,
											int32_t priority,
											int32_t* status);

#ifdef __cplusplus
}
#endif

