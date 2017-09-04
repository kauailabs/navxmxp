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


			VMXErrorCode vmxerr;
			VMXResourceHandle i2c_res_handle;
			VMXChannelIndex i2c_channels[2];
			i2c_channels[0] = 32; /* SDA */
			i2c_channels[1] = 33; /* SCL */
			VMXChannelCapability i2c_channel_capabilities[2];
			i2c_channel_capabilities[0] = VMXChannelCapability::I2C_SDA;
			i2c_channel_capabilities[1] = VMXChannelCapability::I2C_SCL;

			I2CConfig i2c_cfg;

			if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(i2c_channels), i2c_channels, i2c_channel_capabilities, i2c_res_handle, &i2c_cfg, &vmxerr)) {
				printf("Failed to Activate I2C Resource.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Successfully Activated I2C Resource with VMXChannels %d and %d\n",
						i2c_channels[0], i2c_channels[1]);
			}

			uint8_t device_7bit_i2c_address = 0x50;
			uint8_t tx_data[1];
			uint8_t rx_data[1];

			tx_data[0] = 1;
			for (int i = 0; i < 50; i++) {
				if (!vmx.io.I2C_Transaction(i2c_res_handle, device_7bit_i2c_address,
				                    tx_data, ARRAY_SIZE(tx_data),
				                    rx_data, ARRAY_SIZE(rx_data),
				                    &vmxerr)) {
					printf("I2C Transaction Error.\n");
					DisplayVMXError(vmxerr);
				} else {
					printf("I2C rx data:  %02x\n", rx_data[0]);
				}

				vmx.time.DelayMilliseconds(20);
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


