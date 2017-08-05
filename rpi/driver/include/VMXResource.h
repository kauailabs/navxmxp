/*
 * VMXResource.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
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
