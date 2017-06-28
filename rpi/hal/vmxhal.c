/*
 * vmxhal.c
 *
 *  Created on: 27 Jun 2017
 *      Author: pi
 */

#include "vmxhal.h"

void VMXBaseInitialize(int32_t *status)
{

}

VMXHAL_Initialize(int32_t mode)
{

}

const char* VMXHAL_GetErrorMessage(int32_t code)
{

}

int32_t VMXHAL_GetFirmwareVersion(int32_t* status)
{

}

int64_t VMXHAL_GetFirmwareRevision(int32_t* status)
{

}

VMXHAL_RuntimeType VMXHAL_GetRuntimeType()
{

}

VMXHAL_Bool VMXHAL_GetSystemActive(int32_t* status)
{

}

VMXHAL_Bool VMXHAL_GetBrownedOut(int32_t* status)
{

}

uint64_t VMXHAL_GetMicrocontrollerTime(int32_t* status)
{
	uint64_t uc_time = 0;	/* Todo:  Granularity? Is this uC or Rpi time? */
	return uc_time;
}

#define MAX_DIGITAL_CHANNELS  28
#define MAX_ANALOG_INPUT_CHANNELS 4

#define MAX_CHANNEL_NUMBER (MAX_DIGITAL_CHANNELS - 1)

/* PortHandles are indexes, identifical to channel numbers.
 * The channeltype (Digital [including DigitalPWM], AnalogInput) is not
 * represented by the port handle.
 *
 * PortHandle values cannot exceed the maximum number of
 * channels for any channel type
 */
VMXHAL_PortHandle VMXHAL_GetPort(int32_t channel)
{
	if(channel > MAX_DIGITAL_CHANNELS) {
		return (VMXHAL_PortHandle)-1;
	}
	return (VMXHAL_PortHandle)channel;
}

/* Only one module is present (module 1) is present. */
VMXHAL_PortHandle VMXHAL_GetPortWithModule(int32_t module, int32_t channel)
{
	return VMXHAL_GetPort(channel);
}

int64_t VMXHAL_Report(int32_t resource, int32_t instanceNumber, int32_t context,
                   const char* feature)
{

}

