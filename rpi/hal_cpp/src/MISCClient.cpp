/*
 * MISCClient.cpp
 *
 *  Created on: 15 Jul 2017
 *      Author: pi
 */

#include "MISCClient.h"
#include "MISCRegisters.h"

MISCClient::MISCClient(SPIClient& client_ref) :
	client(client_ref)
{
	get_capability_flags(cap_flags);
}

MISCClient::~MISCClient() {
	// TODO Auto-generated destructor stub
}

bool MISCClient::get_capability_flags(MISC_CAPABILITY_FLAGS& value)
{
	uint16_t cap_flags;
	if (client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, capability_flags), cap_flags)) {
		value = *((MISC_CAPABILITY_FLAGS *)&cap_flags);
		return true;
	}
	return false;
}

bool MISCClient::GetAnalogInputFullScaleVoltage(float& full_scale_voltage) {
	MISC_CAPABILITY_FLAGS cap_flags;
	if (get_capability_flags(cap_flags)) {
		full_scale_voltage = (cap_flags.an_in_5V ? 5.0f : 3.3f);
		return true;
	}
	return false;
}

bool MISCClient::get_extpower_ctl_status(MISC_EXT_PWR_CTL_STATUS& status)
{
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, ext_pwr_ctl_status), status);
}

const float vmxpi_extpower_in_vdiv_ratio = (3.24f / (11.5f + 3.24f));

bool MISCClient::get_extpower_voltage(float& ext_power_volts)
{
	uint16_t value;
	if(client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, ext_pwr_voltage_value), value)) {
		/* Convert to voltage (reversing effects of VMX-pi ext power voltage divider. */
		ext_power_volts = (((float)value) / 4096) * 3.3f;
		ext_power_volts *= (1.0f/vmxpi_extpower_in_vdiv_ratio);
		return true;
	}
	return false;
}

bool MISCClient::get_extpower_cfg(MISC_EXT_PWR_CTL_CFG& extpower_cfg)
{
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, ext_pwr_ctl_cfg), extpower_cfg);
}

bool MISCClient::set_extpower_cfg(MISC_EXT_PWR_CTL_CFG extpower_cfg)
{
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, ext_pwr_ctl_cfg),
			extpower_cfg);
}


bool MISCClient::get_rtc_time(uint8_t& hours, uint8_t& minutes, uint8_t& seconds, uint32_t& subseconds)
{
	MISC_RTC_TIME rtc_time;
	if(client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, rtc_time), rtc_time)){
		hours = rtc_time.hours;
		minutes = rtc_time.minutes;
		seconds = rtc_time.seconds;
		subseconds = rtc_time.subseconds;
		return true;
	}
	return false;
}

bool MISCClient::get_rtc_date(uint8_t& weekday, uint8_t& day, uint8_t& month, uint8_t& year)
{
	MISC_RTC_DATE rtc_date;
	if(client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, rtc_date), rtc_date)){
		weekday = rtc_date.weekday;
		day = rtc_date.date;
		month = rtc_date.month;
		year = rtc_date.year;
		return true;
	}
	return false;
}

bool MISCClient::set_rtc_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	typedef struct __attribute__ ((__packed__)) {
		uint8_t seconds;
		uint8_t minutes;
		uint8_t hours;
	} RTC_TIME_NO_SUBSECONDS;
	RTC_TIME_NO_SUBSECONDS rtc_time_no_subseconds;
	rtc_time_no_subseconds.hours = hours;
	rtc_time_no_subseconds.minutes = minutes;
	rtc_time_no_subseconds.seconds = seconds;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, rtc_time_set.seconds),
			rtc_time_no_subseconds);
}

bool MISCClient::set_rtc_date(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year)
{
	MISC_RTC_DATE rtc_date;
	rtc_date.weekday = weekday;
	rtc_date.date = day;
	rtc_date.month = month;
	rtc_date.year = year;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, rtc_date_set),
			rtc_date);
}

bool MISCClient::get_rtc_cfg(MISC_RTC_CFG& rtc_cfg)
{
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, rtc_cfg), rtc_cfg);
}

bool MISCClient::set_rtc_cfg(MISC_RTC_CFG rtc_cfg)
{
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, rtc_cfg),
			rtc_cfg);
}

bool MISCClient::set_analog_trigger_mode(uint8_t analog_trigger_num, ANALOG_TRIGGER_MODE mode)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	uint8_t value = uint8_t(mode);
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_trigger_cfg) + (analog_trigger_num * sizeof(value)),
			value);
}

bool MISCClient::get_analog_trigger_mode(uint8_t analog_trigger_num, ANALOG_TRIGGER_MODE& mode)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	uint8_t value;
	if(client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_trigger_cfg) + (analog_trigger_num * sizeof(value)), value)) {
		mode = (ANALOG_TRIGGER_MODE)value;
		return true;
	}
	return false;
}

bool MISCClient::get_analog_trigger_state(uint8_t analog_trigger_num, ANALOG_TRIGGER_STATE& state)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	uint8_t value = uint8_t(state);
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_trigger_state) + (analog_trigger_num * sizeof(value)),
			value);
}

bool MISCClient::get_analog_trigger_threshold_low(uint8_t analog_trigger_num, uint16_t& threshold)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_trigger_threshold_low) + (analog_trigger_num * sizeof(threshold)), threshold);
}

bool MISCClient::get_analog_trigger_threshold_high(uint8_t analog_trigger_num, uint16_t& threshold)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_trigger_threshold_high) + (analog_trigger_num * sizeof(threshold)), threshold);
}

bool MISCClient::set_analog_trigger_threshold_low(uint8_t analog_trigger_num, uint16_t threshold)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_trigger_threshold_low) + (analog_trigger_num * sizeof(threshold)),
			threshold);
}

bool MISCClient::set_analog_trigger_threshold_high(uint8_t analog_trigger_num, uint16_t threshold)
{
	if ( analog_trigger_num > MISC_NUM_ANALOG_TRIGGERS-1) return false;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_trigger_threshold_high) + (analog_trigger_num * sizeof(threshold)),
			threshold);
}

/* Accumulators */

bool MISCClient::get_accumulator_oversample_bits(uint8_t accumulator_num, uint8_t& oversample_bits)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_input_oversample_bits) + (accumulator_num * sizeof(oversample_bits)), oversample_bits);
}

bool MISCClient::get_accumulator_average_bits(uint8_t accumulator_num, uint8_t& average_bits)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_input_average_bits) + (accumulator_num * sizeof(average_bits)), average_bits);
}

bool MISCClient::set_accumulator_oversample_bits(uint8_t accumulator_num, uint8_t oversample_bits)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_input_oversample_bits) + (accumulator_num * sizeof(oversample_bits)),
			oversample_bits);
}

bool MISCClient::set_accumulator_average_bits(uint8_t accumulator_num, uint8_t average_bits)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.write(MISC_REGISTER_BANK,
			offsetof(struct MISC_REGS, analog_input_average_bits) + (accumulator_num * sizeof(average_bits)),
			average_bits);
}

bool MISCClient::get_accumulator_oversample_value(uint8_t accumulator_num, uint32_t& oversample_value)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_input_oversample_value) + (accumulator_num * sizeof(oversample_value)), oversample_value);
}

bool MISCClient::get_accumulator_input_average_value(uint8_t accumulator_num, uint32_t& average_value)
{
	if ( accumulator_num > MISC_NUM_ANALOG_INPUTS-1) return false;
	return client.read(MISC_REGISTER_BANK, offsetof(struct MISC_REGS, analog_input_average_value) + (accumulator_num * sizeof(average_value)), average_value);
}

static const float adc_count_to_voltage_conversion_factor = 3.3f / 4096;

bool MISCClient::get_accumulator_avg_voltage(uint8_t accumulator_num, float& avg_voltage)
{
	uint32_t average;
	if (get_accumulator_input_average_value(accumulator_num, average)) {
		if (cap_flags.an_in_5V) {
			/* Reverse effects of the vmx-pi 2/3 voltage divider */
			average += (average << 1);	/* Multiply by 3 */
			average >>= 1;				/* Divide by 2 */
		}
		avg_voltage = adc_count_to_voltage_conversion_factor * average;
		return true;
	}
	return false;
}

bool MISCClient::get_accumulator_instataneous_voltage(uint8_t accumulator_num, float& instantaneous_voltage)
{
	return get_accumulator_avg_voltage(accumulator_num, instantaneous_voltage);
}
