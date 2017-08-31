/*
 * CANClient.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef CANCLIENT_H_
#define CANCLIENT_H_

#include <stdint.h>

#include "IPIGPIOInterruptSinks.h"
#include "VMXHandlers.h"
class SPIClient;
class PIGPIOClient;

#include "CANRegisters.h"

class CANClient : public ICANInterruptSink {

	SPIClient& client;
	PIGPIOClient& pigpio;
	bool resources_released;

	typedef struct {
		CAN_NewRxDataNotifyHandler handler_func;
		void *param;
		void Init() {
			handler_func = 0;
			param = 0;
		}
	} CANNotificationInfo;

	CANNotificationInfo can_notification;

	void CANInterrupt(uint64_t curr_timestamp);

public:

	typedef enum { RXBUFFER_0 = 0, RXBUFFER_1 = 1 } CANRXBufferID;

	CANClient(SPIClient& client_ref, PIGPIOClient& pigpio_ref);
	void ReleaseResources();
	virtual ~CANClient();

	bool RegisterNewRxDataNotifyHandler(CAN_NewRxDataNotifyHandler handler, void *param);
	bool DeregisterNewRxDataNotifyHandler();

	bool get_capability_flags(CAN_CAPABILITY_FLAGS& f);

	bool get_interrupt_status(CAN_IFX_INT_FLAGS & f, uint8_t& rx_fifo_available_count);
	bool get_interrupt_enable(CAN_IFX_INT_FLAGS & f);
	bool set_interrupt_enable(CAN_IFX_INT_FLAGS interrupts_to_enable);
	bool clear_interrupt_flags(CAN_IFX_INT_FLAGS flags_to_clear);

	bool get_receive_fifo_entry_count(uint8_t& count);
	bool get_receive_data(TIMESTAMPED_CAN_TRANSFER *p_transfer, int n_transfers, uint8_t& remaining_transfer_count);

	bool get_transmit_fifo_entry_count(uint8_t& count);
	bool enqueue_transmit_data(CAN_TRANSFER *p_tx_data);

	bool get_bus_errors(CAN_ERROR_FLAGS & f, uint8_t& tx_error_count, uint8_t& rx_error_count,
			uint32_t& bus_off_count, uint32_t& tx_full_count);

	bool get_mode(CAN_MODE& mode);
	bool set_mode(CAN_MODE mode);

	bool reset();
	bool flush_rx_fifo();
	bool flush_tx_fifo();

	bool get_rxbuffer_filter_mode(CANRXBufferID buffid, CAN_RX_FILTER_MODE& mode);
	bool set_rxbuffer_filter_mode(CANRXBufferID buffid, CAN_RX_FILTER_MODE mode);
	bool get_rxbuffer_accept_mask(CANRXBufferID buffid, CAN_ID& mask);
	bool set_rxbuffer_accept_mask(CANRXBufferID buffid, CAN_ID mask);
	bool get_rxbuffer_accept_filter(CANRXBufferID buffid, uint8_t filter_id /* 0-1 for first, 0-3 for second */, CAN_ID& filter);
	bool set_rxbuffer_accept_filter(CANRXBufferID buffid, uint8_t filter_id /* 0-1 for first, 0-3 for second */, CAN_ID filter);
};

#endif /* CANCLIENT_H_ */
