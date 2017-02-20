/*
 * IOCXClient.h
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#ifndef IOCXCLIENT_H_
#define IOCXCLIENT_H_

#include <stdint.h>
#include "SPIClient.h"
#include "IOCXRegisters.h"

class IOCXClient {

private:
	SPIClient& client;

public:
	IOCXClient(SPIClient& client);

	uint8_t get_num_gpios() { return IOCX_NUM_GPIOS; }
	uint8_t get_num_timers() { return IOCX_NUM_TIMERS; }
	uint8_t get_num_channels_per_timer() { return IOCX_NUM_CHANNELS_PER_TIMER; }
	uint8_t get_num_analog_inputs() { return IOCX_NUM_ANALOG_INPUTS; }

	bool get_gpio_config(int gpio_index, IOCX_GPIO_TYPE& type, IOCX_GPIO_INPUT& input, IOCX_GPIO_INTERRUPT& interrupt);
	bool set_gpio_config(int gpio_index, IOCX_GPIO_TYPE type, IOCX_GPIO_INPUT input, IOCX_GPIO_INTERRUPT interrupt);
	bool get_gpio(int gpio_index, bool& high);
	bool set_gpio(int gpio_index, bool high);
	bool get_capability_flags(uint16_t& value);
	bool get_interrupt_config(uint16_t& value);
	bool get_gpio_interrupt_status(uint16_t& value);
	bool get_external_power_voltage(uint16_t& value);
	bool get_timer_config(int timer_index, IOCX_TIMER_MODE& mode);
	bool set_timer_config(int timer_index, IOCX_TIMER_MODE mode);
	bool get_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET& mode);
	bool set_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET mode);
	bool get_timer_prescaler(int timer_index, uint16_t& value);
	bool set_timer_prescaler(int timer_index, uint16_t value);
	bool get_timer_aar(int timer_index, uint16_t& value);
	bool set_timer_aar(int timer_index, uint16_t value);
	bool get_timer_chx_ccr(int timer_index, int channel_index, uint16_t& value);
	bool set_timer_chx_ccr(int timer_index, int channel_index, uint16_t value);
	bool get_timer_status(int timer_index, uint8_t& value);
	bool get_timer_counter(int timer_index, uint16_t& value);
	bool get_ext_power_voltage(float& value);
	bool get_analog_input_voltage(int analog_input_index, float& value);

	virtual ~IOCXClient() {}
};

#endif /* IOCXCLIENT_H_ */
