/*
 * MISCClient.h
 *
 *  Created on: 15 Jul 2017
 *      Author: pi
 */

#ifndef MISCCLIENT_H_
#define MISCCLIENT_H_

#include <stdint.h>

#include "CANRegisters.h"
#include "SPIClient.h"
#include "MISCRegisters.h"

class MISCClient {
	friend class VMXPower;
	friend class VMXTime;
	friend class VMXIO;
private:
	SPIClient& client;
	MISC_CAPABILITY_FLAGS cap_flags;
public:
	MISCClient(SPIClient& client);
	virtual ~MISCClient();

protected:

	bool get_capability_flags(MISC_CAPABILITY_FLAGS& value);

	/* Analog Input Configuration */
	bool GetAnalogInputFullScaleVoltage(float& full_scale_voltage);

	/* Analog Triggers */

	bool set_analog_trigger_mode(uint8_t analog_trigger_num, ANALOG_TRIGGER_MODE mode);
	bool get_analog_trigger_mode(uint8_t analog_trigger_num, ANALOG_TRIGGER_MODE& mode);

	bool get_analog_trigger_state(uint8_t analog_trigger_num, ANALOG_TRIGGER_STATE& state);

	bool get_analog_trigger_threshold_low(uint8_t analog_trigger_num, uint16_t& threshold);
	bool get_analog_trigger_threshold_high(uint8_t analog_trigger_num, uint16_t& threshold);
	bool set_analog_trigger_threshold_low(uint8_t analog_trigger_num, uint16_t threshold);
	bool set_analog_trigger_threshold_high(uint8_t analog_trigger_num, uint16_t threshold);

	/* Accumulators */

	bool get_accumulator_oversample_bits(uint8_t accumulator_num, uint8_t& oversample_bits);
	bool get_accumulator_average_bits(uint8_t accumulator_num, uint8_t& average_bits);
	bool set_accumulator_oversample_bits(uint8_t accumulator_num, uint8_t oversample_bits);
	bool set_accumulator_average_bits(uint8_t accumulator_num, uint8_t average_bits);

	bool get_accumulator_oversample_value(uint8_t accumulator_num, uint32_t& oversample_value);
	bool get_accumulator_input_average_value(uint8_t accumulator_num, uint32_t& oversample_value);

	bool get_accumulator_avg_voltage(uint8_t accumulator_num, float& avg_voltage);
	bool get_accumulator_instataneous_voltage(uint8_t accumulator_num, float& instantaneous_voltage);

	/* ExtPower */
	bool get_extpower_ctl_status(MISC_EXT_PWR_CTL_STATUS& value);
	bool get_extpower_voltage(float& ext_power_volts);
	bool get_extpower_cfg(MISC_EXT_PWR_CTL_CFG& extpower_cfg);
	bool set_extpower_cfg(MISC_EXT_PWR_CTL_CFG extpower_cfg);

	/* RTC */
	bool get_rtc_time(uint8_t& hours, uint8_t& minutes, uint8_t& seconds, uint32_t& subseconds);
	bool get_rtc_date(uint8_t& weekday, uint8_t& day, uint8_t& month, uint8_t& year);
	bool set_rtc_time(uint8_t hours, uint8_t minutes, uint8_t seconds);
	bool set_rtc_date(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year);
};

#endif /* MISCCLIENT_H_ */
