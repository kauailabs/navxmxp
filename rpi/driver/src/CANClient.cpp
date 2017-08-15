/*
 * CANClient.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#include <string.h>
#include <time.h> /* nanosleep() */
#include <mutex>

#include "CANClient.h"
#include "SPIClient.h"
#include "PIGPIOClient.h"
#include "NavXSPIMessageEx.h"

static std::mutex handler_mutex;

CANClient::CANClient(SPIClient& client_ref, PIGPIOClient& pigpio_ref) :
	client(client_ref),
	pigpio(pigpio_ref)
{
	can_notification.Init();
	pigpio.SetCANInterruptSink((ICANInterruptSink *)this);
	pigpio.EnableCANInterrupt();
	resources_released = false;
}

void CANClient::ReleaseResources()
{
	if (!resources_released) {
		resources_released = true;
		pigpio.DisableCANInterrupt();
		pigpio.SetCANInterruptSink(0);
		DeregisterNewRxDataNotifyHandler();
	}
}

CANClient::~CANClient() {
	ReleaseResources();
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

bool CANClient::get_bus_errors(CAN_ERROR_FLAGS & f, uint8_t& tx_error_count, uint8_t& rx_error_count,
		uint32_t& bus_off_count, uint32_t& tx_full_count) {
	struct __attribute__ ((__packed__)) {
		CAN_ERROR_FLAGS value1;
		uint8_t value2;
		uint8_t value3;
		uint32_t value4;
		uint32_t value5;
	} values;
	if(client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, bus_error_flags), values)){
		f = values.value1;
		tx_error_count = values.value2;
		rx_error_count = values.value3;
		bus_off_count = values.value4;
		tx_full_count = values.value5;
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

bool CANClient::get_rxbuffer_filter_mode(CANRXBufferID buffid, CAN_RX_FILTER_MODE& mode) {
	uint8_t mode_byte;
	bool success;
	if (buffid == RXBUFFER_0) {
		success = client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_filter_mode), mode_byte);
	} else {
		success = client.read(CAN_REGISTER_BANK, offsetof(struct CAN_REGS, rx_filter_mode) + sizeof(uint8_t), mode_byte);
	}
	mode = (CAN_RX_FILTER_MODE)mode_byte;
	return success;
}

bool CANClient::set_rxbuffer_filter_mode(CANRXBufferID buffid, CAN_RX_FILTER_MODE mode) {
	uint8_t mode_byte = (uint8_t)mode;
	if (buffid == RXBUFFER_0) {
		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, rx_filter_mode),
				mode_byte);
	} else {
		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, rx_filter_mode) + sizeof(uint8_t),
				mode_byte);
	}
}

bool CANClient::get_rxbuffer_accept_mask(CANRXBufferID buffid, CAN_ID& mask) {
	if (buffid == RXBUFFER_0) {
		return client.read(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_mask_rxb0),
				mask);
	} else {
		return client.read(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_mask_rxb1),
				mask);
	}
}

bool CANClient::set_rxbuffer_accept_mask(CANRXBufferID buffid, CAN_ID mask) {
	if (buffid == RXBUFFER_0) {
		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_mask_rxb0),
				mask);
	} else {
		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_mask_rxb1),
				mask);
	}
}

bool CANClient::get_rxbuffer_accept_filter(CANRXBufferID buffid, uint8_t id /* 0-1 or 0-3 */, CAN_ID& filter) {
	if (buffid == RXBUFFER_0) {
		if (id >= NUM_ACCEPT_FILTERS_RX0_BUFFER) return false;

		return client.read(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_filter_rxb0) + (id * sizeof(filter)),
				filter);
	} else {
		if (id >= NUM_ACCEPT_FILTERS_RX1_BUFFER) return false;

		return client.read(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_filter_rxb1) + (id * sizeof(filter)),
				filter);
	}
}

bool CANClient::set_rxbuffer_accept_filter(CANRXBufferID buffid, uint8_t id /* 0-1 */, CAN_ID filter) {
	if (buffid == RXBUFFER_0) {
		if (id >= NUM_ACCEPT_FILTERS_RX0_BUFFER) return false;

		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_filter_rxb0) + (id * sizeof(filter)),
				filter);
	} else {
		if (id >= NUM_ACCEPT_FILTERS_RX1_BUFFER) return false;

		return client.write(CAN_REGISTER_BANK,
				offsetof(struct CAN_REGS, accept_filter_rxb1) + (id * sizeof(filter)),
				filter);
	}
}

void CANClient::CANInterrupt(uint64_t curr_timestamp) {
	std::unique_lock<std::mutex> sync(handler_mutex);
	if (can_notification.handler_func) {
		can_notification.handler_func(can_notification.param, curr_timestamp);
	}
}

bool CANClient::RegisterNewRxDataNotifyHandler(CAN_NewRxDataNotifyHandler handler, void *param)
{
	{
		std::unique_lock<std::mutex> sync(handler_mutex);
		if (can_notification.handler_func) return false;
		if (!handler) return false;
		can_notification.handler_func = handler;
		can_notification.param = param;
	}
	CAN_IFX_INT_FLAGS can_int_flags;
	if (get_interrupt_enable(can_int_flags)) {
		can_int_flags.rx_fifo_nonempty = 1;
		if (set_interrupt_enable(can_int_flags)) {
			return true;
		}
	}

	return false;
}

bool CANClient::DeregisterNewRxDataNotifyHandler()
{
	{
		std::unique_lock<std::mutex> sync(handler_mutex);
		can_notification.Init();
	}
	CAN_IFX_INT_FLAGS can_int_flags;
	if (get_interrupt_enable(can_int_flags)) {
		can_int_flags.rx_fifo_nonempty = 0;
		if (set_interrupt_enable(can_int_flags)) {
			return true;
		}
	}

	return true;
}


