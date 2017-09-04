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
			VMXResourceHandle spi_res_handle;
			VMXChannelIndex spi_channels[4];
			spi_channels[0] = 28; /* CLK */
			spi_channels[1] = 29; /* MOSI */
			spi_channels[2] = 30; /* MISO */
			spi_channels[3] = 31; /* CS */
			VMXChannelCapability spi_channel_capabilities[4];
			spi_channel_capabilities[0] = VMXChannelCapability::SPI_CLK;
			spi_channel_capabilities[1] = VMXChannelCapability::SPI_MOSI;
			spi_channel_capabilities[2] = VMXChannelCapability::SPI_MISO;
			spi_channel_capabilities[3] = VMXChannelCapability::SPI_CS;

			SPIConfig spi_cfg(1000000 /* Bitrate */);

			if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(spi_channels), spi_channels, spi_channel_capabilities, spi_res_handle, &spi_cfg, &vmxerr)) {
				printf("Failed to Activate SPI Resource.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Successfully Activated UART Resource with VMXChannels %d, %d, %d and %d\n",
						spi_channels[0], spi_channels[1], spi_channels[2], spi_channels[3]);
			}

			for (int i = 0; i < 50; i++) {

				uint8_t rx_data[10];
				uint8_t tx_data[10];
				tx_data[0] = 1;
				tx_data[1] = 2;
				tx_data[2] = 3;
				tx_data[3] = 4;
				tx_data[4] = 5;
				if (!vmx.io.SPI_Transaction(spi_res_handle, tx_data, rx_data, 10, &vmxerr)) {
					printf("SPI Transaction Failed.\n");
					DisplayVMXError(vmxerr);
				} else {
					printf("SPI Transaction complete.  rx_data:  ");
					for (int i = 5; i < 10; i++) {
						printf("%02X ", rx_data[i]);
					}
					printf("\n");
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


