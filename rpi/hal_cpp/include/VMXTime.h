/* ============================================
VMX-pi HAL source code is placed under the MIT license
Copyright (c) 2017 Kauai Labs
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef VMXTIME_H_
#define VMXTIME_H_

#include <stdint.h>

class PIGPIOClient;
class MISCClient;
class VMXTimeImpl;

#include "VMXErrors.h"
#include "VMXHandlers.h"

class VMXTime {

	friend class VMXPi;
	friend class VMXIO;

	PIGPIOClient& pigpio;
	MISCClient& misc;
	VMXTimeImpl *p_impl;

	VMXTime(PIGPIOClient& pigpio_ref, MISCClient& misc_ref);
	virtual ~VMXTime();
	bool Init();
	void ReleaseResources();
	uint64_t GetTotalSystemTimeOfTick(uint32_t tick);

public:
	uint64_t GetCurrentOSTimeMicroseconds();

	uint32_t GetCurrentMicroseconds();
	uint64_t GetCurrentTotalMicroseconds();
	uint32_t GetCurrentMicrosecondsHighPortion();

	/* Delays the requested number of microseconds.  Delay values of less than 100 microseconds
	 * are implemented as a busy-wait.
	 */
	uint32_t DelayMicroseconds(uint32_t delay_us);
	uint32_t DelayMilliseconds(uint32_t delay_ms);
	uint32_t DelaySeconds(uint32_t delay_sec);

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

	/*** Real-Time Clock ***/
	typedef enum {
		DSAdjustmentNone = 0,
		DSAdjustmentAddOneHour = 1,
		DSAdjustmentSubtractOneHour = 2,
	} DaylightSavingsAdjustment;

	bool GetRTCTime(uint8_t& hours, uint8_t& minutes, uint8_t& seconds, uint32_t& subseconds, VMXErrorCode *errcode = 0);
	bool GetRTCDate(uint8_t& weekday, uint8_t& day, uint8_t& month, uint8_t& years, VMXErrorCode *errcode = 0);
	bool GetRTCDaylightSavingsAdjustment(DaylightSavingsAdjustment& dsa, VMXErrorCode *errcode = 0);
	bool SetRTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, VMXErrorCode *errcode = 0);
	bool SetRTCDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t years, VMXErrorCode *errcode = 0);
	bool SetRTCDaylightSavingsAdjustment(DaylightSavingsAdjustment dsa, VMXErrorCode *errcode = 0);
};

#endif /* VMXTIME_H_ */
