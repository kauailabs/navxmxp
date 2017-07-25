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
#include "VMXChannelManager.h"
#include "VMXResourceManager.h"
#include "IPIGPIOInterruptSink.h"
#include "SystemClocks.h"

#define MAX_NUM_TIMER_NOTIFY_HANDLERS 10

class VMXPiClient : public IPIGPIOInterruptSink {
public:

	PIGPIOClient pigpio;

protected:

	SPIClient spi;
	VMXChannelManager chan_mgr;
	VMXResourceManager rsrc_mgr;

	typedef struct {
		NotifyHandler p_handler;
		void *param;
		bool repeat;
		bool expired;
		int index;
		VMXPiClient *p_this;
		void Init() {
			p_handler = 0;
			param = 0;
			repeat = false;
			expired = true;
		}
	} TimerNotificationInfo;

	TimerNotificationInfo timer_notifications[MAX_NUM_TIMER_NOTIFY_HANDLERS];

public:

	IOCXClient iocx;	/* Make this protected? */
	AHRS ahrs;
	CANClient can;
	MISCClient misc;
	VMXIO io;
	SystemClocks clocks;

public:

	typedef void (*gpioInterruptFunc_t)
	   (int vmx_gpio, int level, uint32_t tick);

	VMXPiClient(uint8_t ahrs_update_rate_hz);

	virtual ~VMXPiClient();

	bool is_open() { return pigpio.is_open(); }

	uint8_t get_num_vmx_dios() { return iocx.GetNumGpios(); }
	uint8_t get_num_rpi_dios() { return 16; }
	uint8_t get_max_num_vmx_pwm_channels() { return iocx.get_num_pwm_channels(); }
	uint8_t get_actual_num_vmx_pwm_channels() { return 0; /* Todo */ }
	uint8_t get_max_num_rpi_pwm_channels() { return 16; }
	uint8_t get_actual_num_rpi_pwm_channels() { return 0; /* Todo */ }
	uint8_t get_num_counters() { return iocx.get_num_timers(); }
	uint8_t get_max_num_encoders() { return 4; /* 5? */ /* Todo:  Move to IOCXClient */ }
	uint8_t get_actual_num_encoders() { return 0; /* Todo */ }
	uint8_t get_num_interrupts() { return iocx.get_num_interrupts() + 16; }
	uint8_t get_num_analog_inputs() { return 4; } /* Todo:  Move to ResMgr */

	uint8_t get_num_pigpio_interrupts() { return 16; }

	void test_pigpio_pwm_outputs() { pigpio.test_pwm_outputs(); }
	void test_pigpio_ext_i2c() { pigpio.test_ext_i2c(); }
	void test_pigpio_gpio_inputs(int iteration_count) { pigpio.test_gpio_inputs(iteration_count); }

	std::string GetFirmwareVersion() { return ahrs.GetFirmwareVersion(); }

	uint64_t GetCurrentSystemTimestamp() { return clocks.GetCurrentTotalMicroseconds(); }

	/* Delays for specified number of microseconds.  Returns the actual number of milliseconds delayed.
	 * Values less than 100us will delay via busy wait; values > 100us will cause the current thread
	 * to yield.
	 */
	uint32_t Delay(uint32_t delay_us) { return pigpio.Delay(delay_us); }

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

private:
	void iocx_interrupt(int level, uint32_t tick);
	void can_interrupt(int level, uint32_t tick);
	void ahrs_interrupt(int level, uint32_t tick);
	void pigpio_interrupt(int gpio_num, int level, uint32_t tick);

	static void TimerNotificationHandlerInternal(void *param);
};

#endif /* VMXPICLIENT_H_ */
