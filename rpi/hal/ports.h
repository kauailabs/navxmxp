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

/* VMX HAL Port Quantities */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t VMXHAL_GetNumAccumulators(void);		/* VMX-only:  4 */
int32_t VMXHAL_GetNumAnalogTriggers(void);		/* VMX-only:  4 */
int32_t VMXHAL_GetNumAnalogInputs(void);		/* VMX-only:  4 */
int32_t VMXHAL_GetCounters(void);				/* VMX-only:  5 (6?) */
int32_t VMXHAL_GetNumDigitalHeaders(void);		/* 12 + 6 + 10 (opt) */
int32_t VMXHAL_GetNumPWMHeaders(void);			/* 10 (opt) */
int32_t VMXHAL_GetNumDigitalChannels(void);		/* 12 + 6 + 10 (opt) */
int32_t VMXHAL_GetNumPWMChannels(void);			/* 10 or 0 */
int32_t VMXHAL_GetNumDigitalPWMOutputs(void);	/* 12 + 6 + 10 (opt) */
int32_t VMXHAL_GetNumEncoders(void);			/* 4 */
int32_t VMXHAL_GetNumInterrupts(void);			/* 12 + 6 + 10 (opt) */

#ifdef __cplusplus
}
#endif

