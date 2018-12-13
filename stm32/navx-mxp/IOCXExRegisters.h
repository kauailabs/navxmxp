/* ============================================
navX MXP source code is placed under the MIT license
Copyright (c) 2015 Kauai Labs

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

/* The IOCXEx (IOCX Extended) Register Bank is an extension of the IOCX Registers,
 * including additional features that could not be fully contained within the
 * IOCX Register Bank.
 */

#ifndef IOCX_EX_REGISTERS_H_
#define IOCX_EX_REGISTERS_H_

#include <stddef.h>
#include "IOCXRegisters.h"

/**************************************************/
/* IOCX Ex Register Bank (Bank 4 [fifth bank])    */
/**************************************************/

#define IOCX_EX_REGISTER_BANK 4

typedef struct {
	uint16_t unused				: 11;
	uint16_t tmrcntreset_support: 1;  /* 1:  Timer Counter can be reset by interrupt.   */
	uint16_t countercfg_support : 1;  /* 1:  Adv Timer Counter configuration supported. */
	uint16_t slvmd_cfg_support  : 1;  /* 1:  Timer Slave Mode configuration supported.  */
	uint16_t stall_support		: 1;  /* 1:  Input cap stall detection supported.       */
	uint16_t inputcap_support	: 1;  /* 1:  Input cap cfg/ch1 & ch2 cfg are supported. */
} IOCX_EX_CAPABILITY_FLAGS;

typedef enum _TIMER_COUNTER_CLK_SOURCE {
	TIMER_COUNTER_CLK_INTERNAL 			= 0, /* Internal 48Mhz Clock [DEFAULT] */
	TIMER_COUNTER_CLK_EDGEDETECT_CH1	= 4, /* Unfiltered Input Ch1 Edges */
	TIMER_COUNTER_CLK_FILTERED_CH1		= 5, /* Filtered Input Ch1 */
	TIMER_COUNTER_CLK_FILTERED_CH2		= 6, /* Filtered Input Ch2 */
} TIMER_COUNTER_CLK_SOURCE;

typedef enum _TIMER_COUNTER_DIRECTION {
	TIMER_COUNTER_DIRECTION_UP = 0, /* Default */
	TIMER_COUNTER_DIRECTION_DN = 1,
} TIMER_COUNTER_DIRECTION;

typedef enum _TIMER_COUNTER_INTERRUPT_RESET {
	TIMER_COUNTER_INTERRUPT_RESET_DISABLED = 0,
	TIMER_COUNTER_INTERRUPT_RESET_ENABLED = 1, // Counter reset when interrupt occurs
} TIMER_COUNTER_INTERRUPT_RESET;

typedef enum _TIMER_COUNTER_LEVEL_RESET {
	TIMER_COUNTER_LEVEL_RESET_NONE = 0,
	TIMER_COUNTER_LEVEL_RESET_HIGH = 1,  // Counter returns 0 when level is HIGH
	TIMER_COUNTER_LEVEL_RESET_LOW = 2,   // Counter returns 0 when level is LOW
} TIMER_COUNTER_LEVEL_RESET;

typedef enum _TIMER_SLAVE_MODE {
	TIMER_SLAVE_MODE_DISABLED	= 0, /* Default */
	TIMER_SLAVE_MODE_RESET		= 4,
	TIMER_SLAVE_MODE_GATED		= 5,
	TIMER_SLAVE_MODE_TRIGGER	= 6,
} TIMER_SLAVE_MODE;

typedef enum _TIMER_SLAVE_MODE_TRIGGER_SOURCE {
	TIMER_SLAVE_MODE_TRIGGER_EDGEDETECT_CH1	= 4,
	TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1	= 5, /* Default */
	TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH2	= 6,
} TIMER_SLAVE_MODE_TRIGGER_SOURCE;

typedef enum _TIMER_INPUT_CAPTURE_CH_SOURCE {
	INPUT_CAPTURE_FROM_SIGNAL_A	= 0, /* Default for Ch1 */
	INPUT_CAPTURE_FROM_SIGNAL_B	= 1, /* Default for Ch2 */
} TIMER_INPUT_CAPTURE_CH_SOURCE;

typedef enum _TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE {
	INPUT_CAPTURE_CH_ACTIVE_RISING	= 0, /* Default */
	INPUT_CAPTURE_CH_ACTIVE_FALLING	= 1,
	INPUT_CAPTURE_CH_ACTIVE_BOTH	= 2
} TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE;

typedef enum _TIMER_INPUT_CAPTURE_CH_PRESCALER {
	INPUT_CAPTURE_PRESCALE_1x = 0, /* Default */
	INPUT_CAPTURE_PRESCALE_2x = 1,
	INPUT_CAPTURE_PRESCALE_4x = 2,
	INPUT_CAPTURE_PRESCALE_8x = 3,
} TIMER_INPUT_CAPTURE_CH_PRESCALER;

typedef enum _TIMER_INPUT_CAPTURE_CH_FILTER {
	INPUT_CAPTURE_CH_FILTER_NONE =  0,	/* 48Mhz     (0.02us); 96Mhz on TIM1, TIM9 (Default) */
	INPUT_CAPTURE_CH_FILTER_2x	 =  1,	/* 24Mhz     (0.04us); 48Mhz on TIM1, TIM9 */
	INPUT_CAPTURE_CH_FILTER_4x	 =  2,	/* 12Mhz     (0.08us); 24Mhz on TIM1, TIM9 */
	INPUT_CAPTURE_CH_FILTER_8x	 =  3,	/*  6Mhz     (0.17us); 12Mhz on TIM1, TIM9 */
	INPUT_CAPTURE_CH_FILTER_12x	 =  4,	/*  4Mhz     (0.25us) */
	INPUT_CAPTURE_CH_FILTER_16x	 =  5,	/*  3Mhz     (0.3us) */
	INPUT_CAPTURE_CH_FILTER_24x	 =  6,	/*  2Mhz     (0.4us) */
	INPUT_CAPTURE_CH_FILTER_32x  =  7,	/*  1.5Mhz   (0.7us) */
	INPUT_CAPTURE_CH_FILTER_48x  =  8,	/*  1Mhz     (1.0us) */
	INPUT_CAPTURE_CH_FILTER_64x  =  9,	/*  750Khz   (1.3us) */
	INPUT_CAPTURE_CH_FILTER_80x  = 10,	/*  600Khz   (1.7us) */
	INPUT_CAPTURE_CH_FILTER_96x  = 11,  /*  500Khz   (2 us) */
	INPUT_CAPTURE_CH_FILTER_128x = 12,	/*  375Khz   (2.7us) */
	INPUT_CAPTURE_CH_FILTER_160x = 13,  /*  300Khz   (3.3us) */
	INPUT_CAPTURE_CH_FILTER_192x = 14,  /*  250Khz   (4.0us) */
	INPUT_CAPTURE_CH_FILTER_256x = 15,  /*  187.5Khz (5.3us) */
} TIMER_INPUT_CAPTURE_CH_FILTER;

typedef enum _TIMER_INPUT_CAPTURE_STALL_ACTION {
	TIMER_STALL_ACTION_NONE				= 0,
	TIMER_STALL_ACTION_CLEAR_COUNTER	= 1,
} TIMER_INPUT_CAPTURE_STALL_ACTION;

/* Timer Input Channel Stall Timeout */
#define TIMER_INPUT_CH_STALL_TIMEOUT_DISABLED	0
#define TIMER_INPUT_CH_STALL_TIMEOUT_MIN		0
#define TIMER_INPUT_CH_STALL_TIMEOUT_MAX		127
#define TIMER_INPUT_CH_SHALL_TIMEOUT_UNIT_MS	20

struct __attribute__ ((__packed__)) IOCX_EX_REGS {
	/****************/
	/* Capabilities */
	/****************/
	IOCX_EX_CAPABILITY_FLAGS capability_flags;
	/* TIMER_COUNTER_CLK_SOURCE, DIRECTION */
	uint8_t timer_counter_cfg[IOCX_NUM_TIMERS];
	/* TIMER_SLAVE_MODE, TIMER_SLAVE_MODE_TRIGGER_SOURCE */
	uint8_t timer_slavemode_cfg[IOCX_NUM_TIMERS];
	/* TIMER_INPUT_CAPTURE_STALL_ACTION, stall timeout */
	uint8_t timer_ic_stall_cfg[IOCX_NUM_TIMERS];
	/* TIMER_INPUT_CAPTURE_CH_SOURCE, ACTIVE_EDGE, PRESCALER */
	uint8_t timer_ic_ch_cfg[IOCX_NUM_TIMERS * IOCX_NUM_CHANNELS_PER_TIMER];
	/* TIMER_INPUT_CAPTURE_CH_FILTER, TIMER_STALL_CTL */
	uint8_t timer_ic_ch_cfg2[IOCX_NUM_TIMERS * IOCX_NUM_CHANNELS_PER_TIMER];
	/* TIMER_COUNTER_INTERRUPT_RESET, Interrupt Reset Source (4 bits)   */
	/* Resource Source is Interrupt Index, 0 to (IOCX_NUM_INTERRUPTS-1) */
	uint8_t timer_counter_reset_cfg[IOCX_NUM_TIMERS];
	/*****************/
	/* Configuration */
	/*****************/
	uint8_t end_of_bank;
};

static RegEncoding timer_counter_clk_src_reg = { offsetof(struct IOCX_EX_REGS, timer_counter_cfg), 0, 0x07 };
static RegEncoding timer_counter_direction_reg = { offsetof(struct IOCX_EX_REGS, timer_counter_cfg), 4, 0x01 };
static RegEncoding timer_slavemode_reg = { offsetof(struct IOCX_EX_REGS, timer_slavemode_cfg), 1, 0x07 };
static RegEncoding timer_slavemode_trigger_src_reg = { offsetof(struct IOCX_EX_REGS, timer_slavemode_cfg), 4, 0x07 };
static RegEncoding timer_ic_ch_src_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_ch_cfg), 0, 0x01 };
static RegEncoding timer_ic_ch_active_edge_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_ch_cfg), 2, 0x03 };
static RegEncoding timer_ic_ch_prescaler_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_ch_cfg), 4, 0x03 };
static RegEncoding timer_ic_ch_filter_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_ch_cfg2), 0, 0x0F };
static RegEncoding timer_ic_stall_timeout_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_stall_cfg), 0, 0x7F };
static RegEncoding timer_ic_stall_action_reg = { offsetof(struct IOCX_EX_REGS, timer_ic_stall_cfg), 7, 0x01 };
static RegEncoding timer_counter_interrupt_reset_mode_reg = { offsetof(struct IOCX_EX_REGS, timer_counter_reset_cfg), 0, 0x01 };
static RegEncoding timer_counter_level_reset_mode_reg = { offsetof(struct IOCX_EX_REGS, timer_counter_reset_cfg), 1, 0x03 };
static RegEncoding timer_counter_reset_src_reg = { offsetof(struct IOCX_EX_REGS, timer_counter_reset_cfg), 4, 0x0F };

inline void iocx_ex_timer_encode_counter_clk_src(uint8_t* reg, TIMER_COUNTER_CLK_SOURCE val) {
	encode_reg(reg, (uint8_t)val, &timer_counter_clk_src_reg);
}
inline TIMER_COUNTER_CLK_SOURCE iocx_ex_timer_decode_counter_clk_src(uint8_t *reg) {
	return (TIMER_COUNTER_CLK_SOURCE)decode_reg(reg, &timer_counter_clk_src_reg);
}
inline void iocx_ex_timer_encode_counter_direction(uint8_t* reg, TIMER_COUNTER_DIRECTION val) {
	encode_reg(reg, (uint8_t)val, &timer_counter_direction_reg);
}
inline TIMER_COUNTER_DIRECTION iocx_ex_timer_decode_counter_direction(uint8_t *reg) {
	return (TIMER_COUNTER_DIRECTION)decode_reg(reg, &timer_counter_direction_reg);
}
inline void iocx_ex_timer_encode_slavemode(uint8_t* reg, TIMER_SLAVE_MODE val) {
	encode_reg(reg, (uint8_t)val, &timer_slavemode_reg);
}
inline TIMER_SLAVE_MODE iocx_ex_timer_decode_slavemode(uint8_t *reg) {
	return (TIMER_SLAVE_MODE)decode_reg(reg, &timer_slavemode_reg);
}
inline void iocx_ex_timer_encode_slavemode_trigger_source(uint8_t* reg, TIMER_SLAVE_MODE_TRIGGER_SOURCE val) {
	encode_reg(reg, (uint8_t)val, &timer_slavemode_trigger_src_reg);
}
inline TIMER_SLAVE_MODE_TRIGGER_SOURCE iocx_ex_timer_decode_slavemode_trigger_source(uint8_t *reg) {
	return (TIMER_SLAVE_MODE_TRIGGER_SOURCE)decode_reg(reg, &timer_slavemode_trigger_src_reg);
}
inline void iocx_ex_timer_encode_ic_ch_src(uint8_t* reg, TIMER_INPUT_CAPTURE_CH_SOURCE val) {
	encode_reg(reg, (uint8_t)val, &timer_ic_ch_src_reg);
}
inline TIMER_INPUT_CAPTURE_CH_SOURCE iocx_ex_timer_decode_ic_ch_src(uint8_t *reg) {
	return (TIMER_INPUT_CAPTURE_CH_SOURCE)decode_reg(reg, &timer_ic_ch_src_reg);
}
inline void iocx_ex_timer_encode_ic_ch_active_edge(uint8_t* reg, TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE val) {
	encode_reg(reg, (uint8_t)val, &timer_ic_ch_active_edge_reg);
}
inline TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE iocx_ex_timer_decode_ic_ch_active_edge(uint8_t *reg) {
	return (TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE)decode_reg(reg, &timer_ic_ch_active_edge_reg);
}
inline void iocx_ex_timer_encode_ic_ch_prescaler(uint8_t* reg, TIMER_INPUT_CAPTURE_CH_PRESCALER val) {
	encode_reg(reg, (uint8_t)val, &timer_ic_ch_prescaler_reg);
}
inline TIMER_INPUT_CAPTURE_CH_PRESCALER iocx_ex_timer_decode_ic_ch_prescaler(uint8_t *reg) {
	return (TIMER_INPUT_CAPTURE_CH_PRESCALER)decode_reg(reg, &timer_ic_ch_prescaler_reg);
}
inline void iocx_ex_timer_encode_ic_ch_filter(uint8_t* reg, TIMER_INPUT_CAPTURE_CH_FILTER val) {
	encode_reg(reg, (uint8_t)val, &timer_ic_ch_filter_reg);
}
inline TIMER_INPUT_CAPTURE_CH_FILTER iocx_ex_timer_decode_ic_ch_filter(uint8_t *reg) {
	return (TIMER_INPUT_CAPTURE_CH_FILTER)decode_reg(reg, &timer_ic_ch_filter_reg);
}
inline void iocx_ex_timer_encode_ic_stall_timeout(uint8_t* reg, uint8_t val) {
	encode_reg(reg, val, &timer_ic_stall_timeout_reg);
}
inline uint8_t iocx_ex_timer_decode_ic_stall_timeout(uint8_t *reg) {
	return decode_reg(reg, &timer_ic_stall_timeout_reg);
}
inline void iocx_ex_timer_encode_ic_stall_action(uint8_t* reg, TIMER_INPUT_CAPTURE_STALL_ACTION val) {
	encode_reg(reg, (uint8_t)val, &timer_ic_stall_action_reg);
}
inline TIMER_INPUT_CAPTURE_STALL_ACTION iocx_ex_timer_decode_ic_stall_action(uint8_t *reg) {
	return (TIMER_INPUT_CAPTURE_STALL_ACTION)decode_reg(reg, &timer_ic_stall_action_reg);
}
inline void iocx_ex_timer_encode_counter_interrupt_reset_mode(uint8_t *reg, TIMER_COUNTER_INTERRUPT_RESET val) {
	encode_reg(reg, (uint8_t)val, &timer_counter_interrupt_reset_mode_reg);
}
inline TIMER_COUNTER_INTERRUPT_RESET iocx_ex_timer_decode_counter_interrupt_reset_mode(uint8_t *reg) {
	return (TIMER_COUNTER_INTERRUPT_RESET)decode_reg(reg, &timer_counter_interrupt_reset_mode_reg);
}
inline void iocx_ex_timer_encode_counter_level_reset_mode(uint8_t *reg, TIMER_COUNTER_LEVEL_RESET val) {
	encode_reg(reg, (uint8_t)val, &timer_counter_level_reset_mode_reg);
}
inline TIMER_COUNTER_LEVEL_RESET iocx_ex_timer_decode_counter_level_reset_mode(uint8_t *reg) {
	return (TIMER_COUNTER_LEVEL_RESET)decode_reg(reg, &timer_counter_level_reset_mode_reg);
}
inline void iocx_ex_timer_encode_counter_reset_source(uint8_t* reg, uint8_t val) {
	encode_reg(reg, val, &timer_counter_reset_src_reg);
}
inline uint8_t iocx_ex_timer_decode_counter_reset_source(uint8_t *reg) {
	return decode_reg(reg, &timer_counter_reset_src_reg);
}

#endif /* IOCX_EX_REGISTERS_H_ */
