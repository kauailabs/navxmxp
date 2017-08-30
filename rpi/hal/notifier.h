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

/* VMX HAL Notifier Interface */

#pragma once

#include <stdint.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Note that currentTime is in units of Microseconds, absolute time based on the current system clock. */
typedef void (*VMXHAL_NotifierProcessFunction)(uint64_t currentTime,
                                            VMXHAL_NotifierHandle handle);

VMXHAL_NotifierHandle VMXHAL_InitializeNotifier(VMXHAL_NotifierProcessFunction process,
                                          void* param, int32_t* status);
VMXHAL_NotifierHandle VMXHAL_InitializeNotifierThreaded(
    VMXHAL_NotifierProcessFunction process, void* param, int32_t* status);
void VMXHAL_CleanNotifier(VMXHAL_NotifierHandle notifierHandle, int32_t* status);
void* VMXHAL_GetNotifierParam(VMXHAL_NotifierHandle notifierHandle, int32_t* status);
void VMXHAL_UpdateNotifierAlarm(VMXHAL_NotifierHandle notifierHandle,
                             uint64_t triggerTime, int32_t* status);
void VMXHAL_StopNotifierAlarm(VMXHAL_NotifierHandle notifierHandle, int32_t* status);
#ifdef __cplusplus
}
#endif
