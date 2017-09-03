/*
 * VMXTimeImpl.h
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#ifndef VMXTIMEIMPL_H_
#define VMXTIMEIMPL_H_

#include <stdint.h>

class PIGPIOClient;

#include "VMXErrors.h"
#include "VMXHandlers.h"

#define MAX_NUM_TIMER_NOTIFY_HANDLERS 10

class VMXTimeImpl {

	friend class VMXPi;
	friend class VMXIO;

	PIGPIOClient& pigpio;

	typedef struct {
		VMXNotifyHandler p_handler;
		void *param;
		bool repeat;
		bool expired;
		int index;
		VMXTimeImpl *p_this;
		void Init() {
			p_handler = 0;
			param = 0;
			repeat = false;
			expired = true;
		}
	} TimerNotificationInfo;

	TimerNotificationInfo timer_notifications[MAX_NUM_TIMER_NOTIFY_HANDLERS];

	static void TimerNotificationHandlerInternal(void *param);
	uint64_t GetTotalSystemTimeOfTick(uint32_t tick);

public:
	VMXTimeImpl(PIGPIOClient& pigpio_ref);
	virtual ~VMXTimeImpl();
	bool Init();
	void ReleaseResources();

	uint64_t GetCurrentTotalMicroseconds();

	/* Registers for notification at an absolute timestamp [based on the Current System Timestamp] */
	/* Note:  a maximum of 10 Timer Notifications may be simultaneously registered.  Each of the 10 */
	/* is uniquely identified by timer_notify_handler */
	bool RegisterTimerNotificationAbsolute(VMXNotifyHandler timer_notify_handler, uint64_t trigger_timestamp_us, void *param);
	/* Registers for notification at a relative time from now */
	/* Note:  a maximum of 10 Timer Notifications may be simultaneously registered.  Each of the 10 */
	/* is uniquely identified by timer_notify_handler */
	bool RegisterTimerNotificationRelative(VMXNotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat);
	/* Unregister previously registered notify handler */
	bool DeregisterTimerNotification(VMXNotifyHandler timer_notify_handler);
	bool IsTimerNotificationExpired(VMXNotifyHandler timer_notify_handler, bool& expired);
};

#endif /* VMXTIMEIMPL_H_ */
