/*
 * SystemClocks.h
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#ifndef SYSTEMCLOCKS_H_
#define SYSTEMCLOCKS_H_

#include <stdint.h>
#include "PIGPIOClient.h"

class SystemClocks {
	PIGPIOClient& pigpio;
public:
	SystemClocks(PIGPIOClient& pigpio_ref);
	virtual ~SystemClocks();

	bool Init();
	uint64_t GetCurrentSystemTimeInMicroseconds();
	uint64_t GetCurrentSystemTimeInMicrosecondsAlt();
	uint32_t GetCurrentMicroseconds();
	uint64_t GetCurrentTotalMicroseconds();
};

#endif /* SYSTEMCLOCKS_H_ */
