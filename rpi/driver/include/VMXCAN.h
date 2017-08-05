/*
 * VMXCAN.h
 *
 *  Created on: 1 Aug 2017
 *      Author: pi
 */

#ifndef VMXCAN_H_
#define VMXCAN_H_

class CANClient;

#include <stdint.h>
#include <string.h>

#include "VMXErrors.h"
#include "CANRegisters.h"

/* NOTE:  Usage Model can be seen in CtreCanNode.cpp, PCM.cpp, PDP.cpp (hal/lib/athena/ctre) */

// ala CANInterfacePlugin.h

#define VMXCAN_IS_FRAME_REMOTE			0x80000000
#define VMXCAN_IS_FRAME_11BIT			0x40000000
#define VMXCAN_29BIT_MESSAGE_ID_MASK	0x1FFFFFFF
#define VMXCAN_11BIT_MESSAGE_ID_MASK	0x000007FF

// ala CANSessionMux.h

#define VMXCAN_SEND_PERIOD_NO_REPEAT		0
#define VMXCAN_SEND_PERIOD_STOP_REPEATING	-1

#define VMXERR_CAN_InvalidBuffer 		-44086
#define VMXERR_CAN_MessageNotFound 		-44087
#define VMXERR_WARN_NoToken				 44087
#define VMXERR_NotAllowed				-44088
#define VMXERR_NotInitialized			-44089
#define VMXERR_SessionOverrun			 44050

typedef struct sVMXCANMessage {
	uint32_t messageID;
	uint8_t  data[8];
	uint8_t  dataSize;
} VMXCANMessage;

typedef struct sVMXTimestampedCANMessage : public sVMXCANMessage {
	uint32_t timeStampMS;
} VMXCANTimestampedMessage;

typedef struct {
	float percentBusUtilization;
	/* busOffCount:  The number of times that sendMessage failed with a busOff error indicating that messages are not
	 * successfully transmitted on the bus.
	 */
	uint32_t busOffCount;
	/* txFullCount:  The number of times that sendMessage failed with a txFifoFull error indicating that messages are not
	 * successfully received by any CAN device.
	 */
	uint32_t txFullCount;
	/* receiveErrorCount:  The count of receive errors as reported by the CAN driver. */
	uint32_t receiveErrorCount;
	/* transmitErrorCount:  The count of transmit errors as reported by the CAN driver. */
	uint32_t transmitErrorCount;
	bool busWarning; 		/* Currently a bus warning condition exists */
	bool busPassiveError; 	/* Currently a bus passive condition exists */
	bool busOffError;     	/* Currently a bus off condition exists */
	bool hwRxOverflow;		/* Hardware-level buffer overflow */
	bool swRxOverflow;		/* Firmware-level buffer overflow */
	bool busError;			/* Transceiver bus error detected */
	bool wake;				/* Transceiver wakeup event occurred */
	bool messageError;		/* Controller message error detected */
} VMXCANBusStatus;

typedef uint32_t VMXCANReceiveStreamHandle;

class VMXCANReceiveStreamManager;

class VMXCAN {

	CANClient& can;
	VMXCANReceiveStreamManager *p_stream_mgr;

public:
	VMXCAN(CANClient& can_ref);
	virtual ~VMXCAN();
	void ReleaseResources();

	/* This function may be called from multiple contexts and must therefore be reentrant.
	 *
	 * The message is placed into a Fifo and transmitted as soon as possible.
	 *
	 * periodMs:  if non zero, the message is periodically transmitted.  Periodic transmission of data occurs
	 * until the next time SendMessage is invoked.  If the periodMs is 0, this disables the messageID's
	 * preceding periodic transmit, and sends the data.  To cancel periodic transmission w/out sending new
	 * data, set periodMs to -1.
	 */
	bool SendMessage(VMXCANMessage& msg, int32_t periodMs, VMXErrorCode *errcode = 0);

	/* messageID:  The CAN Arb ID for the message of interest.  THis is basically the Message FILTER.  */
	/* messsageMask:  The Message MASK to use for the messages of interest.  NOTE: the underlying hardware supports */
	/* one mask for each of the two receive buffers.  If more than two streams are opened with different masks, these */
	/* masks are ANDed. */
	/* maxMessages:  This is maximum number of received messages to internally buffer for this stream. */
	bool OpenReceiveStream(VMXCANReceiveStreamHandle& session_handle, uint32_t messageID, uint32_t messageMask, uint32_t maxMessages, VMXErrorCode *errcode = 0);
	bool ReadReceiveStream(VMXCANReceiveStreamHandle sessionHandle, VMXCANTimestampedMessage *messages, uint32_t messagesToRead, uint32_t& messagesRead, VMXErrorCode *errcode = 0);
	bool CloseReceiveStream(VMXCANReceiveStreamHandle, VMXErrorCode *errcode = 0);

	bool GetCANBUSStatus(VMXCANBusStatus& bus_status, VMXErrorCode *errcode = 0);

	/* Note:  may block for 3-4ms */
	bool Reset(VMXErrorCode *errcode = 0);
	bool FlushTxFIFO(VMXErrorCode *errcode = 0);
	bool FlushRxFIFO(VMXErrorCode *errcode = 0);

	typedef enum {VMXCAN_OFF, VMXCAN_CONFIG, VMXCAN_NORMAL, VMXCAN_LISTEN, VMXCAN_LOOPBACK } VMXCANMode;
	bool SetMode(VMXCANMode mode, VMXErrorCode *errcode = 0);
	bool GetMode(VMXCANMode& mode, VMXErrorCode *errcode = 0);

	bool ClearErrors(VMXErrorCode *errcode = 0);

	bool RetrieveAllCANData(VMXErrorCode *errcode = 0);
};

#endif /* VMXCAN_H_ */
