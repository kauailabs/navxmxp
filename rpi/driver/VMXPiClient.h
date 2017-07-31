/*
 * VMXPiClient.h
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#ifndef VMXPICLIENT_H_
#define VMXPICLIENT_H_

#include <stdint.h>
#include "PIGPIOClient.h"
#include "SPIClient.h"

#include "AHRS.h"
#include "IOCXClient.h"
#include "CANClient.h"
#include "MISCClient.h"
#include "VMXIO.h"
#include "Power.h"
#include "VMXChannelManager.h"
#include "VMXResourceManager.h"
#include "IPIGPIOInterruptSink.h"
#include "IInterruptRegistrar.h"
#include "SystemTime.h"
#include "Version.h"

#define MAX_NUM_INTERRUPT_NOTIFY_HANDLERS 32

class VMXPiClient : public IPIGPIOInterruptSink, public IInterruptRegistrar {
protected:

	PIGPIOClient pigpio;
	SPIClient spi;
	IOCXClient iocx;
	VMXChannelManager chan_mgr;
	VMXResourceManager rsrc_mgr;

public:

	AHRS 		ahrs;
	CANClient 	can;
	MISCClient 	misc;
	VMXIO 		io;
	SystemTime 	time;
	Power		power;
	Version		version;

	VMXPiClient(uint8_t ahrs_update_rate_hz);

	virtual ~VMXPiClient();

	bool IsOpen() { return pigpio.IsOpen(); }

	virtual bool RegisterCANNewRxDataHandler(CAN_NewRxDataNotifyHandler handler, void *param);
	virtual bool DeregisterCANNewRxDataHandler();

	virtual bool RegisterAHRSNewRxDataHandler(AHRS_NewRxDataNotifyHandler handler, void *param);
	virtual bool DeregisterAHRSNewRxDataHandler();

protected:

	typedef struct {
		IO_InterruptHandler interrupt_func;
		void *interrupt_params;
		void Init() {
			interrupt_func = 0;
			interrupt_params = 0;
		}
	} InterruptNotificationInfo;

	InterruptNotificationInfo interrupt_notifications[MAX_NUM_INTERRUPT_NOTIFY_HANDLERS];

	typedef struct {
		CAN_NewRxDataNotifyHandler handler_func;
		void *param;
		void Init() {
			handler_func = 0;
			param = 0;
		}
	} CANNotificationInfo;

	CANNotificationInfo can_notification;

	typedef struct {
		AHRS_NewRxDataNotifyHandler handler_func;
		void *param;
		void Init() {
			handler_func = 0;
			param = 0;
		}
	} AHRSNotificationInfo;

	AHRSNotificationInfo ahrs_notification;

	/* IInterruptRegistrar implementation */
	virtual bool RegisterIOInterruptHandler(uint8_t index, IO_InterruptHandler handler, void *param);
	virtual bool DeregisterIOInterruptHandler(uint8_t index);

	void iocx_interrupt(int level, uint32_t tick);
	void can_interrupt(int level, uint32_t tick);
	void ahrs_interrupt(int level, uint32_t tick);
	void pigpio_interrupt(int gpio_num, int level, uint32_t tick);
};

#endif /* VMXPICLIENT_H_ */
