/*
 * VMXPi.h
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#ifndef VMXPI_H_
#define VMXPI_H_

#include <stdint.h>
#include "AHRS.h"
#include "VMXCAN.h"
#include "VMXIO.h"
#include "VMXPower.h"
#include "VMXTime.h"
#include "VMXVersion.h"
#include "VMXThread.h"

class VMXPiImpl;

class VMXPi {

	VMXPiImpl*	p_impl;

public:

	AHRS 		ahrs;
	VMXTime 	time;
	VMXIO 		io;
	VMXCAN		can;
	VMXPower	power;
	VMXVersion	version;
	VMXThread	thread;

	VMXPi(bool realtime, uint8_t ahrs_update_rate_hz);
	virtual ~VMXPi();

	bool IsOpen();
};

#endif /* VMXPI_H_ */
