/*
 * DaGamaClient.h
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#ifndef DAGAMACLIENT_H_
#define DAGAMACLIENT_H_

#include <stdint.h>
#include "IOCXClient.h"

class DaGamaClient : public IOCXClient {
private:
	bool pigpio_initialized;
	int spi_handle;
public:
	DaGamaClient();

	bool is_open() { return pigpio_initialized; }

	virtual bool transmit(uint8_t *p_data, uint8_t len);
	virtual bool transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len);

	virtual ~DaGamaClient();
};

#endif /* DAGAMACLIENT_H_ */
