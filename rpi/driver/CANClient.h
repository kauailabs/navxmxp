/*
 * CANClient.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef CANCLIENT_H_
#define CANCLIENT_H_

#include <stdint.h>

#include "CANRegisters.h"
#include "SPIClient.h"

class CANClient {
private:
	SPIClient& client;
public:
	CANClient(SPIClient& client);
	virtual ~CANClient();

	bool get_capability_flags(CAN_CAPABILITY_FLAGS& f);

	bool get_interrupt_status(CAN_IFX_INT_FLAGS & f, uint8_t& rx_fifo_available_count);
	bool get_interrupt_enable(CAN_IFX_INT_FLAGS & f);
	bool set_interrupt_enable(CAN_IFX_INT_FLAGS interrupts_to_enable);
	bool clear_interrupt_flags(CAN_IFX_INT_FLAGS flags_to_clear);

	bool get_receive_fifo_entry_count(uint8_t& count);
	bool get_receive_data(CAN_TRANSFER *p_transfer, int n_transfers);

	bool get_transmit_fifo_entry_count(uint8_t& count);
	bool enqueue_transmit_data(CAN_TRANSFER *p_tx_data);

	bool get_bus_errors(CAN_ERROR_FLAGS & f, uint8_t& tx_error_count, uint8_t rx_error_count);

	bool get_mode(CAN_MODE& mode);
	bool set_mode(CAN_MODE mode);

	bool reset();

	bool get_rxb0_accept_mask(CAN_ID& mask);
	bool set_rxb0_accept_mask(CAN_ID& mask);
	bool get_rxb1_accept_mask(CAN_ID& mask);
	bool set_rxb1_accept_mask(CAN_ID& mask);
	bool get_rxb0_accept_filter(uint8_t id /* 0-1 */, CAN_ID& filter);
	bool set_rxb0_accept_filter(uint8_t id /* 0-1 */, CAN_ID& filter);
	bool get_rxb1_accept_filter(uint8_t id /* 0-3 */, CAN_ID& filter);
	bool set_rxb1_accept_filter(uint8_t id /* 0-3 */, CAN_ID& filter);
};

#endif /* CANCLIENT_H_ */
