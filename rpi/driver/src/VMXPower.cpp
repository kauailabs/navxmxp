/*
 * VMXPower.cpp
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#include "VMXPower.h"
#include "MISCClient.h"

VMXPower::VMXPower(MISCClient& misc_ref) :
	misc(misc_ref)
{
}

void VMXPower::ReleaseResources()
{
}

VMXPower::~VMXPower() {
	ReleaseResources();
}

bool VMXPower::GetOvercurrent(bool& overcurrent, VMXErrorCode *errcode)
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

bool VMXPower::GetSystemVoltage(float& ext_power_volts, VMXErrorCode *errcode)
{
	if (!misc.get_extpower_voltage(ext_power_volts)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXPower::GetOvercurrentLimitEnabled(bool& enabled, VMXErrorCode *errcode) {
	MISC_EXT_PWR_CTL_CFG extpower_cfg;
	if (misc.get_extpower_cfg(extpower_cfg)) {
		enabled = extpower_cfg.ext_pwr_currentlimit_mode;
		return true;
	} else {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
}

bool VMXPower::SetOvercurrentLimitEnabled(bool enabled, VMXErrorCode *errcode) {
	MISC_EXT_PWR_CTL_CFG extpower_cfg;
	extpower_cfg.ext_pwr_currentlimit_mode = enabled;
	if (!misc.set_extpower_cfg(extpower_cfg)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}
