/* ============================================
VMX-pi HAL source code is placed under the MIT license
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

#ifndef VMXHANDLERS_H_
#define VMXHANDLERS_H_

#include <stdint.h>

typedef enum {
	FALLING_EDGE_INTERRUPT = 0,
	RISING_EDGE_INTERRUPT = 1,
} InterruptEdgeType;

typedef void (*VMXIO_InterruptHandler)(uint32_t vmx_channel_index, InterruptEdgeType edge, void* param, uint64_t timestamp_us);

/* Handler invoked when a registered notification has been triggered */
typedef void (*VMXNotifyHandler)(void *param, uint64_t timestamp_us);

typedef VMXNotifyHandler CAN_NewRxDataNotifyHandler;

typedef VMXNotifyHandler AHRS_NewRxDataNotifyHandler;

#endif /* VMXHANDLERS_H_ */
