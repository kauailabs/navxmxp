/*
 * vmxhal.c
 *
 *  Created on: 27 Jun 2017
 *      Author: pi
 */

#include "../vmxhal.h"
#include "../handles/handles_internal.h"
#include "globs.h"
#include <sstream>
#include <iostream>
#include <string>

void VMXHAL_BaseInitialize(int32_t *status)
{

}

int32_t VMXHAL_Initialize(int32_t mode)
{
	uint8_t default_update_rate_hz = 50;
	p_vmx = new VMXPiClient(default_update_rate_hz);
	return VMXHAL_STATUS_OK;
}

int32_t VMXHAL_Deinitialize()
{
	delete p_vmx;

	p_vmx = 0;

	return VMXHAL_STATUS_OK;
}

const char* VMXHAL_GetErrorMessage(int32_t code)
{
	return "";
}

int32_t VMXHAL_GetFirmwareVersion(int32_t* status)
{
	std::string version_string = p_vmx->GetFirmwareVersion();
    istringstream version_stream(version_string);
    string str;
    int count = 0;
    int32_t version = -1;
    while (getline(version_stream, str, '.')) {
    	if(count == 0) {
    		version = std::stoi( str );
    		break;
    	}
    	count++;
    }
    return version;
}

int64_t VMXHAL_GetFirmwareRevision(int32_t* status)
{
	std::string version_string = p_vmx->GetFirmwareVersion();
    istringstream version_stream(version_string);
    string str;
    int count = 0;
    int64_t revision = -1;
    while (getline(version_stream, str, '.')) {
    	if(count == 1) {
    		revision = std::stoi( str );
    		break;
    	}
    	count++;
    }
    return revision;
}

VMXHAL_RuntimeType VMXHAL_GetRuntimeType()
{
	return VMXHAL_VMX;
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

#define MODULE_NUMBER 1 /* Module Numbers not used in VMX HAL */

/* PortHandles are indexes, identical to channel numbers.
 * The channeltype (Digital [including DigitalPWM], AnalogInput) is ~not~
 * represented by the port handle.
 *
 * Therefore, there can be identical VMXHAL_PortHandles for
 * PWM 3 and Analog Input 3, for example.
 *
 * PortHandle values cannot exceed the maximum number of
 * channels for any channel type
 */
VMXHAL_PortHandle VMXHAL_GetPort(int32_t channel)
{
	if(channel > MAX_DIGITAL_CHANNELS) {
		return InvalidVMXHandleIndex;
	}
	return createPortHandle(channel, 1);
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

