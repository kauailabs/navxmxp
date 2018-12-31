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
	uint16_t analog_in		: 1; /* Analog Input Support */
	uint16_t powerctl		: 1; /* EXT. Power Management Support */
	uint16_t analog_trigger	: 1; /* Analog Trigger Support */
	uint16_t rtc			: 1; /* Real-time Clock/Calendar Support */
	uint16_t an_in_5V		: 1; /* If set, analog input voltage is 5V (3.3V if clear) */
	uint16_t accum_counters : 1; /* If set, analog accumulator counters are supported. */
	uint16_t unused		    : 10;
} MISC_CAPABILITY_FLAGS;

typedef struct {
	uint8_t ext_pwr_overcurrent : 1; /* Output Overcurrent currently detected */
	uint8_t ext_pwr_undervoltage: 1; /* Input Undervoltage currently detected */
	uint8_t					    : 6;
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

#define MISC_NUM_ANALOG_INPUTS 4

#define MISC_NUM_ANALOG_ACCUMULATORS 2

#define MISC_MAX_NUM_OVERSAMPLE_BITS 6
#define MISC_MAX_NUM_AVERAGE_BITS MISC_MAX_NUM_OVERSAMPLE_BITS

#define EXTPOWER_INPUT_VDIV_RATIO (3.24f / (11.5f + 3.24f))

typedef struct {
    uint8_t enabled 		: 1;
    uint8_t anin_channel	: 2;
    uint8_t unused			: 4;
    uint8_t reset			: 1;
} ACCUMULATOR_CONFIG_REG;

struct __attribute__ ((__packed__)) MISC_REGS {
	/****************************/
	/* Capabilities (Read-only) */
	/****************************/
	MISC_CAPABILITY_FLAGS capability_flags;

	/**********************/
	/* Status (Read-only) */
	/**********************/
	/* Power */
	MISC_EXT_PWR_CTL_STATUS ext_pwr_ctl_status;
	uint16_t ext_pwr_voltage_value;
	/* RTC */
	MISC_RTC_TIME rtc_time;
	MISC_RTC_DATE rtc_date;
	/* Analog Trigger */
	uint8_t analog_trigger_state[MISC_NUM_ANALOG_TRIGGERS]; /* ANALOG_TRIGGER_STATE */
	/* Analog Input */
	uint32_t analog_input_oversample_value[MISC_NUM_ANALOG_INPUTS];
	uint32_t analog_input_average_value[MISC_NUM_ANALOG_INPUTS];

	/************************************/
	/* Basic Configuration (Read/Write) */
	/************************************/
	/* Power */
	MISC_EXT_PWR_CTL_CFG ext_pwr_ctl_cfg;
	/* RTC */
	MISC_RTC_CFG rtc_cfg;
	MISC_RTC_TIME rtc_time_set;
	MISC_RTC_DATE rtc_date_set;
	/* Analog Trigger Configuration */
	uint8_t analog_trigger_cfg[MISC_NUM_ANALOG_TRIGGERS]; /* ANALOG_TRIGGER_MODE */
	uint16_t analog_trigger_threshold_low[MISC_NUM_ANALOG_TRIGGERS];
	uint16_t analog_trigger_threshold_high[MISC_NUM_ANALOG_TRIGGERS];
	/* Analog Input Oversampling/Averaging Accumulation Configuration */
	uint8_t analog_input_oversample_bits[MISC_NUM_ANALOG_INPUTS];
	uint8_t analog_input_average_bits[MISC_NUM_ANALOG_INPUTS];
	/* Analog Accumulator Counter Configuration */
	ACCUMULATOR_CONFIG_REG analog_accumulator_cfg[MISC_NUM_ANALOG_ACCUMULATORS];
	int16_t analog_accumulator_center[MISC_NUM_ANALOG_ACCUMULATORS];
	int16_t analog_accumulator_deadband[MISC_NUM_ANALOG_ACCUMULATORS];
	/* Analog Accumulator Counter Data (read-only) */
	uint32_t analog_accumulator_count[MISC_NUM_ANALOG_ACCUMULATORS];
	int64_t analog_accumulator_value[MISC_NUM_ANALOG_ACCUMULATORS];
};

#endif /* MISCREGISTERS_H_ */
