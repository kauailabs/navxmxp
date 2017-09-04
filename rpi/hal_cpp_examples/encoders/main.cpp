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

			/*
			 * CONFIGURE ENCODER RESOURCES
			 */

			const uint8_t num_encoder_resources = vmx.io.GetNumResourcesByType(VMXResourceType::Encoder);

			VMXErrorCode vmxerr;

			/* Configure Quad Encoder Resources */
			/* Two VMXChannels must be routed to each Quad Encoder */
			const VMXResourceIndex first_encoder_index = 0;
			for ( VMXResourceIndex encoder_index = first_encoder_index;
					encoder_index < (first_encoder_index + num_encoder_resources);
					encoder_index++) {
				VMXChannelIndex enc_channels[2];
				enc_channels[0] = first_flexio_channel_index + (encoder_index * 2) + 0;
				enc_channels[1] = first_flexio_channel_index + (encoder_index * 2) + 1;
				VMXChannelCapability encoder_channel_capabilities[2];
				encoder_channel_capabilities[0] = VMXChannelCapability::EncoderAInput;
				encoder_channel_capabilities[1] = VMXChannelCapability::EncoderBInput;

				VMXResourceHandle encoder_res_handle;
				EncoderConfig encoder_cfg(EncoderConfig::EncoderEdge::x4);

				if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(enc_channels), enc_channels, encoder_channel_capabilities, encoder_res_handle, &encoder_cfg, &vmxerr)) {
					printf("Failed to Activate Encoder Resource %d.\n", encoder_index);
					DisplayVMXError(vmxerr);
					vmx.io.DeallocateResource(encoder_res_handle);
					continue;
				} else {
					printf("Successfully Activated Encoder Resource %d with VMXChannels %d and %d\n", encoder_index, enc_channels[0], enc_channels[1]);
				}
			}

			for ( int i = 0; i < 10; i++) {
				/* Display Quad Encoder Input Counts */
				for ( int encoder_index = first_encoder_index; encoder_index < first_encoder_index + num_encoder_resources; encoder_index++) {
					int32_t counter;
					VMXResourceHandle encoder_res_handle;
					if (!vmx.io.GetResourceHandle(VMXResourceType::Encoder, encoder_index, encoder_res_handle, &vmxerr)) {
						DisplayVMXError(vmxerr);
						continue;
					}

					if (vmx.io.Encoder_GetCount(encoder_res_handle, counter, &vmxerr)) {
						printf("Encoder %d count    :  %d.\n", encoder_index, counter);
						VMXIO::EncoderDirection encoder_direction;
						if(vmx.io.Encoder_GetDirection(encoder_res_handle, encoder_direction, &vmxerr)) {
							printf("Encoder %d direction:  %s.\n", encoder_index, (encoder_direction == VMXIO::EncoderForward) ? "Forward" : "Reverse");
						} else {
							printf("Error retrieving Encoder %d direction.\n", encoder_index);
							DisplayVMXError(vmxerr);
						}
					} else {
						printf("Error retrieving Encoder %d count.\n", encoder_index);
						DisplayVMXError(vmxerr);
					}
				}
				vmx.time.DelayMilliseconds(10);
			}
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


