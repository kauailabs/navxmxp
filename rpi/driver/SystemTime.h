/*
 * SystemTime.h
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#ifndef SYSTEMTIME_H_
#define SYSTEMTIME_H_

#include <stdint.h>
#include "PIGPIOClient.h"
#include "MISCClient.h"
#include "VMXErrors.h"

#define MAX_NUM_TIMER_NOTIFY_HANDLERS 10

class SystemTime {
	PIGPIOClient& pigpio;
	MISCClient& misc;
	friend class VMXPiClient;
public:
	SystemTime(PIGPIOClient& pigpio_ref, MISCClient& misc_ref);
	virtual ~SystemTime();

	bool Init();

	uint64_t GetCurrentOSTimeInMicroseconds();
	uint64_t GetCurrentOSTimeInMicrosecondsAlt();
	uint32_t GetCurrentMicroseconds();
	uint64_t GetCurrentTotalMicroseconds();
	uint32_t GetCurrentMicrosecondsHighPortion();

	/* Delays the requested number of microseconds.  Delay values of less than 100 microseconds
	 * are implemented as a busy-wait.
	 */
	uint32_t DelayMicroseconds(uint32_t delay_us) { return pigpio.Delay(delay_us); }

	/* Registers for notification at an absolute timestamp [based on the Current System Timestamp] */
	/* Note:  a maximum of 10 Timer Notifications may be simultaneously registered.  Each of the 10 */
	/* is uniquely identified by timer_notify_handler */
	bool RegisterTimerNotificationAbsolute(NotifyHandler timer_notify_handler, uint64_t trigger_timestamp_us, void *param);
	/* Registers for notification at a relative time from now */
	/* Note:  a maximum of 10 Timer Notifications may be simultaneously registered.  Each of the 10 */
	/* is uniquely identified by timer_notify_handler */
	bool RegisterTimerNotificationRelative(NotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat);
	/* Unregister previously registered notify handler */
	bool DeregisterTimerNotification(NotifyHandler timer_notify_handler);
	bool IsTimerNotificationExpired(NotifyHandler timer_notify_handler, bool& expired);

	/*** RTC ***/
	bool GetRTCTime(uint8_t& hours, uint8_t& minutes, uint8_t& seconds, uint32_t& subseconds, VMXErrorCode *errcode = 0);
	bool GetRTCDate(uint8_t& weekday, uint8_t& day, uint8_t& month, uint8_t& years, VMXErrorCode *errcode = 0);
	bool SetRTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, VMXErrorCode *errcode = 0);
	bool SetRTCDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t years, VMXErrorCode *errcode = 0);

protected:
	typedef struct {
		NotifyHandler p_handler;
		void *param;
		bool repeat;
		bool expired;
		int index;
		SystemTime *p_this;
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
};

#endif /* SYSTEMTIME_H_ */
