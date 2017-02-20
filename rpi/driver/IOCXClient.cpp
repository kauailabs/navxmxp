/*
 * IOCXClient.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#include <cstdint>
#include "IMURegisters.h"
#include "IOCXRegisters.h"
#include "IOCXClient.h"

IOCXClient::IOCXClient(SPIClient& client_ref) :
	client(client_ref)
{
}

bool IOCXClient::get_gpio_config(int gpio_index, IOCX_GPIO_TYPE& type, IOCX_GPIO_INPUT& input, IOCX_GPIO_INTERRUPT& interrupt)
{
	if ( gpio_index > get_num_gpios()-1) return false;
	uint8_t value;
	if(client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, gpio_cfg) + (gpio_index * sizeof(value)), value)) {
		type = iocx_decode_gpio_type(&value);
		input = iocx_decode_gpio_input(&value);
		interrupt = iocx_decode_gpio_interrupt(&value);
		return true;
	}
	return false;
}

bool IOCXClient::set_gpio_config(int gpio_index, IOCX_GPIO_TYPE type, IOCX_GPIO_INPUT input, IOCX_GPIO_INTERRUPT interrupt)
{
	if ( gpio_index > get_num_gpios()-1) return false;
	uint8_t value = 0;
	iocx_encode_gpio_type(&value, type);
	iocx_encode_gpio_input(&value, input);
	iocx_encode_gpio_interrupt(&value, interrupt);
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, gpio_cfg) + (gpio_index * sizeof(value)),
			value);
}

bool IOCXClient::get_gpio(int gpio_index, bool& high)
{
	if ( gpio_index > get_num_gpios()-1) return false;
	uint8_t value;
	if(client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, gpio_data) + (gpio_index * sizeof(value)), value)) {
		high = (value != IOCX_GPIO_RESET);
		return true;
	}
	return false;
}

bool IOCXClient::set_gpio(int gpio_index, bool high)
{
	if ( gpio_index > get_num_gpios()-1) return false;
	uint8_t reg_value = high ? IOCX_GPIO_SET : IOCX_GPIO_RESET;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, gpio_data) + (gpio_index * sizeof(reg_value)),
			reg_value);
}

bool IOCXClient::get_capability_flags(uint16_t& value)
{
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, capability_flags), value);
}

bool IOCXClient::get_interrupt_config(uint16_t& value)
{
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, int_cfg), value);
}

bool IOCXClient::get_gpio_interrupt_status(uint16_t& value)
{
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, gpio_intstat), value);
}

bool IOCXClient::get_timer_config(int timer_index, IOCX_TIMER_MODE& mode)
{
	if ( timer_index > get_num_timers()-1) return false;
	uint8_t value;
	if(client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_cfg) + (timer_index * sizeof(value)), value)) {
		mode = iocx_decode_timer_mode(&value);
		return true;
	}
	return false;
}

bool IOCXClient::set_timer_config(int timer_index, IOCX_TIMER_MODE mode)
{
	if ( timer_index > get_num_timers()-1) return false;
	uint8_t value = 0;
	iocx_encode_timer_mode(&value, mode);
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_cfg) + (timer_index * sizeof(value)),
			value);
}

bool IOCXClient::get_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET& reset)
{
	if ( timer_index > get_num_timers()-1) return false;
	uint8_t value;
	if(client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_ctl) + (timer_index * sizeof(value)), value)) {
		reset = iocx_decode_timer_counter_reset(&value);
		return true;
	}
	return false;
}

bool IOCXClient::set_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET reset)
{
	if ( timer_index > get_num_timers()-1) return false;
	uint8_t value = 0;
	iocx_encode_timer_counter_reset(&value, reset);
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_ctl) + (timer_index * sizeof(value)),
			value);
}

bool IOCXClient::get_timer_prescaler(int timer_index, uint16_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_ctl) + (timer_index * sizeof(value)), value);
}

bool IOCXClient::set_timer_prescaler(int timer_index, uint16_t value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_prescaler) + (timer_index * sizeof(value)),
			value);
}

bool IOCXClient::get_timer_aar(int timer_index, uint16_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_aar) + (timer_index * sizeof(value)), value);
}

bool IOCXClient::set_timer_aar(int timer_index, uint16_t value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_aar) + (timer_index * sizeof(value)),
			value);
}

bool IOCXClient::get_timer_chx_ccr(int timer_index, int channel_index, uint16_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	if ( timer_index > get_num_channels_per_timer()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_aar) +
			(timer_index * get_num_channels_per_timer() * sizeof(value)) +
			(channel_index * sizeof(value)), value);
}

bool IOCXClient::set_timer_chx_ccr(int timer_index, int channel_index, uint16_t value)
{
	if ( timer_index > get_num_timers()-1) return false;
	if ( timer_index > get_num_channels_per_timer()-1) return false;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_aar) +
			(timer_index * get_num_channels_per_timer() * sizeof(value)) +
			(channel_index * sizeof(value)), value);
}

bool IOCXClient::get_timer_status(int timer_index, uint8_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_status) + (timer_index * sizeof(value)), value);
}

bool IOCXClient::get_timer_counter(int timer_index, uint16_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_counter) + (timer_index * sizeof(value)), value);
}

bool IOCXClient::get_ext_power_voltage(float& value)
{
	uint16_t signed_thousandths;
	bool success = client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, ext_pwr_voltage), signed_thousandths);
	if (success){
		value = IMURegisters::decodeProtocolSignedThousandthsFloat((char *)&signed_thousandths);
	}
	return success;
}

bool IOCXClient::get_analog_input_voltage(int analog_input_index, float& value)
{
	uint16_t signed_thousandths;
	bool success = client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, analog_in_voltage) +
			(analog_input_index * sizeof(signed_thousandths)), signed_thousandths);
	if (success){
		value = IMURegisters::decodeProtocolSignedThousandthsFloat((char *)&signed_thousandths);
	}
	return success;
}
