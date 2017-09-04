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

#ifndef VMXRESOURCE_H_
#define VMXRESOURCE_H_

#include <stdint.h>

#include "VMXChannel.h"
#include "VMXErrors.h"

typedef enum {
	Undefined,
	DigitalIO,
	PWMGenerator,
	PWMCapture,
	Encoder,
	Accumulator,
	AnalogTrigger,
	Interrupt,
	UART,
	SPI,
	I2C,
	MaxVMXResourceType = I2C,
} VMXResourceType;

typedef uint8_t  VMXResourceIndex;
typedef uint16_t VMXResourceHandle;
typedef uint8_t  VMXResourcePortIndex;

const VMXResourceIndex INVALID_VMX_RESOURCE_INDEX = 255;

#define INVALID_VMX_RESOURCE_HANDLE(vmx_res_handle)     (((uint8_t)vmx_res_handle)==INVALID_VMX_RESOURCE_INDEX)
#define CREATE_VMX_RESOURCE_HANDLE(res_type,res_index) 	((((uint16_t)res_type)<<8) | (uint8_t)res_index)
#define EXTRACT_VMX_RESOURCE_TYPE(res_handle)			(VMXResourceType)(res_handle >> 8)
#define EXTRACT_VMX_RESOURCE_INDEX(res_handle)			(uint8_t)(res_handle & 0x00FF)

#endif /* VMXRESOURCE_H_ */
