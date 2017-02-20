/*
 * SPIClient.cpp
 *
 *  Created on: 20 Feb 2017
 *      Author: pi
 */

#include <mutex>
#include <stdio.h>
#include <pigpio.h>

#include "SPIClient.h"
#include "NavXSPIMessage.h"

static std::mutex imu_mutex;

SPIClient::SPIClient() {
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

SPIClient::~SPIClient() {
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

static bool transmit_internal(int spi_handle, uint8_t *p_data, uint8_t len) {
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

bool receive_internal(int spi_handle, uint8_t *p_data, uint8_t len) {
	if(spi_handle == PI_BAD_HANDLE) return false;
	int read_ret = spiRead(spi_handle,(char *)p_data, len);
	if ( read_ret < 0 ) {
	   printf("navxpi read failure %d\n", read_ret);
	} else {
	   //printf("Read %d bytes over SPI interface.\n", read_ret);
	}
	return (read_ret == len);
}

bool SPIClient::transmit(uint8_t *p_data, uint8_t len)
{
	std::unique_lock<std::mutex> sync(imu_mutex);
	return transmit_internal(spi_handle, p_data, len);
}

bool SPIClient::transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len)
{
	std::unique_lock<std::mutex> sync(imu_mutex);
	if(transmit_internal(spi_handle, p_tx_data, tx_len)) {
		return receive_internal(spi_handle, p_rx_data, rx_len);
	} else {
		return false;
	}
}

bool SPIClient::write(NavXSPIMessage& request)
{
	return transmit(request.get_packet_ptr(), request.get_packet_size());
}

bool SPIClient::read(NavXSPIMessage& request, uint8_t *p_response, uint8_t response_len)
{
	if (transmit_and_receive(request.get_packet_ptr(), request.get_packet_size(), p_response, response_len)){
		if (NavXSPIMessage::validate_read_response(p_response,response_len)){
			return true;
		} else {
			printf("CRC error during read response.\n");
		}
	}
	return false;
}
