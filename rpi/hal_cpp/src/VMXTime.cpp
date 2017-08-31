/*
 * VMXTime.cpp
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#include "VMXTime.h"
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

VMXTime::VMXTime(PIGPIOClient& pigpio_ref, MISCClient& misc_ref) :
	pigpio(pigpio_ref),
	misc(misc_ref)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		timer_notifications[i].Init();
		timer_notifications[i].index = i;
		timer_notifications[i].p_this = this;
	}
}

bool VMXTime::Init()
{
	return true;
}

void VMXTime::ReleaseResources()
{
	/* Deregister all handlers */
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler) {
			DeregisterTimerNotification(timer_notifications[i].p_handler);
			timer_notifications[i].p_handler = 0;
		}
	}
}

VMXTime::~VMXTime() {
	ReleaseResources();
}

uint64_t VMXTime::GetCurrentOSTimeInMicroseconds()
{
	  struct timespec now;
	  clock_gettime( CLOCK_MONOTONIC_RAW, &now );
	  return (uint64_t)now.tv_sec * 1000000U + (uint64_t)(now.tv_nsec/1000);
}

uint64_t VMXTime::GetCurrentOSTimeInMicrosecondsAlt()
{
    struct timeval start;
    gettimeofday(&start, NULL);

    return (uint64_t)start.tv_sec * 1000000U + (uint64_t)start.tv_usec;
}

uint32_t VMXTime::GetCurrentMicroseconds()
{
	return pigpio.GetCurrentMicrosecondTicks();
}

uint64_t VMXTime::GetCurrentTotalMicroseconds()
{
	return pigpio.GetTotalCurrentMicrosecondTicks();
}

uint32_t VMXTime::GetCurrentMicrosecondsHighPortion()
{
	return pigpio.GetCurrentMicrosecondTicksHighPortion();
}

uint64_t VMXTime::GetTotalSystemTimeOfTick(uint32_t tick) {
	return pigpio.GetTotalSystemTimeOfTick(tick);
}

bool VMXTime::RegisterTimerNotificationAbsolute(VMXNotifyHandler timer_notify_handler, uint64_t trigger_timestamp_us, void *param)
{
	uint64_t now = GetCurrentTotalMicroseconds();
	if (trigger_timestamp_us > now) {
		uint64_t microseconds_to_trigger = now - trigger_timestamp_us;
		return RegisterTimerNotificationRelative(timer_notify_handler, microseconds_to_trigger, param, false);
	}
	return false;
}

void VMXTime::TimerNotificationHandlerInternal(void *param)
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

bool VMXTime::RegisterTimerNotificationRelative(VMXNotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (!timer_notifications[i].p_handler) {
			std::unique_lock<std::mutex> sync(time_mutex);
			if (!timer_notifications[i].p_handler) {
				unsigned millis_from_now = unsigned(time_from_now_us / 1000);
				if (pigpio.RegisterTimerNotification(i, VMXTime::TimerNotificationHandlerInternal, millis_from_now, (void *)&timer_notifications[i])) {
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

bool VMXTime::DeregisterTimerNotification(VMXNotifyHandler timer_notify_handler)
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

bool VMXTime::IsTimerNotificationExpired(VMXNotifyHandler timer_notify_handler, bool& expired)
{
	for (size_t i = 0; i < (sizeof(timer_notifications)/sizeof(timer_notifications[0])); i++ ) {
		if (timer_notifications[i].p_handler == timer_notify_handler) {
			expired = timer_notifications[i].expired;
		}
	}
	return false;
}

bool VMXTime::GetRTCTime(uint8_t& hours, uint8_t& minutes, uint8_t& seconds, uint32_t& subseconds, VMXErrorCode *errcode)
{
	if (!misc.get_rtc_time(hours, minutes, seconds, subseconds)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXTime::GetRTCDate(uint8_t& weekday, uint8_t& day, uint8_t& month, uint8_t& years, VMXErrorCode *errcode)
{
	if (!misc.get_rtc_date(weekday, day, month, years)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXTime::SetRTCTime(uint8_t hours, uint8_t minutes, uint8_t seconds, VMXErrorCode *errcode)
{
	if (!misc.set_rtc_time(hours, minutes, seconds)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXTime::SetRTCDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t years, VMXErrorCode *errcode)
{
	if (!misc.set_rtc_date(weekday, day, month, years)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXTime::GetRTCDaylightSavingsAdjustment(DaylightSavingsAdjustment& dsa, VMXErrorCode *errcode)
{
	MISC_RTC_CFG rtc_cfg;
	if (!misc.get_rtc_cfg(rtc_cfg)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	switch(rtc_cfg.daylight_savings) {
	case sub1hr:
		dsa = VMXTime::DaylightSavingsAdjustment::DSAdjustmentSubtractOneHour;
		break;
	case add1hr:
		dsa = VMXTime::DaylightSavingsAdjustment::DSAdjustmentAddOneHour;
		break;
	default:
		dsa = VMXTime::DaylightSavingsAdjustment::DSAdjustmentNone;
		break;
	}
	return true;
}

bool VMXTime::SetRTCDaylightSavingsAdjustment(DaylightSavingsAdjustment dsa, VMXErrorCode *errcode)
{
	MISC_RTC_CFG rtc_cfg;
	switch(dsa) {
	case VMXTime::DaylightSavingsAdjustment::DSAdjustmentSubtractOneHour:
		rtc_cfg.daylight_savings = sub1hr;
		break;
	case VMXTime::DaylightSavingsAdjustment::DSAdjustmentAddOneHour:
		rtc_cfg.daylight_savings = add1hr;
		break;
	default:
		rtc_cfg.daylight_savings = none;
		break;
	}
	if (!misc.set_rtc_cfg(rtc_cfg)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}


uint32_t VMXTime::DelayMicroseconds(uint32_t delay_us)
{
	return pigpio.Delay(delay_us);
}


