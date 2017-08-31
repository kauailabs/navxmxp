/*
 * VMXThread.h
 *
 *  Created on: 5 Aug 2017
 *      Author: pi
 */

#ifndef VMXTHREAD_H_
#define VMXTHREAD_H_

#include <stdint.h>

class PIGPIOClient;

#include "VMXErrors.h"

class VMXThread {

	friend class VMXPi;

	PIGPIOClient& pigpio;

	VMXThread(PIGPIOClient& pigpio_ref);
	virtual ~VMXThread();
	void ReleaseResources();

public:
};

#endif /* VMXTHREAD_H_ */
