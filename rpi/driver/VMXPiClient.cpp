/*
 * VMXPiClient.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include "VMXPiClient.h"
#include <pigpio.h>

VMXPiClient::gpioInterruptFunc_t interrupt_funcs[32] = {0};

VMXPiClient::VMXPiClient(uint8_t ahrs_update_rate_hz) :
	pigpio((IPIGPIOInterruptSink&)*this),
	spi((PIGPIOClient&)pigpio),
	iocx((SPIClient&)spi),
	ahrs((SPIClient&)spi, ahrs_update_rate_hz),
	can((SPIClient&)spi),
	misc((SPIClient&)spi),
	io(pigpio, iocx, misc, chan_mgr, rsrc_mgr),
	clocks((PIGPIOClient&)pigpio)
{
	VMXChannelManager::Init();
	clocks.Init();
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		timer_notifications[i].Init();
		timer_notifications[i].index = i;
		timer_notifications[i].p_this = this;
	}
}

VMXPiClient::~VMXPiClient() {
	ahrs.Stop();	/* Stop AHRS Data IO Thread */
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler) {
			DeregisterTimerNotification(timer_notifications[i].p_handler);
		}
	}
}

void VMXPiClient::iocx_interrupt(int level, uint32_t tick) {
	/* Request IOCX Interrupt Status, indicating which VMX GPIO was the source */
	uint16_t iocx_int_status;
	if(iocx.get_gpio_interrupt_status(iocx_int_status)) {
		/* Notify HAL Client of Interrupt */
	} else {
		printf("Error retrieving VMX IOCX Interrupt Status in gpio_isr().\n");
	}
	uint8_t vmx_interrupt_number = 0; /* Todo:  Set to interrupt identified by status. */
	if (interrupt_funcs[vmx_interrupt_number]) {
		interrupt_funcs[vmx_interrupt_number](vmx_interrupt_number, level, tick);
	}
}

void VMXPiClient::can_interrupt(int level, uint32_t tick) {
	/* Inform can client new data is available? */
}

void VMXPiClient::ahrs_interrupt(int level, uint32_t tick) {
	/* Inform AHRS client new data is available? */
}

void VMXPiClient::pigpio_interrupt(int gpio_num, int level, uint32_t tick) {
	uint8_t vmx_interrupt_number = iocx.get_num_interrupts() + gpio_num;
	if (interrupt_funcs[vmx_interrupt_number]) {
		interrupt_funcs[vmx_interrupt_number](vmx_interrupt_number, level, tick);
	}
}

bool VMXPiClient::RegisterTimerNotificationAbsolute(NotifyHandler timer_notify_handler, uint64_t trigger_timestamp_us, void *param)
{
	uint64_t now = clocks.GetCurrentTotalMicroseconds();
	if (trigger_timestamp_us > now) {
		uint64_t microseconds_to_trigger = now - trigger_timestamp_us;
		return RegisterTimerNotificationRelative(timer_notify_handler, microseconds_to_trigger, param, false);
	}
	return false;
}

void VMXPiClient::TimerNotificationHandlerInternal(void *param)
{
	TimerNotificationInfo *p_info = (TimerNotificationInfo *)param;
	NotifyHandler p_handler = p_info->p_handler;
	if (p_handler) {
		(p_handler)(p_info->param, p_info->p_this->GetCurrentSystemTimestamp());
		if (!p_info->repeat) {
			p_info->expired = true;
			p_info->p_this->pigpio.DeregisterTimerNotification(p_info->index);
		}
	}
}

bool VMXPiClient::RegisterTimerNotificationRelative(NotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (!timer_notifications[i].p_handler) {
			unsigned millis_from_now = unsigned(time_from_now_us / 1000);
			if (pigpio.RegisterTimerNotification(i, VMXPiClient::TimerNotificationHandlerInternal, millis_from_now, (void *)&timer_notifications[i])) {
				timer_notifications[i].p_handler  = timer_notify_handler;
				timer_notifications[i].param = param;
				timer_notifications[i].expired = false;
				timer_notifications[i].repeat = repeat;
				return true;
			}
		}
	}
	return false;
}

bool VMXPiClient::DeregisterTimerNotification(NotifyHandler timer_notify_handler)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler == timer_notify_handler) {
			if(pigpio.DeregisterTimerNotification(i)){
				timer_notifications[i].p_handler  = NULL;
				timer_notifications[i].param = NULL;
				timer_notifications[i].expired = true;
				timer_notifications[i].repeat = false;
				return true;
			}
			break;
		}
	}
	return false;
}


bool VMXPiClient::IsTimerNotificationExpired(NotifyHandler timer_notify_handler, bool& expired)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler == timer_notify_handler) {
			expired = timer_notifications[i].expired;
		}
	}
	return false;
}




