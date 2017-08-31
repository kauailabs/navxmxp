/*
 * VMXPower.h
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#ifndef VMXPOWER_H_
#define VMXPOWER_H_

#include "VMXErrors.h"

class MISCClient;

class VMXPower {
	friend class VMXPi;

	MISCClient& misc;

	VMXPower(MISCClient& misc);
	void ReleaseResources();
	virtual ~VMXPower();

public:
	bool GetOvercurrent(bool& overcurrent, VMXErrorCode *errcode = 0);
	bool GetSystemVoltage(float& ext_power_volts, VMXErrorCode *errcode = 0);
	bool GetOvercurrentLimitEnabled(bool& enabled, VMXErrorCode *errcode = 0);
	bool SetOvercurrentLimitEnabled(bool enabled, VMXErrorCode *errcode = 0);
};

#endif /* VMXPOWER_H_ */
