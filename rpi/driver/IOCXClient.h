/*
 * IOCXClient.h
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#ifndef IOCXCLIENT_H_
#define IOCXCLIENT_H_

#include <stdint.h>
#include "IOCXRegisters.h"
#include "NavXSPIMessage.h"

class IOCXClient {

protected:
	virtual bool transmit(uint8_t *p_data, uint8_t len){return false;}
	virtual bool transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len) { return false; }
	bool write(NavXSPIMessage& write);
	bool read(NavXSPIMessage& request, uint8_t *p_response, uint8_t response_len);
	template<typename T> bool read(uint8_t bank, uint8_t offset, T& response);
	template<typename T> bool write(uint8_t bank, uint8_t offset, T value);

public:
	IOCXClient();

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
