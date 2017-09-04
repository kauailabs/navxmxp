/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h>  /* printf() */
#include <string.h> /* memcpy() */
#include <inttypes.h>

#include "VMXPi.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

void DisplayVMXError(VMXErrorCode vmxerr) {
	const char *p_err_description = GetVMXErrorString(vmxerr);
	printf("VMXError %d:  %s\n", vmxerr, p_err_description);
}

int main(int argc, char *argv[])
{
	bool realtime = false;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	try {
		if(vmx.IsOpen()) {
			VMXChannelIndex first_flexio_channel_index;
			uint8_t num_flexio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::FlexDIO, first_flexio_channel_index);
			printf("FlexDIO Channel Indexes:  %d - %d\n", first_flexio_channel_index, first_flexio_channel_index + num_flexio_channels - 1);
			VMXChannelIndex first_hicurrdio_channel_index;
			uint8_t num_hicurrdio_input_channels = vmx.io.GetNumChannelsByType(VMXChannelType::HiCurrDIO, first_hicurrdio_channel_index);

			bool supports_output = vmx.io.ChannelSupportsCapability(first_hicurrdio_channel_index, VMXChannelCapability::DigitalOutput);
			printf("HiCurrDIO Header Direction:  %s\n", supports_output ? "Output" : "Input");

			if (!supports_output) {
				num_hicurrdio_input_channels = 0;
				printf("HiCurrDIO Channels may NOT be used for PWM Generation due to jumper setting.\n");
			} else {
				printf("HiCurrDIO Channel Indexes:  %d - %d\n", first_hicurrdio_channel_index, first_hicurrdio_channel_index + num_hicurrdio_input_channels - 1);
			}

			VMXChannelIndex first_comm_dio_channel_index;
			uint8_t num_comm_dio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommDIO, first_comm_dio_channel_index);
			printf("Comm/DIO Channel Indexes:  %d - %d\n", first_comm_dio_channel_index, first_comm_dio_channel_index + num_comm_dio_channels - 1);

			/* Allocate FlexIO Channels to PWM Generation; note that the PWM Generator Resources use w/FlexIO Channels */
			/* have two ports, and thus FlexIO channels must be routed in pairs to PWM Generator Resources */
			/* It is not required that both channels be routed to PWM Generation simultaneously - but if */
			/* they ~are~ both routed, they must both be routed to the same resource. */

			VMXErrorCode vmxerr;

			const uint8_t num_pwmgen_resources = num_flexio_channels / 2;

			for (int pwmgen_index = 0;
					pwmgen_index < num_pwmgen_resources;
					pwmgen_index++ ) {

				VMXResourceHandle pwmgen_res_handle;
				VMXChannelIndex pwm_channels[2];
				pwm_channels[0] = first_flexio_channel_index + (pwmgen_index * 2) + 0;
				pwm_channels[1] = first_flexio_channel_index + (pwmgen_index * 2) + 1;
				VMXChannelCapability pwm_channel_capabilities[2];
				pwm_channel_capabilities[0] = VMXChannelCapability::PWMGeneratorOutput;
				pwm_channel_capabilities[1] = VMXChannelCapability::PWMGeneratorOutput2;

				PWMGeneratorConfig pwmgen_cfg(200 /* Frequency in Hz */);

				if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(pwm_channels), pwm_channels, pwm_channel_capabilities, pwmgen_res_handle, &pwmgen_cfg, &vmxerr)) {
					printf("Failed to Activate PWMGenerator Resource %d.\n", pwmgen_index);
					DisplayVMXError(vmxerr);
					vmx.io.DeallocateResource(pwmgen_res_handle);
					continue;
				} else {
					printf("Successfully Activated PWMGenerator Resource %d with VMXChannels %d and %d\n", pwmgen_index, pwm_channels[0], pwm_channels[1]);
					for (uint8_t port_index = 0; port_index < 2; port_index++) {
						if (!vmx.io.PWMGenerator_SetDutyCycle(pwmgen_res_handle, port_index, 128, &vmxerr)) {
							printf("Failed to set DutyCycle for PWMGenerator Resource %d, Port %d.\n", pwmgen_index, port_index);
							DisplayVMXError(vmxerr);
						}
					}
				}
			}

			/* Configure all HighCurrDIOs for PWM Generation. */
			for ( int dio_channel_index = first_hicurrdio_channel_index; dio_channel_index < first_hicurrdio_channel_index + num_hicurrdio_input_channels; dio_channel_index++) {
				VMXResourceHandle pwmgen_res_handle;
				PWMGeneratorConfig pwmgen_cfg(200 /* Frequency in Hz */);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::PWMGeneratorOutput,
						pwmgen_res_handle, &pwmgen_cfg, &vmxerr)) {
					printf("Error Activating Singlechannel Resource PWM Generator for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(pwmgen_res_handle),
							EXTRACT_VMX_RESOURCE_INDEX(pwmgen_res_handle));
				}
			}

			/* Configure all CommDIOs for PWM Generation. */
			for ( int dio_channel_index = first_comm_dio_channel_index; dio_channel_index < first_comm_dio_channel_index + num_comm_dio_channels; dio_channel_index++) {
				VMXResourceHandle pwmgen_res_handle;
				PWMGeneratorConfig pwmgen_cfg(200 /* Frequency in Hz */);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::PWMGeneratorOutput,
						pwmgen_res_handle, &pwmgen_cfg, &vmxerr)) {
					printf("Error Activating Singlechannel Resource PWM Generator for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(pwmgen_res_handle),
							EXTRACT_VMX_RESOURCE_INDEX(pwmgen_res_handle));
				}
			}
			/* Delay for awhile; during this time, PWM will be generated on all PWM channels. */
			vmx.time.DelaySeconds(10);
		} else {
			printf("Error:  Unable to open VMX Client.\n");
			printf("\n");
			printf("        - Is pigpio functional/running?\n");
			printf("        - Does this application have root privileges?\n");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}


