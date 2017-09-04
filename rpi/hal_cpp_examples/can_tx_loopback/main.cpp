/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h>  /* printf() */
#include <string.h> /* memcpy() */
#include <inttypes.h>

#include "VMXPi.h"

void DisplayVMXError(VMXErrorCode vmxerr) {
	const char *p_err_description = GetVMXErrorString(vmxerr);
	printf("VMXError %d:  %s\n", vmxerr, p_err_description);
}

int main(int argc, char *argv[])
{
	bool realtime = false;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	try {
		if(vmx.IsOpen()) {

			/*
			 * CONFIGURE CAN; receive and display data; open 3 streams w/different address ranges
			 */

			VMXErrorCode vmxerr;
			VMXCANReceiveStreamHandle canrxhandles[1];
			if (!vmx.can.OpenReceiveStream(canrxhandles[0], 0x08041400, 0x0FFFFFF0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 1.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 1, handle:  %d\n", canrxhandles[0]);
			}

			/* Flush Rx/Tx fifo not necessary if invoking reset above. */
			if (!vmx.can.FlushRxFIFO(&vmxerr)) {
				printf("Error Flushing CAN RX FIFO.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Flushed CAN RX FIFO\n");
			}

			if (!vmx.can.FlushTxFIFO(&vmxerr)) {
				printf("Error Flushing CAN TX FIFO.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Flushed CAN TX FIFO\n");
			}

			if (!vmx.can.SetMode(VMXCAN::VMXCAN_LOOPBACK)) {
				printf("Error setting CAN Mode to LOOPBACK\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Set CAN Mode to Loopback.\n");
			}

			/* It's recommended to delay 20 Milliseconds after transitioning modes -
			 * to allow the CAN circuitry to stabilize; otherwise, sometimes
			 * there will be errors transmitting data during this period.
			 */
			vmx.time.DelayMilliseconds(20);

			for ( int i = 0; i < 10; i++) {
				VMXCANMessage msg;
				msg.messageID = 0x8041403; /* This is an extended ID */
				msg.dataSize = 8;
				memcpy(msg.data,"Hellox!",msg.dataSize);
				msg.data[5] = uint8_t('A') + i;
				if (!vmx.can.SendMessage(msg, 0, &vmxerr)) {
					printf("Error sending CAN message %d\n", i);
					DisplayVMXError(vmxerr);
				}
			}

			VMXCAN::VMXCANMode can_mode;
			if(vmx.can.GetMode(can_mode)) {
				printf("Current CAN Mode:  ");
				switch(can_mode) {
				case VMXCAN::VMXCAN_LISTEN:
					printf("LISTEN");
					break;
				case VMXCAN::VMXCAN_LOOPBACK:
					printf("LOOPBACK");
					break;
				case VMXCAN::VMXCAN_NORMAL:
					printf("NORMAL");
					break;
				case VMXCAN::VMXCAN_CONFIG:
					printf("CONFIG");
					break;
				case VMXCAN::VMXCAN_OFF:
					printf("OFF (SLEEP)");
					break;
				}
				printf("\n");
			} else {
				printf("Error retrieving current CAN Mode.\n");
			}

			/* Allow time for some CAN messages to be received. */
			for (int i = 0; i < 10; i++) {
				vmx.time.DelayMilliseconds(50);
				VMXCANBusStatus can_bus_status;
				if (!vmx.can.GetCANBUSStatus(can_bus_status, &vmxerr)) {
					printf("Error getting CAN BUS Status.\n");
					DisplayVMXError(vmxerr);
				} else {
					if(can_bus_status.busWarning) {
						printf("CAN Bus Warning.\n");
					}
					if(can_bus_status.busPassiveError) {
						printf("CAN Bus in Passive mode due to errors.\n");
					}
					if(can_bus_status.busOffError) {
						printf("CAN Bus Transmitter Off due to errors.\n");
					}
					if(can_bus_status.transmitErrorCount > 0) {
						printf("CAN Bus Tx Error Count:  %d\n", can_bus_status.transmitErrorCount);
					}
					if(can_bus_status.receiveErrorCount > 0) {
						printf("CAN Bus Rx Error Count:  %d\n", can_bus_status.receiveErrorCount);
					}
					if(can_bus_status.busOffCount > 0) {
						printf("CAN Bus Tx Off Count:  %d\n", can_bus_status.busOffCount);
					}
					if(can_bus_status.txFullCount > 0) {
						printf("CAN Bus Tx Full Count:  %d\n", can_bus_status.txFullCount);
					}
					if(can_bus_status.hwRxOverflow) {
						printf("CAN HW Receive Overflow detected.\n");
					}
					if(can_bus_status.swRxOverflow) {
						printf("CAN SW Receive Overflow detected.\n");
					}
					if(can_bus_status.busError) {
						printf("CAN Bus Error detected.\n");
					}
					if(can_bus_status.wake) {
						printf("CAN Bus Wake occured.\n");
					}
					if(can_bus_status.messageError) {
						printf("CAN Message Error detected.\n");
					}
				}

				for (int i = 0; i < 1; i++) {
					bool done = false;
					while (!done) {
						VMXCANTimestampedMessage stream_msg;
						uint32_t num_msgs_read;
						if (!vmx.can.ReadReceiveStream(canrxhandles[i], &stream_msg, 1, num_msgs_read, &vmxerr)) {
							printf("Error invoking CAN ReadReceiveStream for stream %d.\n", canrxhandles[i]);
							DisplayVMXError(vmxerr);
							done = true;
						}
						if (num_msgs_read == 0) {
							done = true;
						} else {
							bool is_eid = true;
							if (stream_msg.messageID & VMXCAN_IS_FRAME_11BIT) {
								is_eid = false;
								stream_msg.messageID &= ~VMXCAN_IS_FRAME_11BIT;
							}
							stream_msg.messageID &= ~VMXCAN_IS_FRAME_REMOTE;
							printf("[%10d] CAN Stream %d:  %d bytes from %s 0x%x:  ",
									stream_msg.timeStampMS,
									canrxhandles[i],
									stream_msg.dataSize,
									(is_eid ? "EID" : "ID "),
									stream_msg.messageID);
							for ( int j = 0; j < stream_msg.dataSize; j++) {
								printf("%02X ", stream_msg.data[j]);
							}
							printf("\n");
						}
					}
				}
			}
		} else {
			printf("Error:  Unable to open VMX Client.\n");
			printf("\n");
			printf("        - Is pigpio functional/running?\n");
			printf("        - Does this application have root privileges?\n");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}


