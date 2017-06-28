/*
 * DaGamaClient.h
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#ifndef DAGAMACLIENT_H_
#define DAGAMACLIENT_H_

#include <stdint.h>
#include "SPIClient.h"
#include "AHRS.h"
#include "IOCXClient.h"
#include "CANClient.h"

class DaGamaClient : public SPIClient, public AHRS, public IOCXClient, public CANClient {
public:
	DaGamaClient(uint8_t ahrs_update_rate_hz);

	virtual ~DaGamaClient();

	bool enable_rpi_gpio_interrupt(unsigned vmx_pi_gpio_num);
	bool disable_rpi_gpio_interrupt_int(unsigned vmx_pi_gpio_num);


private:
	static void gpio_isr(int gpio, int level, uint32_t tick, void *userdata);
};

#endif /* DAGAMACLIENT_H_ */
