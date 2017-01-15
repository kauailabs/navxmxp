/*
 * DaGamaClient.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include <stdio.h>
#include <pigpio.h>
#include "DaGamaClient.h"

DaGamaClient::DaGamaClient() {
	int ret = gpioInitialise();
	pigpio_initialized = (ret >= 0);
	if (pigpio_initialized)
	{
		printf("pigpio library version %d opened.\n", ret);
	} else {
	   printf("Error initializing pigpio library.\n");
	}

	spi_handle = PI_BAD_HANDLE;
	NavXSPIMessage::init_crc_table();
	if(pigpio_initialized){
		unsigned spi_open_flags = 0;
		/* Mode 0, active low (all channels), reserve GPIO for SPI, 8-bits/word, std 4-wire SPI, MSB first */
		spi_open_flags |= 0x100; /* Auxiliary SPI Device */
		unsigned navxpi_aux_spi_chan = 2;

		unsigned baud = 3900000; /* APB Clock (250MHz) / 64 = 3.9Mhz */ //500000;
		spi_handle = spiOpen(navxpi_aux_spi_chan, baud, spi_open_flags);
		if ( spi_handle >= 0 ) {
		   printf("SPI Aux Channel 2 opened.\n");
		} else {
			printf("Error opening SPI AUX Channel 2.\n");
		}
	}
}

DaGamaClient::~DaGamaClient() {
	if(spi_handle != PI_BAD_HANDLE){
	   if(spiClose(spi_handle)==0){
		   printf("Closed SPI Aux Channel 2.\n");
	   } else {
			printf("Error opening SPI AUX Channel 2.\n");
	   }
	}
	if (pigpio_initialized) {
	   gpioTerminate();
	   printf("pigpio library closed.\n");
	}
}

bool DaGamaClient::transmit(uint8_t *p_data, uint8_t len)
{
	if(spi_handle == PI_BAD_HANDLE) return false;
	int write_ret = spiWrite(spi_handle, (char *)p_data, len);
	if ( write_ret < 0 ) {
	   printf("navxpi write failure %d\n", write_ret);
	} else {
	   //printf("Wrote %d bytes over SPI interface.\n", write_ret);
	}
	gpioDelay(30); /* Delays < 100us are implemented as busy wait. */
	return (write_ret == len);
}

bool DaGamaClient::receive(uint8_t *p_data, uint8_t len)
{
	if(spi_handle == PI_BAD_HANDLE) return false;
	int read_ret = spiRead(spi_handle,(char *)p_data, len);
	if ( read_ret < 0 ) {
	   printf("navxpi read failure %d\n", read_ret);
	} else {
	   //printf("Read %d bytes over SPI interface.\n", read_ret);
	}
	return (read_ret == len);
}


