/*
 * VMXVersion.h
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#ifndef VMXVERSION_H_
#define VMXVERSION_H_

#include "AHRS.h"

class VMXVersion {

	friend class VMXPi;

	AHRS& ahrs;

	VMXVersion(AHRS& ahrs_ref);
	void ReleaseResources();
	virtual ~VMXVersion();

public:
	std::string GetFirmwareVersion();
	std::string GetHALVersion();
};

#endif /* VMXVERSION_H_ */
