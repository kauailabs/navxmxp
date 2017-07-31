/*
 * Version.h
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#ifndef VERSION_H_
#define VERSION_H_

#include "AHRS.h"

class Version {
	AHRS& ahrs;
public:
	Version(AHRS& ahrs_ref);
	virtual ~Version();

	/* VERSION */

	std::string GetFirmwareVersion() { return ahrs.GetFirmwareVersion(); }
};

#endif /* VERSION_H_ */
