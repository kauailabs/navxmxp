/*
 * CANClient.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#include "CANClient.h"
#include <string.h>
#include <time.h> /* nanosleep() */

CANClient::CANClient(SPIClient& client_ref) :
	client(client_ref) {
}

CANClient::~CANClient() {
}

bool CANClient::get_capability_flags(CAN_CAPABILITY_FLAGS& f) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, capability_flags), f);
}

bool CANClient::get_interrupt_status(CAN_IFX_INT_FLAGS & f, uint8_t& rx_fifo_available_count) {
	struct __attribute__ ((__packed__)) {
		CAN_IFX_INT_FLAGS value1;
		uint8_t value2;
	} values;
	if(client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, int_status), values)){
		f = values.value1;
		rx_fifo_available_count = values.value2;
		return true;
	}
	return false;
}

bool CANClient::get_interrupt_enable(CAN_IFX_INT_FLAGS & f) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, int_enable), f);
}

bool CANClient::set_interrupt_enable(CAN_IFX_INT_FLAGS interrupts_to_enable) {
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, int_enable),
			interrupts_to_enable);
}

bool CANClient::clear_interrupt_flags(CAN_IFX_INT_FLAGS flags_to_clear) {
	/* Todo:  Implement BitModify SPI Command on SPI interface */
	return true;
}

bool CANClient::get_receive_fifo_entry_count(uint8_t& count) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_fifo_entry_count), count);
}

bool CANClient::get_receive_data(CAN_TRANSFER *p_transfer, int n_transfers) {
	uint8_t bytes_to_transfer = n_transfers * sizeof(CAN_TRANSFER);
	if ( bytes_to_transfer > 254) return false;
	uint8_t transfer_buffer[255];

	NavXSPIMessage msg(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_fifo_tail), bytes_to_transfer);
	if (client.read(msg, transfer_buffer, bytes_to_transfer+1 /* Add 1 for CRC */ )) {
		memcpy((uint8_t *)p_transfer, transfer_buffer, bytes_to_transfer);
		return true;
	}
	return false;
}

bool CANClient::get_transmit_fifo_entry_count(uint8_t& count) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, tx_fifo_entry_count), count);
}

bool CANClient::enqueue_transmit_data(CAN_TRANSFER *p_tx_data) {
	NavXSPIMessage msg(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, tx_fifo_head), sizeof(*p_tx_data), (uint8_t *)p_tx_data);
	return client.write(msg);
}

bool CANClient::get_bus_errors(CAN_ERROR_FLAGS & f, uint8_t& tx_error_count, uint8_t rx_error_count) {
	struct __attribute__ ((__packed__)) {
		CAN_ERROR_FLAGS value1;
		uint8_t value2;
		uint8_t value3;
	} values;
	if(client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, bus_error_flags), values)){
		f = values.value1;
		tx_error_count = values.value2;
		rx_error_count = values.value3;
		return true;
	}
	return false;
}

bool CANClient::get_mode(CAN_MODE& mode) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, opmode), mode);
}

bool CANClient::set_mode(CAN_MODE mode) {
	bool success = client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, opmode),
			mode);

	/* Note:  This command may take several milliseconds      */
	/* to complete.  To help avoid communication errors,      */
	/* delay approximately 10 ms after invoking this command. */
	/* Todo:  This delay should occur within the SPIClient,   */
	/* within the context of the communication mutex - in     */
	/* order to ensure that communication in other threads    */
	/* are not impacted by this case.                         */

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 10000000;
	nanosleep(&ts, NULL);

	return success;
}

bool CANClient::reset() {
	uint8_t value = 1;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, reset),
			value);
}

bool CANClient::get_rxb0_accept_mask(CAN_ID& mask) {
	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb0),
			mask);
}

bool CANClient::set_rxb0_accept_mask(CAN_ID& mask) {
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb0),
			mask);
}

bool CANClient::get_rxb1_accept_mask(CAN_ID& mask) {
	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb1),
			mask);
}

bool CANClient::set_rxb1_accept_mask(CAN_ID& mask) {
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb1),
			mask);
}

bool CANClient::get_rxb0_accept_filter(uint8_t id /* 0-1 */, CAN_ID& filter) {
	if (id >= NUM_ACCEPT_FILTERS_RX0_BUFFER) return false;

	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_filter_rxb0) + (id * sizeof(filter)),
			filter);
}

bool CANClient::set_rxb0_accept_filter(uint8_t id /* 0-1 */, CAN_ID& filter) {
	if (id >= NUM_ACCEPT_FILTERS_RX0_BUFFER) return false;

	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_filter_rxb0) + (id * sizeof(filter)),
			filter);
}

bool CANClient::get_rxb1_accept_filter(uint8_t id /* 0-3 */, CAN_ID& filter) {
	if (id >= NUM_ACCEPT_FILTERS_RX1_BUFFER) return false;

	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_filter_rxb1) + (id * sizeof(filter)),
			filter);
}

bool CANClient::set_rxb1_accept_filter(uint8_t id /* 0-3 */, CAN_ID& filter) {
	if (id >= NUM_ACCEPT_FILTERS_RX1_BUFFER) return false;

	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_filter_rxb1) + (id * sizeof(filter)),
			filter);
}
