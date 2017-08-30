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

#define INVALID_IOCX_TIMER_NUMBER 0xFF

class IOCXClient {

private:
	SPIClient& client;

public:
	IOCXClient(SPIClient& client);

	bool get_timer_and_timer_channel_number_for_gpio_number(uint8_t gpio_number,
			uint8_t& timer_number, uint8_t& timer_channel_number);

	uint8_t GetNumGpios() { return IOCX_NUM_GPIOS; }
	uint8_t get_num_timers() { return IOCX_NUM_TIMERS; }
	uint8_t get_num_channels_per_timer() { return IOCX_NUM_CHANNELS_PER_TIMER; }
	uint8_t get_num_pwm_channels() { return get_num_timers() * get_num_channels_per_timer(); }
	uint8_t get_num_interrupts() { return IOCX_NUM_INTERRUPTS; }

	bool get_gpio_config(int gpio_index, IOCX_GPIO_TYPE& type, IOCX_GPIO_INPUT& input, IOCX_GPIO_INTERRUPT& interrupt);
	bool set_gpio_config(int gpio_index, IOCX_GPIO_TYPE type, IOCX_GPIO_INPUT input, IOCX_GPIO_INTERRUPT interrupt);
	bool get_gpio(int gpio_index, bool& high);
	bool set_gpio(int gpio_index, bool high);
	bool get_capability_flags(IOCX_CAPABILITY_FLAGS& value);
	bool get_interrupt_config(uint16_t& value);
	bool set_interrupt_config(uint16_t value);
	bool get_gpio_interrupt_status(uint16_t& interrupt_flags, uint16_t& last_interrupt_edge);
	bool set_gpio_interrupt_status(uint16_t interrupt_flags_to_clear);
	bool get_timer_config(int timer_index, uint8_t& mode);
	bool set_timer_config(int timer_index, uint8_t mode);
	bool get_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET& mode);
	bool set_timer_control(int timer_index, IOCX_TIMER_COUNTER_RESET mode);
	bool get_timer_prescaler(int timer_index, uint16_t& value);
	bool set_timer_prescaler(int timer_index, uint16_t value);
	bool get_timer_aar(int timer_index, uint16_t& value);
	bool set_timer_aar(int timer_index, uint16_t value);
	bool get_timer_chx_ccr(int timer_index, int channel_index, uint16_t& value);
	bool set_timer_chx_ccr(int timer_index, int channel_index, uint16_t value);
	bool get_timer_status(int timer_index, uint8_t& value);
	bool get_timer_counter(int timer_index, int32_t& value);

	virtual ~IOCXClient() {}
};

#endif /* IOCXCLIENT_H_ */
