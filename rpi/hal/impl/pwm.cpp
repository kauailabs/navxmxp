/*
 * pwm.c
 *
 *  Created on: 27 Jun 2017
 *      Author: pi
 */

#include "../pwm.h"
#include "../handles/handles_internal.h"
#include "globs.h"
#include "../errors.h"

typedef struct {
	VMXHAL_DigitalHandle pwm_port_handle;
	bool allocated;
} VMXDigitalHandle;


VMXHAL_DigitalHandle VMXHAL_InitializePWMPort(	VMXHAL_PortHandle portHandle,
                                        		int32_t* status)
{
	int vmx_channel = getHandleIndex(portHandle);
	VMXHAL_HandleEnum handleType = getHandleType(portHandle);

	if(handleType == VMXHAL_HandleEnum::Port) {
		if (VMXHAL_CheckPWMChannel(vmx_channel)) {
			/* There's a 1-1 mapping between channel index and PWM index */
			uint8_t pwm_resource_index = uint8_t(vmx_channel);
			VMXResourceHandle vmx_pwm_res_handle = p_vmx->io.AllocateResource(VMXResourceType::PWMGenerator, pwm_resource_index);
			if(!INVALID_VMX_RESOURCE_HANDLE(vmx_pwm_res_handle)) {
				VMXHAL_DigitalHandle pwm_res_handle = createHandle(pwm_resource_index, VMXHAL_HandleEnum::DigitalPWM);
				if(p_vmx->io.RouteChannelToResource(uint8_t(vmx_channel), pwm_resource_index)) {
					*status = VMXHAL_STATUS_OK;
					return pwm_res_handle;
				} else {
					*status = RESOURCE_IS_ALLOCATED;
				}
			} else {
				*status = HAL_HANDLE_ERROR;
			}
		} else {
			*status = PARAMETER_OUT_OF_RANGE;
		}
	} else {
		*status = HAL_HANDLE_ERROR;
	}
	return VMXHAL_kInvalidHandle;
}

void VMXHAL_FreePWMPort(VMXHAL_DigitalHandle pwmPortHandle, int32_t* status)
{
	if(p_vmx->io.DeallocateResource(pwmPortHandle)) {
		*status = VMXHAL_STATUS_OK;
	} else {
		*status = HAL_HANDLE_ERROR;
	}
}

VMXHAL_Bool VMXHAL_CheckPWMChannel(int32_t vmx_channel)
{
	return (p_vmx->chan_mgr.ChannelSupportsCapability(
			uint8_t(vmx_channel), VMXChannelCapability::PWMGeneratorOutput)) ? 0 : !0;
}

