/*
 * VMXCANReceiveStream.h
 *
 *  Created on: 4 Aug 2017
 *      Author: pi
 */

#ifndef VMXCANRECEIVESTREAM_H_
#define VMXCANRECEIVESTREAM_H_

#include <deque>
#include <queue>
#include "VMXCAN.h"
#include <boost/circular_buffer.hpp>

class VMXCANReceiveStream {
	uint32_t CANIDFilter;
	uint32_t CANIDMask;
	uint32_t NumMessages;
	uint8_t rxb_id;
	uint8_t rxb_filter_id;
    boost::circular_buffer<VMXCANTimestampedMessage>* p_rx_data;

public:
	VMXCANReceiveStream(uint32_t id, uint32_t mask, uint8_t rxb_id, uint8_t rxbfilter_id, uint32_t NumMessages);
	virtual ~VMXCANReceiveStream();

	uint8_t GetRXBID() { return rxb_id; }
	uint8_t GetRXBFilterID() { return rxb_filter_id; }
	uint32_t GetFilter() { return CANIDFilter; }
	uint32_t GetMask() { return CANIDMask; }

	static void VMXCANMessageToCANTransfer(VMXCANMessage& src, CAN_TRANSFER& dest)
	{
		dest.payload.dlc.len = src.dataSize;
		if (dest.payload.dlc.len > 8) {
			dest.payload.dlc.len = 8;
		}
		memcpy(dest.payload.buff, src.data, dest.payload.dlc.len);
		CANMessageIDToCAN_ID(src.messageID, dest.id);
	}

	static void CANTransferToVMXCANMessage(CAN_TRANSFER& src, VMXCANMessage& dest)
	{
		dest.dataSize = src.payload.dlc.len;
		if (dest.dataSize > 8) {
			dest.dataSize = 8;
		}
		memcpy(dest.data, src.payload.buff,dest.dataSize);
		dest.messageID = 0;
		if (src.id.sidl.ide) {
			CAN_unpack_extended_id(&src.id, &dest.messageID);
		} else {
			CAN_unpack_standard_id(&src.id, &dest.messageID);
			dest.messageID |= VMXCAN_IS_FRAME_11BIT;
		}
		if (src.id.sidl.srr) {
			dest.messageID |= VMXCAN_IS_FRAME_REMOTE;
		}
	}

	static void TimestampedCANTransferToVMXCANStreamMessage(TIMESTAMPED_CAN_TRANSFER& src, VMXCANTimestampedMessage& dest)
	{
		dest.timeStampMS = src.timestamp_ms;
		CANTransferToVMXCANMessage(src.transfer, dest);
	}

	static void CANMessageIDToCAN_ID(uint32_t messageID, CAN_ID& dest)
	{
		if (messageID & VMXCAN_IS_FRAME_11BIT) {
			CAN_pack_standard_id(messageID, &dest);
		} else {
			CAN_pack_extended_id(messageID, &dest);
		}
		if (messageID & VMXCAN_IS_FRAME_REMOTE) {
			dest.sidl.srr = 1;
		} else {
			dest.sidl.srr = 0;
		}
	}

	static void CAN_IDToCANMessageID(CAN_ID& id, uint32_t& messageID)
	{
		messageID = 0;
		if (id.sidl.ide) {
			CAN_unpack_extended_id(&id, &messageID);
		} else {
			CAN_unpack_standard_id(&id, &messageID);
			messageID |= VMXCAN_IS_FRAME_11BIT;
		}
		if (id.sidl.srr) {
			messageID |= VMXCAN_IS_FRAME_REMOTE;
		}
	}

	bool ReceiveNewData(TIMESTAMPED_CAN_TRANSFER& new_data) {
    	VMXCANTimestampedMessage new_msg;
    	TimestampedCANTransferToVMXCANStreamMessage(new_data, new_msg);
    	p_rx_data->push_back(new_msg);
    	return true;
    }

    bool IDMatches(uint32_t messageID) {
    	return ((messageID & CANIDMask) == CANIDFilter);
    }

    bool IDMatches(CAN_ID& id) {
    	uint32_t messageID;
    	CAN_IDToCANMessageID(id,messageID);
    	return IDMatches(messageID);
    }

    uint32_t GetNumAvailableMessages() {
    	return uint32_t(p_rx_data->size());
    }

	bool Read(VMXCANTimestampedMessage *messages, uint32_t messagesToRead, uint32_t& messagesRead) {
		messagesRead = 0;
		for (uint32_t i = 0; i < messagesToRead; i++) {
			if (p_rx_data->empty()) break;
			messages[i] = p_rx_data->back();
			p_rx_data->pop_back();
			messagesRead++;
		}
		return true;
	}
};

#endif /* VMXCANRECEIVESTREAM_H_ */
