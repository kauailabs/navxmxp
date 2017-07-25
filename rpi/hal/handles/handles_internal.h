/*
 * handles_internal.h
 *
 *  Created on: 27 Jun 2017
 *      Author: pi
 */

#ifndef HANDLES_INTERNAL_H_
#define HANDLES_INTERNAL_H_

#include "../types.h"
#include "VMXResource.h"

constexpr int16_t InvalidVMXHandleIndex = -1;

enum class VMXHAL_HandleEnum {
	Undefined = 0,
	DIO = 1,
	Port = 2,			/* AKA "Channel" */
	Notifier = 3,
	Interrupt = 4,
	AnalogInput = 6,
	AnalogTrigger = 7,
	DigitalPWM = 10,
	Counter = 11,
	Encoder = 13,
	Vendor = 17
};

/* specialized functions for Port handle
 * Port Handle Data Layout
 * Bits 0-7:    Channel Number
 * Bits 8-15:   Module Number (not used)
 * Bits 16-23:  Unused
 * Bits 24-30:  Handle Type
 * Bit 31:      1 if handle error, 0 if no error
 */

static inline int16_t getHandleIndex(VMXHAL_Handle handle) {
	// mask and return last 16 bits
	return static_cast<int16_t>(handle & 0xffff);
}

static inline VMXHAL_HandleEnum getHandleType(VMXHAL_Handle handle) {
	return static_cast<VMXHAL_HandleEnum>((handle >> 24) & 0xff);
}

static inline bool isHandleType(VMXHAL_Handle handle, VMXHAL_HandleEnum handleType) {
	return handleType == getHandleType(handle);
}

static inline int16_t getHandleTypedIndex(VMXHAL_Handle handle,
										  VMXHAL_HandleEnum enumType) {
	if(!isHandleType(handle, enumType)) return InvalidVMXHandleIndex;
	return getHandleIndex(handle);
}

VMXHAL_PortHandle createPortHandle(uint8_t channel,  uint8_t module);
VMXHAL_PortHandle createPortHandleForSPI(uint8_t channel);
VMXHAL_Handle createHandle(int16_t index, VMXHAL_HandleEnum handleType);

bool getResourceHandle(VMXHAL_Handle handle, VMXResourceHandle& vmx_res_handle)
{
	VMXHAL_HandleEnum vmxhal_handle_type = getHandleType(handle);
	int16_t vmxhal_handle_index = getHandleIndex(handle);
	VMXResourceType vmx_res_type = VMXResourceType::Undefined;
	switch(vmxhal_handle_type) {
	case VMXHAL_HandleEnum::Undefined:
		vmx_res_type = VMXResourceType::Undefined;
		break;
	case VMXHAL_HandleEnum::DigitalPWM:
		vmx_res_type = VMXResourceType::PWMGenerator;
		break;
	case VMXHAL_HandleEnum::DIO:
		vmx_res_type = VMXResourceType::DIO;
		break;
	case VMXHAL_HandleEnum::Notifier:
		// TODO::::
		break;
	case VMXHAL_HandleEnum::Interrupt:
		vmx_res_type = VMXResourceType::Interrupt;
		break;
	case VMXHAL_HandleEnum::AnalogInput:
		vmx_res_type = VMXResourceType::Accumulator;
		break;
	case VMXHAL_HandleEnum::AnalogTrigger:
		vmx_res_type = VMXResourceType::AnalogTrigger;
		break;
	case VMXHAL_HandleEnum::Counter:
		vmx_res_type = VMXResourceType::PWMCapture;
		break;
	case VMXHAL_HandleEnum::Encoder:
		vmx_res_type = VMXResourceType::Encoder;
		break;
	case VMXHAL_HandleEnum::Port:
		// This does not map to a VMXResourceHandle
		return false;
	case VMXHAL_HandleEnum::Vendor:
		// This does not map to a VMXResourceHandle
		return false;
	}
	vmx_res_handle = CREATE_VMX_RESOURCE_HANDLE(vmx_res_type, uint8_t(vmxhal_handle_index));
	return true;
}

#endif /* HANDLES_INTERNAL_H_ */
