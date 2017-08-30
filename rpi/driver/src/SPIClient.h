/*
 * SPIClient.h
 *
 *  Created on: 20 Feb 2017
 *      Author: pi
 */

#ifndef SPICLIENT_H_
#define SPICLIENT_H_

#include "PIGPIOClient.h"
#include "NavXSPIMessage.h"

class SPIClient {

private:
	PIGPIOClient& pigpio;

public:
	SPIClient(PIGPIOClient& gpio_client);

	/* Transmits data of the specified length to VMX via internal SPI communication.  */
	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	bool transmit(uint8_t *p_data, uint8_t len, bool write);

	/* Requests data of the specified length from VMX via internal SPI communication.  */
	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	/* Note:  rx_len should include one byte for the CRC, in addition to the
	 * amount of expected data.
	 */
	bool transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len, bool write);

	/* Transmits the NAVXPMessage to VMX via internal SPI communication. */
	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	bool write(NavXSPIMessage& write);

	/* Transmits the read-request NAVXPMessage to VMX via internal SPI communication. */
	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	/* Note:  response_len should include one byte for the CRC, in addition to the
	 * amount of expected data.
	 */
	bool read(NavXSPIMessage& request, uint8_t *p_response, uint8_t response_len);

	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	template<typename T> bool read(uint8_t bank, uint8_t offset, T& value)
	{
		NavXSPIMessage msg(bank,
				offset,
				sizeof(T));
		uint8_t response_packet[sizeof(T) + 1];
		if(read(msg, response_packet, sizeof(response_packet))){
			value = (T)*((T*)(&response_packet[0]));
			return true;
		}
		return false;
	}

	/* Internally, uses a mutex to ensure VMX SPI transactions are atomic.            */
	template<typename T> bool write(uint8_t bank, uint8_t offset, T value)
	{
		NavXSPIMessage msg(bank,
				offset,
				sizeof(T),
				(uint8_t *)&value);
		return write(msg);
	}

	~SPIClient();
};

#endif /* SPICLIENT_H_ */
