/*
 * SPIClient.cpp
 *
 *  Created on: 20 Feb 2017
 *      Author: pi
 */

#include <mutex>
#include <stdio.h>
#include "SPIClient.h"
#include "NavXSPIMessage.h"
#include "SPICommCtrl.h"

static std::mutex vmx_spi_mutex;

SPIClient::SPIClient(PIGPIOClient& gpio_client) :
	pigpio(gpio_client) {
	NavXSPIMessage::init_crc_table();
}

SPIClient::~SPIClient() {
}

bool SPIClient::transmit(uint8_t *p_data, uint8_t len, bool write)
{
	std::unique_lock<std::mutex> sync(vmx_spi_mutex);
	if (write && (len > NavXSPIMessage::get_standard_packet_size())) {
		/* If this is a write request w/larger-than-standard packet size, switch to temporary variable write length mode */
		NavXSPIMessage var_write_request_msg(COMM_MODE_BANK,
				COMM_MODE_REG_VARIABLEWRITE | 0x80,
				len,
				NULL);
		if(!pigpio.int_spi_transmit(var_write_request_msg.get_packet_ptr(), var_write_request_msg.get_packet_size(), true)) {
			return false;
		}
	}

	return pigpio.int_spi_transmit(p_data, len, write);
}

/* Note:  rx_len should include one byte for the CRC, in addition to the
 * amount of expected data.
 */
bool SPIClient::transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len, bool write)
{
	std::unique_lock<std::mutex> sync(vmx_spi_mutex);
	if(pigpio.int_spi_transmit(p_tx_data, tx_len, write)) {
		return pigpio.int_spi_receive(p_rx_data, rx_len);
	} else {
		return false;
	}
}

bool SPIClient::write(NavXSPIMessage& request)
{
	return transmit(request.get_packet_ptr(), request.get_packet_size(), true);
}

/* Note:  response_len should include one byte for the CRC, in addition to the
 * amount of expected data.
 */
bool SPIClient::read(NavXSPIMessage& request, uint8_t *p_response, uint8_t response_len)
{
	if (transmit_and_receive(request.get_packet_ptr(), request.get_packet_size(), p_response, response_len, false)){
		if (NavXSPIMessage::validate_read_response(p_response,response_len)){
			//printf("Read Complete:  %d bytes, CRC:  %u, Data:  %u %u %u %u %u\n", response_len, p_response[response_len-1], p_response[0], p_response[1], p_response[2], p_response[3], p_response[4]);
			return true;
		} else {
			printf("CRC error during read response.\n");
		}
	}
	return false;
}
