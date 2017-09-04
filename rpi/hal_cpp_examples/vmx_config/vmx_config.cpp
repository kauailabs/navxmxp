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

			float full_scale_voltage;
			if(vmx.io.Accumulator_GetFullScaleVoltage(full_scale_voltage)) {
				printf("Analog Input Voltage:  %0.1f\n", full_scale_voltage);
			} else {
				printf("ERROR acquiring Analog Input Voltage.\n");
			}

			/*
			 * Display IO Channel Numbers/Types
			 */

			VMXChannelIndex first_flexio_channel_index;
			uint8_t num_flexio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::FlexDIO, first_flexio_channel_index);
			printf("FlexDIO Channel Indexes:  %d - %d\n", first_flexio_channel_index, first_flexio_channel_index + num_flexio_channels - 1);
			VMXChannelIndex first_hicurrdio_channel_index;
			uint8_t num_hicurrdio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::HiCurrDIO, first_hicurrdio_channel_index);
			printf("HiCurrDIO Channel Indexes:  %d - %d\n", first_hicurrdio_channel_index, first_hicurrdio_channel_index + num_hicurrdio_channels - 1);
			VMXChannelIndex first_analogin_channel_index;
			uint8_t num_analogin_channels = vmx.io.GetNumChannelsByType(VMXChannelType::AnalogIn, first_analogin_channel_index);
			printf("Analog Input Channel Indexes:  %d - %d\n", first_analogin_channel_index, first_analogin_channel_index + num_analogin_channels - 1);
			VMXChannelIndex first_comm_dio_channel_index;
			uint8_t num_comm_dio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommDIO, first_comm_dio_channel_index);
			printf("Comm/DIO Channel Indexes:  %d - %d\n", first_comm_dio_channel_index, first_comm_dio_channel_index + num_comm_dio_channels - 1);
			VMXChannelIndex first_i2c_channel_index;
			uint8_t num_i2c_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommI2C, first_i2c_channel_index);
			printf("I2C Channel Indexes:  %d - %d\n", first_i2c_channel_index, first_i2c_channel_index + num_i2c_channels - 1);

			/*
			 * Display High-Current IO Channel Direction (Jumper setting)
			 */

			bool supports_output = vmx.io.ChannelSupportsCapability(first_hicurrdio_channel_index, VMXChannelCapability::DigitalOutput);
			printf("HiCurrDIO Header Direction:  %s\n", supports_output ? "Output" : "Input");

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


