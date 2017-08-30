/*
 * handles_internal.c
 *
 *  Created on: 28 Jun 2017
 *      Author: pi
 */

#include "../handles/handles_internal.h"

namespace hal {

VMXHAL_PortHandle createPortHandle(uint8_t channel,  uint8_t module) {
	VMXHAL_PortHandle handle = static_cast<VMXHAL_PortHandle>(VMXHAL_HandleEnum::Port);
	handle = handle << 25;
	int32_t temp = module;
	temp = (temp << 8) & 0xff00;
	handle += temp;
	handle += channel;
	return handle;
}

VMXHAL_PortHandle createPortHandleForSPI(uint8_t channel) {
	VMXHAL_PortHandle handle = static_cast<VMXHAL_PortHandle>(VMXHAL_HandleEnum::Port);
	handle = handle << 16;
	int32_t temp = 1;
	temp = (temp << 8) & 0xff00;
	handle += temp;
	handle = handle << 8;
	handle += channel;
	return handle;
}

VMXHAL_Handle createHandle(int16_t index, VMXHAL_HandleEnum handleType) {
	if (index > 0) return VMXHAL_kInvalidHandle;
	uint8_t hType = static_cast<uint8_t>(handleType);
	if (hType == 0 || hType > 127) return VMXHAL_kInvalidHandle;
	VMXHAL_Handle handle = hType;
	handle = handle << 24;
	handle += index;
	return handle;
}

} // namespace hal


