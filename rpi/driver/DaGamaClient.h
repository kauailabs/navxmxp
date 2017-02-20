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
};

#endif /* DAGAMACLIENT_H_ */
