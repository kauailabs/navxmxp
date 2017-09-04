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
			VMXResourceHandle uart_res_handle;
			VMXChannelIndex uart_channels[2];
			uart_channels[0] = 26; /* TX */
			uart_channels[1] = 27; /* RX */
			VMXChannelCapability uart_channel_capabilities[2];
			uart_channel_capabilities[0] = VMXChannelCapability::UART_TX;
			uart_channel_capabilities[1] = VMXChannelCapability::UART_RX;

			UARTConfig uart_cfg(57600 /* Bitrate */);

			if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(uart_channels), uart_channels, uart_channel_capabilities, uart_res_handle, &uart_cfg, &vmxerr)) {
				printf("Failed to Activate UART Resource.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Successfully Activated UART Resource with VMXChannels %d and %d\n", uart_channels[0], uart_channels[1]);
			}

			for (int i = 0; i < 50; i++) {

				if (!vmx.io.UART_Write(uart_res_handle, (uint8_t *)"Data", 4, &vmxerr)) {
					printf("Failed to Write UART Data.\n");
					DisplayVMXError(vmxerr);
				} else {
					printf("Wrote UART Data.\n");
				}
				uint16_t bytes_available;
				if (!vmx.io.UART_GetBytesAvailable(uart_res_handle, bytes_available, &vmxerr)) {
					printf("Error getting UART bytes available.\n");
				} else {
					if (bytes_available == 0) {
						printf("No UART data available.\n");
					} else {
						printf("Got %d bytes of UART data:  ", bytes_available);
						uint8_t bytes[bytes_available];
						uint16_t bytes_actually_read;
						if (!vmx.io.UART_Read(uart_res_handle, bytes, bytes_available, bytes_actually_read, &vmxerr)) {
							printf("ERROR READING RECEIVED BYTES.\n");
						} else {
							for (uint16_t byte = 0; byte < bytes_actually_read; byte++) {
								printf("%02x", bytes[byte]);
							}
							printf("\n");
						}
					}
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


