/*
 * VMXTime.cpp
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#include "VMXTime.h"
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

VMXTime::VMXTime(PIGPIOClient& pigpio_ref, MISCClient& misc_ref) :
	pigpio(pigpio_ref),
	misc(misc_ref)
{
	p_impl = new VMXTimeImpl(pigpio_ref);
}

bool VMXTime::Init()
{
	return true;
}

void VMXTime::ReleaseResources()
{
	p_impl->ReleaseResources();
}

VMXTime::~VMXTime() {
	ReleaseResources();
	delete p_impl;
}

uint64_t VMXTime::GetCurrentOSTimeMicroseconds()
{
	  struct timespec now;
	  clock_gettime( CLOCK_MONOTONIC_RAW, &now );
	  return (uint64_t)now.tv_sec * 1000000U + (uint64_t)(now.tv_nsec/1000);
}

uint32_t VMXTime::GetCurrentMicroseconds()
{
	return pigpio.GetCurrentMicrosecondTicks();
}

uint64_t VMXTime::GetCurrentTotalMicroseconds()
{
	return p_impl->GetCurrentTotalMicroseconds();
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
	return p_impl->RegisterTimerNotificationAbsolute(timer_notify_handler, trigger_timestamp_us, param);
}

bool VMXTime::RegisterTimerNotificationRelative(VMXNotifyHandler timer_notify_handler, uint64_t time_from_now_us, void *param, bool repeat)
{
	return p_impl->RegisterTimerNotificationRelative(timer_notify_handler, time_from_now_us, param, repeat);
}

bool VMXTime::DeregisterTimerNotification(VMXNotifyHandler timer_notify_handler)
{
	return p_impl->DeregisterTimerNotification(timer_notify_handler);
}

bool VMXTime::IsTimerNotificationExpired(VMXNotifyHandler timer_notify_handler, bool& expired)
{
	return p_impl->IsTimerNotificationExpired(timer_notify_handler, expired);
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

uint32_t VMXTime::DelayMilliseconds(uint32_t delay_ms)
{
	uint32_t delay_us = delay_ms * 1000;
	uint32_t actual_delay_us = DelayMicroseconds(delay_us);
	return actual_delay_us / 1000;
}

uint32_t VMXTime::DelaySeconds(uint32_t delay_sec)
{
	uint32_t delay_ms = delay_sec * 1000;
	uint32_t actual_delay_ms = DelayMilliseconds(delay_ms);
	return actual_delay_ms / 1000;
}

