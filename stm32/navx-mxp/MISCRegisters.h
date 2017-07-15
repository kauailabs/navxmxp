/*
 * MISCRegisters.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef MISCREGISTERS_H_
#define MISCREGISTERS_H_

#include <stddef.h>

#define MISC_REGISTER_BANK 3

typedef struct {
	uint16_t powerctl		: 1; /* EXT. Power Management Support */
	uint16_t analog_trigger	: 1; /* Analog Trigger Support */
	uint16_t rtc			: 1; /* Real-time Clock/Calendar Support */
	uint16_t unused		    : 13;
} MISC_CAPABILITY_FLAGS;

typedef struct {
	uint8_t ext_pwr_overcurrent: 1; /* Received data available in Rx FIFO    */
	uint8_t					   : 7;
} MISC_EXT_PWR_CTL_STATUS;

typedef struct {
	uint8_t ext_pwr_currentlimit_mode	: 1;
	uint8_t								: 7;
} MISC_EXT_PWR_CTL_CFG;

typedef enum {
	none = 0,
	add1hr = 1,
	sub1hr = 2,
} MISC_DAYLIGHT_SAVINGS;

typedef struct __attribute__ ((__packed__)) {
	uint8_t			daylight_savings; /* MISC_DAYLIGHT_SAVINGS */
} MISC_RTC_CFG;

typedef struct __attribute__ ((__packed__)) {
	uint32_t       	subseconds; /* 0-999? */
	uint8_t			seconds;	/* 0-59 */
	uint8_t			minutes;	/* 0-59 */
	uint8_t			hours;		/* 0-23 */
} MISC_RTC_TIME;

typedef struct __attribute__ ((__packed__)) {
	uint8_t       	weekday;	/* 1-7, Monday == 1 */
	uint8_t			month;		/* 1-12, January == 1 */
	uint8_t			date;		/* 1-31 */
	uint8_t			year;		/* 0-99 */
} MISC_RTC_DATE;

typedef enum {
	ANALOG_TRIGGER_LOW = 0,
	ANALOG_TRIGGER_HIGH = 1,
	ANALOG_TRIGGER_IN_WINDOW = 2
} ANALOG_TRIGGER_STATE;

typedef enum {
	ANALOG_TRIGGER_DISABLED = 0,
	ANALOG_TRIGGER_MODE_STATE = 1,
	ANALOG_TRIGGER_MODE_RISING_EDGE_PULSE = 2, /* Internal routing to Interrupt only */
	ANALOG_TRIGGER_MODE_FALLING_EDGE_PULSE = 3 /* Internal routing to Interrupt only */
} ANALOG_TRIGGER_MODE;

#define MISC_NUM_ANALOG_TRIGGERS 4

struct __attribute__ ((__packed__)) MISC_REGS {
	/****************************/
	/* Capabilities (Read-only) */
	/****************************/
	MISC_CAPABILITY_FLAGS capability_flags;

	/**********************/
	/* Status (Read-only) */
	/**********************/
	MISC_EXT_PWR_CTL_STATUS ext_pwr_ctl_status;
	MISC_RTC_TIME rtc_time;
	MISC_RTC_DATE rtc_date;
	uint8_t analog_trigger_state[MISC_NUM_ANALOG_TRIGGERS]; /* ANALOG_TRIGGER_STATE */

	/************************************/
	/* Basic Configuration (Read/Write) */
	/************************************/
	MISC_EXT_PWR_CTL_CFG ext_pwr_ctl_cfg;
	MISC_RTC_CFG rtc_cfg;
	MISC_RTC_TIME rtc_time_set;
	MISC_RTC_DATE rtc_date_set;
	uint8_t analog_trigger_cfg[MISC_NUM_ANALOG_TRIGGERS]; /* ANALOG_TRIGGER_MODE */
	uint16_t analog_trigger_threshold_low[MISC_NUM_ANALOG_TRIGGERS];
	uint16_t analog_trigger_threshold_high[MISC_NUM_ANALOG_TRIGGERS];

	/*****************************************************************/
	/* Advanced Configuration (Read/Write)                           */
	/*****************************************************************/
	uint8_t end_of_bank;
};

#endif /* MISCREGISTERS_H_ */
