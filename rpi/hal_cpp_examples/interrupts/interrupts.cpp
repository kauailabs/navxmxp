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

static void VMXIOInterruptHandler(uint32_t io_interrupt_num,
		InterruptEdgeType edge,
		void* param,
		uint64_t timestamp_us)
{
	const char *edge_type = "unknown";
	switch(edge)
	{
	case InterruptEdgeType::RISING_EDGE_INTERRUPT:
		edge_type = "Rising";
		break;
	case InterruptEdgeType::FALLING_EDGE_INTERRUPT:
		edge_type = "Falling";
		break;
	}
	printf("IO Interrupt Received.  Number:  %d, Edge:  %s, Param:  %x, Timestamp:  %" PRIu64 "\n",
			io_interrupt_num,
			edge_type,
			(unsigned int)param,
			timestamp_us);
}

int main(int argc, char *argv[])
{
	bool realtime = false;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	try {
		if(vmx.IsOpen()) {
			/*
			 * Display IO Channel Numbers/Types for all Channels which can be configured as Interrupts
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
			 * CONFIGURE Interrupts
			 */

			VMXErrorCode vmxerr;
			VMXResourceHandle interrupt_res_handles[num_flexio_channels + num_hicurrdio_input_channels + num_comm_dio_channels];

			uint8_t interrupt_res_handle_index = 0;

			/* Configure all FlexDIOs as INTERRUPTS. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(dio_channel_index));
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::InterruptInput,
						interrupt_res_handles[interrupt_res_handle_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(interrupt_res_handles[interrupt_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(interrupt_res_handles[interrupt_res_handle_index]));
				}
				interrupt_res_handle_index++;
			}

			/* Configure all HighCurrDIOs as INTERRUPTS. */
			for ( int dio_channel_index = first_hicurrdio_channel_index; dio_channel_index < first_hicurrdio_channel_index + num_hicurrdio_input_channels; dio_channel_index++) {
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(dio_channel_index));
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::InterruptInput,
						interrupt_res_handles[interrupt_res_handle_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(interrupt_res_handles[interrupt_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(interrupt_res_handles[interrupt_res_handle_index]));
				}
				interrupt_res_handle_index++;
			}

			/* Configure all CommDIOs as INTERRUPTS. */
			for ( int dio_channel_index = first_comm_dio_channel_index; dio_channel_index < first_comm_dio_channel_index + num_comm_dio_channels; dio_channel_index++) {
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(dio_channel_index));
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::InterruptInput,
						interrupt_res_handles[interrupt_res_handle_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(interrupt_res_handles[interrupt_res_handle_index]),
							EXTRACT_VMX_RESOURCE_INDEX(interrupt_res_handles[interrupt_res_handle_index]));
				}
				interrupt_res_handle_index++;
			}

			/* Delay for awhile; any interrupts recieved will be output to console by the interrupt handler registered above. */
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


