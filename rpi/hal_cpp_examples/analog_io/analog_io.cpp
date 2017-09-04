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
			 * CONFIGURE ANALOG ACCUMULATOR RESOURCES
			 */

			VMXErrorCode vmxerr;
			VMXChannelIndex first_anin_channel;
			uint8_t num_analog_inputs = vmx.io.GetNumChannelsByType(VMXChannelType::AnalogIn, first_anin_channel);
			VMXResourceHandle accumulator_res_handles[4];

			for ( uint8_t analog_in_chan_index = first_anin_channel; analog_in_chan_index < first_anin_channel + num_analog_inputs; analog_in_chan_index++) {
				VMXResourceIndex accum_res_index = analog_in_chan_index - first_anin_channel;
				AccumulatorConfig accum_config;
				if (!vmx.io.ActivateSinglechannelResource(analog_in_chan_index, VMXChannelCapability::AccumulatorInput,
						accumulator_res_handles[accum_res_index], &accum_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Accumulator for Channel index %d.\n", analog_in_chan_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("Analog Input Channel %d activated on Resource type %d, index %d\n", analog_in_chan_index,
							EXTRACT_VMX_RESOURCE_TYPE(accumulator_res_handles[accum_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(accumulator_res_handles[accum_res_index]));
				}
			}

			for ( int i = 0; i < 50; i++) {
				/* Display Analog Input Values */
				for (int j = 0; j < num_analog_inputs; j++){
					float an_in_voltage;
					if(vmx.io.Accumulator_GetAverageVoltage(accumulator_res_handles[j], an_in_voltage, &vmxerr)){
						printf("Analog In Channel %d Voltage:  %0.3f\n", j, an_in_voltage);
					} else {
						printf("Error getting Average Voltage of analog accumulator %d\n", j);
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


