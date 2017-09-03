/*
 * VMXTimeImpl.cpp
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#include "VMXTimeImpl.h"
#include "PIGPIOClient.h"
#include "MISCClient.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>

static std::mutex time_mutex;

VMXTimeImpl::VMXTimeImpl(PIGPIOClient& pigpio_ref) :
	pigpio(pigpio_ref)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		timer_notifications[i].Init();
		timer_notifications[i].index = i;
		timer_notifications[i].p_this = this;
	}
}

bool VMXTimeImpl::Init()
{
	return true;
}

void VMXTimeImpl::ReleaseResources()
{
	/* Deregister all handlers */
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler) {
			DeregisterTimerNotification(timer_notifications[i].p_handler);
			timer_notifications[i].p_handler = 0;
		}
	}
}

VMXTimeImpl::~VMXTimeImpl() {
	ReleaseResources();
}

uint64_t VMXTimeImpl::GetCurrentTotalMicroseconds()
{
	return pigpio.GetTotalCurrentMicrosecondTicks();
}

bool VMXTimeImpl::RegisterTimerNotificationAbsolute(VMXNotifyHandler timer_notify_handler, uint64_t trigger_timestamp_us, void *param)
{
	uint64_t now = GetCurrentTotalMicroseconds();
	if (trigger_timestamp_us > now) {
		uint64_t microseconds_to_trigger = now - trigger_timestamp_us;
		return RegisterTimerNotificationRelative(timer_notify_handler, microseconds_to_trigger, param, false);
	}
	return false;
}

void VMXTimeImpl::TimerNotificationHandlerInternal(void *param)
{
	TimerNotificationInfo *p_info = (TimerNotificationInfo *)param;
	VMXNotifyHandler p_handler = p_info->p_handler;
	if (p_handler) {
		std::unique_lock<std::mutex> sync(time_mutex);
		if (p_handler) {
			(p_handler)(p_info->param, p_info->p_this->GetCurrentTotalMicroseconds());
			if (!p_info->repeat) {
				p_info->expired = true;
				p_info->p_this->pigpio.DeregisterTimerNotification(p_info->index);
			}
		}
	}
}

bool VMXTimeImpl::RegisterTimerNotificationRelative(VMXNotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (!timer_notifications[i].p_handler) {
			std::unique_lock<std::mutex> sync(time_mutex);
			if (!timer_notifications[i].p_handler) {
				unsigned millis_from_now = unsigned(time_from_now_us / 1000);
				if (pigpio.RegisterTimerNotification(i, VMXTimeImpl::TimerNotificationHandlerInternal, millis_from_now, (void *)&timer_notifications[i])) {
					timer_notifications[i].p_handler  = timer_notify_handler;
					timer_notifications[i].param = param;
					timer_notifications[i].expired = false;
					timer_notifications[i].repeat = repeat;
					return true;
				}
			}
		}
	}
	return false;
}

bool VMXTimeImpl::DeregisterTimerNotification(VMXNotifyHandler timer_notify_handler)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler == timer_notify_handler) {
			std::unique_lock<std::mutex> sync(time_mutex);
			if (timer_notifications[i].p_handler == timer_notify_handler) {
				if(pigpio.DeregisterTimerNotification(i)){
					timer_notifications[i].p_handler  = NULL;
					timer_notifications[i].param = NULL;
					timer_notifications[i].expired = true;
					timer_notifications[i].repeat = false;
					return true;
				}
			}
			break;
		}
	}
	return false;
}

bool VMXTimeImpl::IsTimerNotificationExpired(VMXNotifyHandler timer_notify_handler, bool& expired)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler == timer_notify_handler) {
			expired = timer_notifications[i].expired;
		}
	}
	return false;
}

