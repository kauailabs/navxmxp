/*
 * Power.h
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#ifndef POWER_H_
#define POWER_H_

#include "MISCClient.h"
#include "VMXErrors.h"

class Power {
	MISCClient& misc;
public:
	Power(MISCClient& misc);
	virtual ~Power();

	bool GetOvercurrent(bool& overcurrent, VMXErrorCode *errcode = 0)
	{
		MISC_EXT_PWR_CTL_STATUS power_ctl_status;
		if (misc.get_extpower_ctl_status(power_ctl_status)) {
			overcurrent = power_ctl_status.ext_pwr_overcurrent;
			return true;
		} else {
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
	}

	bool GetSystemVoltage(float& ext_power_volts, VMXErrorCode *errcode = 0)
	{
		if (!misc.get_extpower_voltage(ext_power_volts)) {
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
		return true;
	}

	bool GetOvercurrentLimitEnabled(bool& enabled, VMXErrorCode *errcode = 0) {
		MISC_EXT_PWR_CTL_CFG extpower_cfg;
		if (misc.get_extpower_cfg(extpower_cfg)) {
			enabled = extpower_cfg.ext_pwr_currentlimit_mode;
			return true;
		} else {
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
	}

	bool SetOvercurrentLimitEnabled(bool enabled, VMXErrorCode *errcode = 0) {
		MISC_EXT_PWR_CTL_CFG extpower_cfg;
		extpower_cfg.ext_pwr_currentlimit_mode = enabled;
		if (!misc.set_extpower_cfg(extpower_cfg)) {
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
		return true;
	}
};

#endif /* POWER_H_ */
