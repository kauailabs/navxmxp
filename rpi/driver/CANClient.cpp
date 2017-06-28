/*
 * CANClient.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#include "CANClient.h"
#include <string.h>
#include <time.h> /* nanosleep() */
#include "NavXSPIMessageEx.h"

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
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, int_flags),
			flags_to_clear);
}

bool CANClient::get_receive_fifo_entry_count(uint8_t& count) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_fifo_entry_count), count);
}

bool CANClient::get_receive_data(TIMESTAMPED_CAN_TRANSFER *p_transfer, int n_transfers, uint8_t& remaining_transfer_count) {
	uint8_t bytes_to_transfer = n_transfers * sizeof(TIMESTAMPED_CAN_TRANSFER);
	if ( bytes_to_transfer > 253) return false; /* Max xfer size 255, w/space for 2 bytes (xfr count, CRC */
	uint8_t transfer_buffer[255];

	NavXSPIMessage msg(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_fifo_tail), bytes_to_transfer+1 /* 1 for bytes-to-transfer */);
	if (client.read(msg, transfer_buffer, bytes_to_transfer+2 /* Add 1 for bytes-to-transfer count, and 1 for CRC */ )) {
		memcpy((uint8_t *)p_transfer, transfer_buffer, bytes_to_transfer);
		remaining_transfer_count = transfer_buffer[bytes_to_transfer];
		return true;
	}
	return false;
}

bool CANClient::get_transmit_fifo_entry_count(uint8_t& count) {
	return client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, tx_fifo_entry_count), count);
}

bool CANClient::enqueue_transmit_data(CAN_TRANSFER *p_tx_data) {
	NavXSPIMessageEx msg(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, tx_fifo_head), sizeof(*p_tx_data), (uint8_t *)p_tx_data);
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
	uint8_t mode_byte;
	bool success = client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, opmode), mode_byte);
	mode = (CAN_MODE)mode_byte;
	return success;
}

bool CANClient::set_mode(CAN_MODE mode) {
	/* Note:  This command may take several milliseconds      */
	/* to complete.  To help avoid communication errors,      */
	/* the SPIClient waits for a signal that the requested    */
	/* action is complete.                                    */

	uint8_t mode_byte = (uint8_t)mode;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, opmode),
			mode_byte);
}

bool CANClient::reset() {
	uint8_t value = CAN_CMD_RESET;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, command),
			value);
}

bool CANClient::flush_rx_fifo() {
	uint8_t value = CAN_CMD_FLUSH_RXFIFO;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, command),
			value);
}

bool CANClient::flush_tx_fifo() {
	uint8_t value = CAN_CMD_FLUSH_TXFIFO;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, command),
			value);
}

bool CANClient::get_rxb0_filter_mode(CAN_RX_FILTER_MODE& mode) {
	uint8_t mode_byte;
	bool success = client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_filter_mode), mode_byte);
	mode = (CAN_RX_FILTER_MODE)mode_byte;
	return success;
}

bool CANClient::set_rxb0_filter_mode(CAN_RX_FILTER_MODE mode) {
	uint8_t mode_byte = (uint8_t)mode;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, rx_filter_mode),
			mode_byte);
}

bool CANClient::get_rxb1_filter_mode(CAN_RX_FILTER_MODE& mode) {
	uint8_t mode_byte;
	bool success = client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_filter_mode) + sizeof(uint8_t), mode_byte);
	mode = (CAN_RX_FILTER_MODE)mode_byte;
	return success;
}

bool CANClient::set_rxb1_filter_mode(CAN_RX_FILTER_MODE mode) {
	uint8_t mode_byte = (uint8_t)mode;
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, rx_filter_mode) + sizeof(uint8_t),
			mode_byte);
}


bool CANClient::get_rxb0_accept_mask(CAN_ID& mask) {
	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb0),
			mask);
}

bool CANClient::set_rxb0_accept_mask(CAN_ID mask) {
	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb0),
			mask);
}

bool CANClient::get_rxb1_accept_mask(CAN_ID& mask) {
	return client.read(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_mask_rxb1),
			mask);
}

bool CANClient::set_rxb1_accept_mask(CAN_ID mask) {
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

bool CANClient::set_rxb0_accept_filter(uint8_t id /* 0-1 */, CAN_ID filter) {
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

bool CANClient::set_rxb1_accept_filter(uint8_t id /* 0-3 */, CAN_ID filter) {
	if (id >= NUM_ACCEPT_FILTERS_RX1_BUFFER) return false;

	return client.write(CAN_REGISTER_BANK,
			offsetof(struct CAN_REGS, accept_filter_rxb1) + (id * sizeof(filter)),
			filter);
}
