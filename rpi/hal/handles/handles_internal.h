/*
 * handles_internal.h
 *
 *  Created on: 27 Jun 2017
 *      Author: pi
 */

#ifndef HANDLES_INTERNAL_H_
#define HANDLES_INTERNAL_H_

#include "../types.h"

constexpr int16_t InvalidVMXHandleIndex = -1;

enum class VMXHAL_HandleEnum {
	Undefined = 0,
	DIO = 1,
	Port = 2,
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

static inline int16_t getHandleTypedIndex(HAL_Handle handle,
										  HAL_HandleEnum enumType) {
	if(!isHandleType(handle, enumType)) return InvalidHandleIndex;
	return getHandleIndex;
}
#endif /* HANDLES_INTERNAL_H_ */
