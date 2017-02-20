/*
 * DaGamaClient.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include "DaGamaClient.h"

DaGamaClient::DaGamaClient(uint8_t ahrs_update_rate_hz) :
	SPIClient(),
	AHRS((SPIClient&)*this, ahrs_update_rate_hz),
	IOCXClient((SPIClient&)*this),
	CANClient((SPIClient&)*this) {
}

DaGamaClient::~DaGamaClient() {
}


