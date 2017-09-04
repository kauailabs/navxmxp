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
			/*
			 * Display IO Channel Numbers/Types for all Channels which can be configured as Digital Inputs
			 */

			VMXChannelIndex first_flexio_channel_index;
			uint8_t num_flexio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::FlexDIO, first_flexio_channel_index);
			printf("FlexDIO Channel Indexes:  %d - %d\n", first_flexio_channel_index, first_flexio_channel_index + num_flexio_channels - 1);
			VMXChannelIndex first_hicurrdio_channel_index;
			uint8_t num_hicurrdio_input_channels = vmx.io.GetNumChannelsByType(VMXChannelType::HiCurrDIO, first_hicurrdio_channel_index);

			bool supports_output = vmx.io.ChannelSupportsCapability(first_hicurrdio_channel_index, VMXChannelCapability::DigitalOutput);
			printf("HiCurrDIO Header Direction:  %s\n", supports_output ? "Output" : "Input");

			if (supports_output) {
				num_hicurrdio_input_channels = 0;
				printf("HiCurrDIO Channels may NOT be used as Digital Inputs due to jumper setting.\n");
			} else {
				printf("HiCurrDIO Channel Indexes:  %d - %d\n", first_hicurrdio_channel_index, first_hicurrdio_channel_index + num_hicurrdio_input_channels - 1);
			}

			VMXChannelIndex first_comm_dio_channel_index;
			uint8_t num_comm_dio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommDIO, first_comm_dio_channel_index);
			printf("Comm/DIO Channel Indexes:  %d - %d\n", first_comm_dio_channel_index, first_comm_dio_channel_index + num_comm_dio_channels - 1);

			/*
			 * CONFIGURE Digital Inputs
			 */

			VMXErrorCode vmxerr;
			VMXResourceHandle digitalio_res_handles[num_flexio_channels + num_hicurrdio_input_channels + num_comm_dio_channels];

			uint8_t digital_input_res_handle_index = 0;

			/* Configure all FlexDIOs as INPUTS. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				DIOConfig dio_config(DIOConfig::InputMode::PULLUP);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalOutput,
						digitalio_res_handles[digital_input_res_handle_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(digitalio_res_handles[digital_input_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(digitalio_res_handles[digital_input_res_handle_index]));
				}
				digital_input_res_handle_index++;
			}

			/* Configure all HighCurrDIOs as INPUTS. */
			for ( int dio_channel_index = first_hicurrdio_channel_index; dio_channel_index < first_hicurrdio_channel_index + num_hicurrdio_input_channels; dio_channel_index++) {
				DIOConfig dio_config(DIOConfig::InputMode::PULLUP);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalOutput,
						digitalio_res_handles[digital_input_res_handle_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(digitalio_res_handles[digital_input_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(digitalio_res_handles[digital_input_res_handle_index]));
				}
				digital_input_res_handle_index++;
			}

			/* Configure all CommDIOs as INPUTS. */
			for ( int dio_channel_index = first_comm_dio_channel_index; dio_channel_index < first_comm_dio_channel_index + num_comm_dio_channels; dio_channel_index++) {
				DIOConfig dio_config(DIOConfig::InputMode::PULLUP);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalOutput,
						digitalio_res_handles[digital_input_res_handle_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(digitalio_res_handles[digital_input_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(digitalio_res_handles[digital_input_res_handle_index]));
				}
				digital_input_res_handle_index++;
			}

			for (int i = 0; i < 100; i++) {
				/* Read all GPIOs. */
				for ( int dig_in_res_index = 0; dig_in_res_index < digital_input_res_handle_index; dig_in_res_index++) {
					bool high;
					if (!vmx.io.DIO_Get(digitalio_res_handles[dig_in_res_index], high, &vmxerr)) {
						printf("Error Reading Digital Input value for Resource Index %d.\n", dig_in_res_index);
						DisplayVMXError(vmxerr);
					} else {
						printf("%d ", (high ? 1 : 0));
					}
				}
				printf("\n");
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


