/*
 * VMXCAN.cpp
 *
 *  Created on: 1 Aug 2017
 *      Author: pi
 */

#include "VMXCAN.h"
#include "VMXErrors.h"
#include "CANClient.h"
#include "VMXCANReceiveStreamManager.h"
#include "Logging.h"
#include <stdio.h>
#include <inttypes.h>
#include <mutex>
#include <condition_variable>

std::mutex isr_mtx;
std::condition_variable cv;

VMXCAN::VMXCAN(CANClient& can_ref, VMXTime& time_ref) :
	can(can_ref),
	time(time_ref)
{
	p_stream_mgr = new VMXCANReceiveStreamManager();
	task_done = false;
	task = new thread(VMXCAN::ThreadFunc, this);
	Reset();
}

int VMXCAN::ThreadFunc(VMXCAN *p_this)
{
	std::unique_lock<std::mutex> lck(isr_mtx);
	while (!p_this->task_done) {
		try {
			cv.wait_for(lck,std::chrono::milliseconds(20));
			/* This can return due to the condition variable being set, or due to timeout. */
			if (!p_this->task_done) {
				/* Process data - the data ready condition occurred. */
				p_this->RetrieveAllCANData();
			}
		} catch(const std::exception& ex){
			printf("VMXCAN::ThreadFunc() - Caught exception:  %s\n", ex.what());
		}
	}
    return 0;
}

void VMXCAN::ReleaseResources()
{
	can.DeregisterNewRxDataNotifyHandler();
	if (!task_done) {
		task_done = true;
		if (task) {
			try {
				if(task->joinable()) {
					task->join();
				}
			} catch(const std::exception& ex){
				printf("VMXCAN::ReleaseResources() - Caught exception:  %s\n", ex.what());
			}
		}
		task = 0;
	}

	if (p_stream_mgr) {
		std::map<uint32_t, VMXCANReceiveStream *>& stream_map =
				p_stream_mgr->GetStreamMap();

		std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
		for (it = stream_map.begin(); it != stream_map.end(); it++) {
			VMXErrorCode errcode;
			CloseReceiveStream(it->first, &errcode);
		}

		delete p_stream_mgr;
		p_stream_mgr = 0;
	}
}

VMXCAN::~VMXCAN() {
	ReleaseResources();
}

bool VMXCAN::SendMessage(VMXCANMessage& msg, int32_t periodMs, VMXErrorCode *errcode)
{
	CAN_TRANSFER tx_data;
	VMXCANReceiveStream::VMXCANMessageToCANTransfer(msg, tx_data);
	if (!can.enqueue_transmit_data(&tx_data)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXCAN::GetCANBUSStatus(VMXCANBusStatus& bus_status, VMXErrorCode *errcode)
{
	CAN_ERROR_FLAGS can_error_flags;
	CAN_IFX_INT_FLAGS can_int_flags;
	uint8_t rx_fifo_count;
	uint8_t tx_error_count = 0;
	uint8_t rx_error_count = 0;
	uint32_t bus_off_count = 0;
	uint32_t tx_full_count = 0;
	if(!can.get_bus_errors(can_error_flags, tx_error_count, rx_error_count,
			bus_off_count, tx_full_count)){
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	if (!can.get_interrupt_status(can_int_flags, rx_fifo_count)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	bus_status.percentBusUtilization = 0.0f; /* ??? */
	bus_status.transmitErrorCount = tx_error_count;
	bus_status.receiveErrorCount = rx_error_count;
	bus_status.busOffCount = bus_off_count;
	bus_status.txFullCount = tx_full_count;
	bus_status.busWarning = can_error_flags.can_bus_warn;
	bus_status.busPassiveError = can_error_flags.can_bus_err_pasv;
	bus_status.busOffError = can_error_flags.can_bus_tx_off;

	bus_status.hwRxOverflow = can_int_flags.hw_rx_overflow;
	bus_status.swRxOverflow = can_int_flags.sw_rx_overflow;
	bus_status.busError = can_int_flags.bus_error;
	bus_status.wake = can_int_flags.wake;
	bus_status.messageError = can_int_flags.message_error;

	return true;
}


bool VMXCAN::RetrieveAllCANData(VMXErrorCode *errcode)
{
	if (p_stream_mgr->GetNumStreams() == 0) return true;
	bool more_can_data_available = true;
	const int max_msgs_per_transfer = 2; /* This value * 17 is the total byte count. */
	const int limit_msgs_per_transfer = 254 / sizeof(TIMESTAMPED_CAN_TRANSFER); /* Limited by SPI Bus capabilities */
	while (more_can_data_available) {
		CAN_IFX_INT_FLAGS can_int_flags;
		uint8_t rx_fifo_count;

		if(can.get_interrupt_status(can_int_flags, rx_fifo_count)) {
#if 0
			if (rx_fifo_count > 0) {
				printf("CAN Rx Fifo Count:  %d\n", rx_fifo_count);
			}
#endif
			if(can_int_flags.hw_rx_overflow) {
				printf("CAN HW Receive Overflow detected.\n");
				ClearErrors();
				printf("Clearing CAN HW Receive Overflow.\n");
			}
			if(can_int_flags.sw_rx_overflow) {
				printf("CAN SW Receive Overflow detected.\n");
			}
			if(can_int_flags.bus_error) {
				printf("CAN Bus Error detected.\n");
			}
			if(can_int_flags.wake) {
				printf("CAN Bus Wake occured.\n");
			}
			if(can_int_flags.message_error) {
				printf("CAN Message Error detected.\n");
			}
			if(rx_fifo_count == 0) {
				more_can_data_available = false;
				break;
			}
			int success_receive_count = 0;
			while ( rx_fifo_count > 0 ) {
				int num_to_transfer = rx_fifo_count;
				if (num_to_transfer > max_msgs_per_transfer) {
					num_to_transfer = max_msgs_per_transfer;
				}
				rx_fifo_count -= num_to_transfer;
				TIMESTAMPED_CAN_TRANSFER rx_data[limit_msgs_per_transfer];
				uint8_t remaining_transfer_count;
				if(can.get_receive_data(&rx_data[0],num_to_transfer, remaining_transfer_count)){
					for ( int i = 0; i < num_to_transfer; i++ ) {
						if (!rx_data[i].transfer.id.sidl.invalid) {
#ifdef DEBUG_RX_ALL_CAN_DATA
							VMXCANTimestampedMessage stream_msg;
							VMXCANReceiveStream::TimestampedCANTransferToVMXCANStreamMessage(rx_data[i], stream_msg);
							uint32_t messageID = stream_msg.messageID;
							bool is_eid = true;
							if (messageID & VMXCAN_IS_FRAME_11BIT) {
								is_eid = false;
								messageID &= ~VMXCAN_IS_FRAME_11BIT;
							}
							messageID &= ~VMXCAN_IS_FRAME_REMOTE;
							printf("%2d:  [%10d] Received %d bytes of CAN data from %s 0x%x:  ",
									success_receive_count++,
									stream_msg.timeStampMS,
									stream_msg.dataSize,
									(is_eid ? "EID" : "ID "),
									messageID);
							for ( int j = 0; j < stream_msg.dataSize; j++) {
								printf("%x ", stream_msg.data[j]);
							}
							printf("\n");
#endif
							VMXCANReceiveStream *p_stream =
									p_stream_mgr->GetStreamForCAN_ID(rx_data[i].transfer.id);
							if (p_stream) {
								p_stream->ReceiveNewData(rx_data[i]);
							} else {
								uint32_t id;
								if (rx_data[i].transfer.id.sidl.ide) {
									CAN_unpack_extended_id(&rx_data[i].transfer.id, &id);
								} else {
									CAN_unpack_standard_id(&rx_data[i].transfer.id, &id);
								}
								printf("Discarding message id 0x%08x - no matching CAN Stream available.\n", id);
							}
						} else {
							printf("%2d:  Empty Packet entry.\n", success_receive_count);
						}
					}
				} else {
					printf("Error retrieving received CAN data.\n");
				}
			}
		} else {
			printf("ERROR retrieving CAN Interrupt Status.\n");
			more_can_data_available = false;
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
	}
	return true;
}

void VMXCAN::CANNewRxDataNotifyHandler(void *param, uint64_t timestamp_us)
{
	cv.notify_all();
}

bool VMXCAN::Reset(VMXErrorCode *errcode)
{
	/* Note:  may block for 3-4ms */
	if (!can.reset()) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	/* Wait 10ms after resetting CAN transceiver/controller */
	time.DelayMilliseconds(10);
	return true;
}

bool VMXCAN::FlushTxFIFO(VMXErrorCode *errcode) {
	if (!can.flush_tx_fifo()) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXCAN::FlushRxFIFO(VMXErrorCode *errcode) {
	p_stream_mgr->FlushReceiveFifos();
	if (!can.flush_rx_fifo()) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXCAN::SetMode(VMXCANMode mode, VMXErrorCode *errcode)
{
	CAN_MODE canmode;
	switch (mode) {
	case VMXCAN_OFF:
		canmode = CAN_MODE_SLEEP;
		break;
	case VMXCAN_CONFIG:
		canmode = CAN_MODE_CONFIG;
		break;
	case VMXCAN_NORMAL:
		canmode = CAN_MODE_NORMAL;
		break;
	case VMXCAN_LISTEN:
		canmode = CAN_MODE_LISTEN;
		break;
	case VMXCAN_LOOPBACK:
		canmode = CAN_MODE_LOOP;
		break;
	}
	if (!can.set_mode(canmode)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXCAN::GetMode(VMXCANMode& mode, VMXErrorCode *errcode)
{
	CAN_MODE canmode;
	if (!can.get_mode(canmode)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	switch(canmode) {
	case CAN_MODE_SLEEP:
		mode = VMXCAN_OFF;
		break;
	case CAN_MODE_CONFIG:
		mode = VMXCAN_CONFIG;
		break;
	case CAN_MODE_NORMAL:
		mode = VMXCAN_NORMAL;
		break;
	case CAN_MODE_LISTEN:
		mode = VMXCAN_LISTEN;
		break;
	case CAN_MODE_LOOP:
		mode = VMXCAN_LOOPBACK;
		break;
	}
	return true;
}

bool VMXCAN::OpenReceiveStream(VMXCANReceiveStreamHandle& session_handle,
		uint32_t messageID, uint32_t messageMask, uint32_t maxMessages, VMXErrorCode *errcode)
{
	uint8_t rxb_index;
	uint8_t rxb_filter_index;
	bool already_exists;
	if (p_stream_mgr->ReserveFilter(messageID, messageMask, rxb_index, rxb_filter_index, already_exists)) {
		if (already_exists) {
			uint32_t existing_streamid;
			if (p_stream_mgr->GetStreamForMessageID(messageID, existing_streamid)) {
				session_handle = existing_streamid;
				return true;
			}
			SET_VMXERROR(errcode, VMXERR_IO_INTERNAL_LOGIC_ERROR);
			return false;
		} else {
			/* Configure filters in hardware for this stream */
			bool success = true;
			CAN_ID existingMask;
			CAN_ID existingFilter;
			CANClient::CANRXBufferID rxbuffid = ((rxb_index == 0) ? CANClient::RXBUFFER_0 : CANClient::RXBUFFER_1);
			if (rxb_filter_index == 0) {
				CAN_ID new_mask;
				new_mask.sidl.srr = 0;
				new_mask.sidl.invalid = 0;
				VMXCANReceiveStream::CANMessageIDToCAN_ID(messageMask, new_mask);
				if (!can.get_rxbuffer_accept_mask(rxbuffid, existingMask)) {
					printf("Error retrieving existing accept mask for CAN buffer %d\n", rxbuffid);
					success = false;
				} else {
					if (!can.set_rxbuffer_accept_mask(rxbuffid, new_mask)) {
						printf("Error setting new accept mask for CAN buffer %d\n", rxbuffid);
						success = false;
					}
				}
				if (!success) {
					SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
					uint8_t remaining_num_filters_in_use;
					p_stream_mgr->UnreserveFilter(rxb_index, rxb_filter_index, remaining_num_filters_in_use);
				} else {
					printf("Set RX Buffer %d's Accept Mask to 0x%08X\n", rxb_index, messageMask);
				}
			}
			if (success) {
				CAN_ID can_filter;
				can_filter.sidl.srr = 0;
				can_filter.sidl.invalid = 0;
				VMXCANReceiveStream::CANMessageIDToCAN_ID(messageID, can_filter);
				if (!can.get_rxbuffer_accept_filter(rxbuffid, rxb_filter_index, existingFilter) ||
					!can.set_rxbuffer_accept_filter(rxbuffid, rxb_filter_index, can_filter)) {
					success = false;
					SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
					uint8_t remaining_num_filters_in_use_on_this_buffer;
					p_stream_mgr->UnreserveFilter(rxb_index, rxb_filter_index, remaining_num_filters_in_use_on_this_buffer);
					if (remaining_num_filters_in_use_on_this_buffer == 0) {
						can.set_rxbuffer_accept_mask(rxbuffid, existingMask);
					}
				} else {
					printf("Set RX Buffer %d's Accept Filter %d to 0x%08X\n", rxb_index, rxb_filter_index, messageID);
				}
				if (success) {
					/* Create a new VMXCANReceiveStream, store it's rxb index and filter index */
					VMXCANReceiveStream *p_new_stream = new VMXCANReceiveStream(
							messageID, messageMask, rxb_index, rxb_filter_index, maxMessages);
					if (p_stream_mgr->AddStream(p_new_stream, session_handle)) {
						if (p_stream_mgr->GetNumStreams() == 1) {
							can.RegisterNewRxDataNotifyHandler(VMXCAN::CANNewRxDataNotifyHandler, this);
						}
						return true;
					} else {
						delete p_new_stream;
						uint8_t remaining_num_filters_in_use_on_this_buffer;
						p_stream_mgr->UnreserveFilter(rxb_index, rxb_filter_index, remaining_num_filters_in_use_on_this_buffer);
						if (remaining_num_filters_in_use_on_this_buffer == 0) {
							can.set_rxbuffer_accept_mask(rxbuffid, existingMask);
						}
						can.set_rxbuffer_accept_filter(rxbuffid, rxb_filter_index, existingFilter);
						return false;
					}
				} else {
					return false;
				}
			}
		}
	} else {
		SET_VMXERROR(errcode, VMXERR_CAN_NO_AVAILABLE_MASKFILTER_SLOTS);
	}
	return false;
}

bool VMXCAN::ReadReceiveStream(VMXCANReceiveStreamHandle streamid,
		VMXCANTimestampedMessage *messages, uint32_t messagesToRead, uint32_t& messagesRead,
		VMXErrorCode *errcode)
{
	VMXCANReceiveStream *p_stream = p_stream_mgr->GetStream(streamid);
	if (!p_stream) {
		SET_VMXERROR(errcode, VMXERR_CAN_INVALID_RECEIVE_STREAM_HANDLE);
		return false;
	}
	return p_stream->Read(messages, messagesToRead, messagesRead);
}

bool VMXCAN::ClearErrors(VMXErrorCode *errcode) {
	CAN_IFX_INT_FLAGS can_int_flags;
	uint8_t rx_fifo_count;

	if (!can.get_interrupt_status(can_int_flags, rx_fifo_count)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	if (can_int_flags.hw_rx_overflow) {
		if (!can.clear_interrupt_flags(can_int_flags)) {
			SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
			return false;
		}
	}
	return true;
}

bool VMXCAN::CloseReceiveStream(VMXCANReceiveStreamHandle streamid, VMXErrorCode *errcode) {
	/* NOTE: Unused Masks default to 0 in hardware. It is likely best to set these to all FFs */
	/* Unused Filters default to an unknown state.  It is likely best to set these to all FFs */
	VMXCANReceiveStream *p_stream = p_stream_mgr->GetStream(streamid);
	if (!p_stream) {
		SET_VMXERROR(errcode, VMXERR_CAN_INVALID_RECEIVE_STREAM_HANDLE);
		return false;
	}
	uint8_t rx_buffer_id = p_stream->GetRXBufferID();
	uint8_t rx_buffer_filter_id = p_stream->GetRXFilterID();
	uint8_t remaining_num_filters_in_use_on_this_buffer;
	if (p_stream_mgr->UnreserveFilter(rx_buffer_id, rx_buffer_filter_id, remaining_num_filters_in_use_on_this_buffer)) {
		CAN_ID unused_filter;
		VMXCANReceiveStream::CANMessageIDToCAN_ID(0xFFFFFFFF, unused_filter);
		if (!can.set_rxbuffer_accept_filter((CANClient::CANRXBufferID)rx_buffer_id, rx_buffer_filter_id, unused_filter)) {
			/* Log error, but continue */
			log_warning("Error clearing CAN RX Buffer %d Filter %d.\n", rx_buffer_id, rx_buffer_filter_id);
		} else {
			printf("Cleared CAN RX Buffer %d Filter %d.\n", rx_buffer_id, rx_buffer_filter_id);
		}
		if (remaining_num_filters_in_use_on_this_buffer == 0) {
			CAN_ID unused_mask;
			VMXCANReceiveStream::CANMessageIDToCAN_ID(0xFFFFFFFF, unused_mask);
			if (!can.set_rxbuffer_accept_mask((CANClient::CANRXBufferID)rx_buffer_id, unused_mask)) {
				/* Log error, but continue */
				log_warning("Error clearing CAN Mask for RX Buffer %d.\n", rx_buffer_id);
			} else {
				printf("Cleared CAN Mask for RX Buffer %d.\n", rx_buffer_id);
			}
		}
		if (!p_stream_mgr->RemoveAndDeleteStream(streamid)) {
			log_warning("Error removing CAN Stream ID %d\n", streamid);
		}
		if (p_stream_mgr->GetNumStreams() == 0) {
			can.DeregisterNewRxDataNotifyHandler();
		}
		return true;
	} else {
		/* Uh-oh, this is a pretty bad internal error, it may not be possible
		 * after this to correctly allocate new VMXCANReceiveStreams.
		 */
		SET_VMXERROR(errcode, VMXERR_IO_INTERNAL_LOGIC_ERROR);
		return false;
	}
}

void VMXCAN::DisplayMasksAndFilters() {
	for (uint8_t buffid = 0; buffid < 2; buffid++) {
		CANClient::CANRXBufferID buff = ((buffid == 0) ? CANClient::RXBUFFER_0 : CANClient::RXBUFFER_1);
		CAN_RX_FILTER_MODE mode;
		if (can.get_rxbuffer_filter_mode(buff, mode)) {
			switch(mode) {
			case CAN_RX_FILTER_ALL:
				printf("Buffer %d:  CAN_RX_FILTER_ALL\n", buff);
				break;
			case CAN_RX_FILTER_SID_ONLY:
				printf("Buffer %d:  CAN_RX_FILTER_SID_ONLY\n", buff);
				break;
			case CAN_RX_FILTER_EID_ONLY:
				printf("Buffer %d:  CAN_RX_FILTER_EID_ONLY\n", buff);
				break;
			case CAN_RX_DISABLE_FILTERS:
				printf("Buffer %d:  CAN_RX_DISABLE_FILTERS\n", buff);
				break;
			}
		} else {
			printf("Error getting Buffer %d filter mode.\n", buff);
		}
		CAN_ID mask;
		if (can.get_rxbuffer_accept_mask(buff, mask)) {
		uint32_t mask_id;
			if (mask.sidl.ide) {
				CAN_unpack_extended_id(&mask, &mask_id);
				printf("Buffer %d Mask:  EID 0x%08X\n", buff, mask_id);
			} else {
				CAN_unpack_standard_id(&mask, &mask_id);
				printf("Buffer %d Mask:  SID 0x%08X\n", buff, mask_id);
			}
		} else {
			printf("Error getting Buffer %d mask.\n", buff);
		}
		for (int filternum = 0; filternum < ((buff == 0) ? 2 : 4); filternum++ ) {
			CAN_ID filter;
			if (can.get_rxbuffer_accept_filter(buff, filternum, filter)) {
				uint32_t filter_id;
				if (filter.sidl.ide) {
					CAN_unpack_extended_id(&filter, &filter_id);
					printf("Buffer %d Filter %d:  EID 0x%08X\n", buff, filternum, filter_id);
				} else {
					CAN_unpack_standard_id(&filter, &filter_id);
					printf("Buffer %d Filter %d:  SID 0x%08X\n", buff, filternum, filter_id);
				}
			} else {
				printf("Error getting Buffer %d filter %d.\n", buff, filternum);
			}
		}
	}
}

