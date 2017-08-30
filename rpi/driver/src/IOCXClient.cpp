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

typedef struct {
	uint8_t timer_number;
	uint8_t timer_channel_number;
} VMXTimerMap;

/* Timer map is indexed by VMX GPIO Number */
VMXTimerMap timer_map[] = {
		{ 5, 2 },
		{ 5, 3 },
		{ 9, 0 },
		{ 9, 1 },
		{ 0, 0 },
		{ 0, 1 },
		{ 1, 0 },
		{ 1, 1 },
		{ 2, 0 },
		{ 2, 1 },
		{ 3, 0 },
		{ 3, 1 }
};

IOCXClient::IOCXClient(SPIClient& client_ref) :
	client(client_ref)
{
}

bool IOCXClient::get_timer_and_timer_channel_number_for_gpio_number(uint8_t gpio_number,
		uint8_t& timer_number, uint8_t& timer_channel_number)
{
	if ( gpio_number < (sizeof(timer_map)/sizeof(timer_map[0]))) {
		timer_number = timer_map[gpio_number].timer_number;
		timer_channel_number = timer_map[gpio_number].timer_channel_number;
		return true;
	}
	return false;
}

bool IOCXClient::get_gpio_config(int gpio_index, IOCX_GPIO_TYPE& type, IOCX_GPIO_INPUT& input, IOCX_GPIO_INTERRUPT& interrupt)
{
	if ( gpio_index > GetNumGpios()-1) return false;
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
	if ( gpio_index > GetNumGpios()-1) return false;
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
	if ( gpio_index > GetNumGpios()-1) return false;
	uint8_t value;
	if(client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, gpio_data) + (gpio_index * sizeof(value)), value)) {
		high = (value != IOCX_GPIO_RESET);
		return true;
	}
	return false;
}

bool IOCXClient::set_gpio(int gpio_index, bool high)
{
	if ( gpio_index > GetNumGpios()-1) return false;
	uint8_t reg_value = high ? IOCX_GPIO_SET : IOCX_GPIO_RESET;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, gpio_data) + (gpio_index * sizeof(reg_value)),
			reg_value);
}

bool IOCXClient::get_capability_flags(IOCX_CAPABILITY_FLAGS& value)
{
	uint16_t cap_flags;
	if (client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, capability_flags), cap_flags)) {
		value = *((IOCX_CAPABILITY_FLAGS *)&cap_flags);
		return true;
	}
	return false;
}

bool IOCXClient::get_interrupt_config(uint16_t& value)
{
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, int_cfg), value);
}

bool IOCXClient::set_interrupt_config(uint16_t value)
{
	return client.write(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, int_cfg), value);
}

bool IOCXClient::get_gpio_interrupt_status(uint16_t& interrupt_flags, uint16_t& last_interrupt_edge) {

	typedef struct {
		uint16_t int_flags;
		uint16_t last_int_edge;
	} AllInterruptStatus;

	AllInterruptStatus all_int_status;

	if (client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, gpio_intstat), all_int_status)) {
		interrupt_flags = all_int_status.int_flags;
		last_interrupt_edge = all_int_status.last_int_edge;
		return true;
	}
	return false;
}

bool IOCXClient::set_gpio_interrupt_status(uint16_t interrupt_flags_to_clear) {

	return (client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, gpio_intstat),
			interrupt_flags_to_clear));
}

bool IOCXClient::get_timer_config(int timer_index, uint8_t& mode)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_cfg) + (timer_index * sizeof(mode)), mode);

}

bool IOCXClient::set_timer_config(int timer_index, uint8_t mode)
{
	if (timer_index > get_num_timers()-1) return false;
	return client.write(IOCX_REGISTER_BANK,
			offsetof(struct IOCX_REGS, timer_cfg) + (timer_index * sizeof(mode)),
			mode);
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
	if ( channel_index > get_num_channels_per_timer()-1) return false;
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

bool IOCXClient::get_timer_counter(int timer_index, int32_t& value)
{
	if ( timer_index > get_num_timers()-1) return false;
	return client.read(IOCX_REGISTER_BANK, offsetof(struct IOCX_REGS, timer_counter) + (timer_index * sizeof(value)), value);
}

