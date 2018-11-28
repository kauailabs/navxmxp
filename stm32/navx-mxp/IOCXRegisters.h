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

#ifndef IO_CONTROL_REGISTERS_H_
#define IO_CONTROL_REGISTERS_H_

#include <stddef.h>

/***********************************************/
/* IO Control Registers (Bank 1 [second bank]) */
/***********************************************/

#define IOCX_REGISTER_BANK 1

#define DEPRECATED_INPUTCAP_REGISTERS 1

typedef struct {
	uint16_t unused			: 14;
	uint16_t rpi_gpio_out	: 1;  /* If set, the 10-pin RPI GPIO Header is OUTPUT */
	uint16_t interrupt_support : 1;
} IOCX_CAPABILITY_FLAGS;

#define IOCX_GPIO_SET	1
#define IOCX_GPIO_RESET 0

// GPIO_TYPE
typedef enum _IOCX_GPIO_TYPE {
	GPIO_TYPE_DISABLED			= 0, /* Default */
	GPIO_TYPE_INPUT 			= 1,
	GPIO_TYPE_OUTPUT_PUSHPULL 	= 2,
	GPIO_TYPE_OUTPUT_OPENDRAIN	= 3, /* Reserved for Later Use */
	GPIO_TYPE_AF 				= 4, /* Timer-capable GPIOs:  Timer Channel; Trigger-capable GPIOs:  Trigger */
} IOCX_GPIO_TYPE;

typedef enum _IOCX_GPIO_INPUT {
	GPIO_INPUT_FLOAT	= 0, /* Default */
	GPIO_INPUT_PULLUP	= 1,
	GPIO_INPUT_PULLDOWN	= 2,
} IOCX_GPIO_INPUT;

typedef enum _IOCX_GPIO_INTERRUPT {
	GPIO_INTERRUPT_DISABLED		= 0, /* Default */
	GPIO_INTERRUPT_RISING_EDGE	= 1,
	GPIO_INTERRUPT_FALLING_EDGE	= 2,
	GPIO_INTERRUPT_BOTH_EDGES	= 3,
} IOCX_GPIO_INTERRUPT;

typedef struct {
	size_t  offset;
	uint8_t start_bit;
	uint8_t mask;
} RegEncoding;

/* GPIO MAP: */
/* GPIO_0:  DIO Header 1; Timer5, Chan1 */
/* GPIO_1:  DIO Header 2; Timer5, Chan2 */
/* GPIO_2:  DIO Header 3; Timer6, Chan1 */
/* GPIO_3:  DIO Header 4; Timer6, Chan2 */
/* GPIO_4:  QE1 Connector A; Timer1, Chan1 */
/* GPIO_5:  QE1 Connector B; Timer1, Chan2 */
/* GPIO_6:  QE2 Connector A; Timer2, Chan1 */
/* GPIO_7:  QE2 Connector B; Timer2, Chan2 */
/* GPIO_8:  QE3 Connector A; Timer3, Chan1 */
/* GPIO_9:  QE3 Connector B; Timer3, Chan2 */
/* GPIO_10: QE4 Connector A; Timer4, Chan1 */
/* GPIO_11: QE4 Connector B; Timer4, Chan2 */
#define IOCX_NUM_GPIOS 12

#define IOCX_NUM_INTERRUPTS      16
/* INTERRUPT MAP:  */
/* INT0:  DIO Connector 1  */
/* INT1:  DIO Connector 2  */
/* INT2:  DIO Connector 3  */
/* INT3:  DIO Connector 4  */
/* INT4:  Encoder 1, Ch A  */
/* INT5:  Encoder 1, Ch B  */
/* INT6:  Encoder 2, Ch A  */
/* INT7:  Encoder 2, Ch B  */
/* INT8:  Encoder 3, Ch A  */
/* INT9:  Encoder 3, Ch B  */
/* INT10: Encoder 4, Ch A  */
/* INT11: Encoder 4, Ch B  */
/* INT12: Analog Trigger 1 */
/* INT13: Analog Trigger 2 */
/* INT14: Analog Trigger 3 */
/* INT15: Analog Trigger 4 */
#define GPIO_NUMBER_TO_INT_BIT(NUM) 			(1<<(NUM))
#define ANALOG_TRIGGER_NUMBER_TO_INT_BIT(NUM) 	(1<<(NUM+IOCX_NUM_GPIOS))

/* Timer Map: */
/* Timer1:  STM32 TIM1 */
/* Timer2:  STM32 TIM2 */
/* Timer3:  STM32 TIM3 */
/* Timer4:  STM32 TIM4 */
/* Timer5:  STM32 TIM5 */
/* Timer6:  STM32 TIM9 */
#define IOCX_NUM_TIMERS 6
#define IOCX_NUM_CHANNELS_PER_TIMER 2

typedef enum _IOCX_TIMER_MODE {
	TIMER_MODE_DISABLED 		= 0, /* Default */
	TIMER_MODE_QUAD_ENCODER		= 1,
	TIMER_MODE_PWM_OUT			= 2,
	TIMER_MODE_INPUT_CAPTURE	= 3,
#ifdef DEPRECATED_INPUTCAP_REGISTERS
	TIMER_MODE_PWM_CAPTURE		= TIMER_MODE_INPUT_CAPTURE
#endif // DEPRECATED_INPUTCAP_REGISTERS
} IOCX_TIMER_MODE;

typedef enum _IOCX_QUAD_ENCODER_MODE {
	QUAD_ENCODER_MODE_4x		= 0, /* Default */
	QUAD_ENCODER_MODE_2x		= 1,
	QUAD_ENCODER_MODE_1x		= 2,
} IOCX_QUAD_ENCODER_MODE;

#ifdef DEPRECATED_INPUTCAP_REGISTERS
typedef enum _IOCX_INPUT_CAPTURE_CHANNEL {
	INPUT_CAPTURE_FROM_CH1		= 0, /* A Input, Default */
	INPUT_CAPTURE_FROM_CH2		= 1, /* B Input, Default */
} IOCX_INPUT_CAPTURE_CHANNEL;

typedef enum _IOCX_INPUT_CAPTURE_POLARITY {
	INPUT_CAPTURE_POLARITY_RISING  = 0, /* Default */
	INPUT_CAPTURE_POLARITY_FALLING = 1
} IOCX_INPUT_CAPTURE_POLARITY;

typedef enum _IOCX_PWM_CAPTURE_TIMEOUT {
	PWM_CAPTURE_TIMEOUT_NONE = 0,
	PWM_CAPTURE_TIMEOUT_1X = 1,
	PWM_CAPTURE_TIMEOUT_2X = 2,
	PWM_CAPTURE_TIMEOUT_3X = 3,
} IOCX_PWM_CAPTURE_TIMEOUT;
#endif

typedef enum _IOCX_TIMER_DIRECTION {
	UP				= 0,
	DOWN		    = 1,
} IOCX_TIMER_DIRECTION;

typedef enum _IOCX_INPUTCAP_CH_ACTIVITY {
	TIMER_INPUT_CH_STALLED  = 0,
	TIMER_INPUT_CH_ACTIVE	= 1,
} IOCX_INPUTCAP_CH_ACTIVITY;

typedef enum _IOCX_TIMER_COUNTER_RESET {
	NORMAL			= 0,
	RESET_REQUEST	= 1,
} IOCX_TIMER_COUNTER_RESET;

#define IOCX_ALL_TIMER_MODES 				\
	((1 << TIMER_MODE_QUAD_ENCODER) || 		\
	 (1 << TIMER_MODE_PWM_OUT) ||			\
	 (1 << TIMER_MODE_PWM_CAPTURE))

#define IOCX_NON_QUAD_ENCODER_TIMER_MODES	\
		((1 << TIMER_MODE_PWM_OUT) ||			\
		 (1 << TIMER_MODE_PWM_CAPTURE))

const uint8_t SUPPORTED_TIMER_MODES[IOCX_NUM_TIMERS] = {
	IOCX_ALL_TIMER_MODES,
	IOCX_ALL_TIMER_MODES,
	IOCX_ALL_TIMER_MODES,
	IOCX_ALL_TIMER_MODES,
	IOCX_ALL_TIMER_MODES,
	IOCX_NON_QUAD_ENCODER_TIMER_MODES
};

// Todo:  for each timer, get supported IOCX_TIMER_MODES (3 bits/timer)
#define IOCX_SUPPORTED_TIMER_MODES_ALL (1 << TIMER_MODE_QUAD_ENCODER)

struct __attribute__ ((__packed__)) IOCX_REGS {
	/****************/
	/* Capabilities */
	/****************/
	IOCX_CAPABILITY_FLAGS capability_flags;
	/*****************/
	/* Configuration */
	/*****************/
	uint8_t timer_cfg[IOCX_NUM_TIMERS]; /* IOCX_TIMER_MODE, IOCX_QUAD_ENCODER_MODE */
	uint8_t timer_ctl[IOCX_NUM_TIMERS]; /* IOCX_TIMER_COUNTER_RESET */
	uint16_t timer_prescaler[IOCX_NUM_TIMERS]; /* Timer Frequency (48Mhz divider) */
	uint16_t timer_aar[IOCX_NUM_TIMERS]; /* PWM:  Frame Period; QE:  auto-set to 0xFFFF */
	/* PWM Mode:  Duty-cycle Period; QE: Mode:  read-only, internal use */
	uint16_t timer_chx_ccr[IOCX_NUM_TIMERS * IOCX_NUM_CHANNELS_PER_TIMER];
	uint16_t int_cfg;			/* Mask for:  GPIO/Analog Trigger Interrupts  */
	uint8_t  gpio_cfg[IOCX_NUM_GPIOS];	 /* IOCX_GPIO_TYPE, _INPUT, _INTERRUPT */
	/*****************/
	/* Data/Status   */
	/*****************/
	uint16_t gpio_intstat;  			 /* Bitmask:  1 = int present.  Write 1 to clear. */
	uint16_t gpio_last_int_edge;		 /* Bitmask:  1 = last edge rising; 0:  last edge falling */
	uint8_t  gpio_data[IOCX_NUM_GPIOS];  /* IOCX_GPIO_SET = High, IOCX_GPIO_RESET = Low.  */
	uint8_t timer_status[IOCX_NUM_TIMERS]; /* IOCX_TIMER_DIRECTION, IOCX_INPUTCAP_CH_ACTIVITY */
	int32_t timer_counter[IOCX_NUM_TIMERS]; /* QE Mode:  Encoder Counts */
	uint8_t end_of_bank;
};

static RegEncoding gpio_type_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 0, 0x07 };
static RegEncoding gpio_input_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 3, 0x03 };
static RegEncoding gpio_interrupt_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 5, 0x03 };
static RegEncoding timer_mode_reg = { offsetof(struct IOCX_REGS, timer_cfg), 0, 0x03 };
static RegEncoding quad_enc_mode_reg = {offsetof(struct IOCX_REGS, timer_cfg), 2, 0x03 };
#ifdef DEPRECATED_INPUTCAP_REGISTERS
static RegEncoding input_cap_ch_reg = {offsetof(struct IOCX_REGS, timer_cfg), 4, 0x01 };
static RegEncoding input_cap_polarity_reg = {offsetof(struct IOCX_REGS, timer_cfg), 5, 0x01};
static RegEncoding pwm_cap_timeout_reg = {offsetof(struct IOCX_REGS, timer_cfg), 6, 0x03};
#endif // DEPRECATED_INPUTCAP_REGISTERS
static RegEncoding timer_counter_reset_reg = { offsetof(struct IOCX_REGS, timer_ctl), 0, 0x01 };
static RegEncoding timer_direction_reg = { offsetof(struct IOCX_REGS, timer_status), 0, 0x01 };
static RegEncoding timer_in_ch_activity_reg = { offsetof(struct IOCX_REGS, timer_status), 1, 0x01 };

inline void clear_reg( uint8_t *reg, RegEncoding *encoding){
	*reg &= ~(encoding->mask << encoding->start_bit);
}

inline void encode_reg( uint8_t *reg, uint8_t value, RegEncoding* encoding){
	clear_reg(reg, encoding);
	*reg |= ((value & encoding->mask) << encoding->start_bit);
}

inline uint8_t decode_reg( uint8_t *reg, RegEncoding *encoding){
	return (*reg >> encoding->start_bit) & encoding->mask;
}

inline void iocx_encode_gpio_type(uint8_t* reg, IOCX_GPIO_TYPE val) {
	encode_reg(reg, (uint8_t)val, &gpio_type_reg);
}
inline IOCX_GPIO_TYPE iocx_decode_gpio_type(uint8_t *reg) {
	return (IOCX_GPIO_TYPE)decode_reg(reg, &gpio_type_reg);
}
inline void iocx_encode_gpio_input(uint8_t* reg, IOCX_GPIO_INPUT val) {
	encode_reg(reg, (uint8_t)val, &gpio_input_reg);
}
inline IOCX_GPIO_INPUT iocx_decode_gpio_input(uint8_t *reg) {
	return (IOCX_GPIO_INPUT)decode_reg(reg, &gpio_input_reg);
}
inline void iocx_encode_gpio_interrupt(uint8_t* reg, IOCX_GPIO_INTERRUPT val) {
	encode_reg(reg, (uint8_t)val, &gpio_interrupt_reg);
}
inline IOCX_GPIO_INTERRUPT iocx_decode_gpio_interrupt(uint8_t *reg) {
	return (IOCX_GPIO_INTERRUPT)decode_reg(reg, &gpio_interrupt_reg);
}
inline void iocx_encode_timer_mode(uint8_t* reg, IOCX_TIMER_MODE val) {
	encode_reg(reg, (uint8_t)val, &timer_mode_reg);
}
inline IOCX_TIMER_MODE iocx_decode_timer_mode(uint8_t *reg) {
	return (IOCX_TIMER_MODE)decode_reg(reg, &timer_mode_reg);
}
inline void iocx_encode_quad_encoder_mode(uint8_t* reg, IOCX_QUAD_ENCODER_MODE val) {
	encode_reg(reg, (uint8_t)val, &quad_enc_mode_reg);
}
inline IOCX_QUAD_ENCODER_MODE iocx_decode_quad_encoder_mode(uint8_t *reg) {
	return (IOCX_QUAD_ENCODER_MODE)decode_reg(reg, &quad_enc_mode_reg);
}
#ifdef DEPRECATED_INPUTCAP_REGISTERS
inline void iocx_encode_input_capture_channel(uint8_t* reg, IOCX_INPUT_CAPTURE_CHANNEL val) {
	encode_reg(reg, (uint8_t)val, &input_cap_ch_reg);
}
inline IOCX_INPUT_CAPTURE_CHANNEL iocx_decode_input_capture_channel(uint8_t *reg) {
	return (IOCX_INPUT_CAPTURE_CHANNEL)decode_reg(reg, &input_cap_ch_reg);
}
inline void iocx_encode_input_capture_polarity(uint8_t* reg, IOCX_INPUT_CAPTURE_POLARITY val) {
	encode_reg(reg, (uint8_t)val, &input_cap_polarity_reg);
}
inline IOCX_INPUT_CAPTURE_POLARITY iocx_decode_input_capture_polarity(uint8_t *reg) {
	return (IOCX_INPUT_CAPTURE_POLARITY)decode_reg(reg, &input_cap_polarity_reg);
}
inline void iocx_encode_pwm_capture_timeout(uint8_t* reg, IOCX_PWM_CAPTURE_TIMEOUT val) {
	encode_reg(reg, (uint8_t)val, &pwm_cap_timeout_reg);
}
inline IOCX_PWM_CAPTURE_TIMEOUT iocx_decode_pwm_capture_timeout(uint8_t *reg) {
	return (IOCX_PWM_CAPTURE_TIMEOUT)decode_reg(reg, &pwm_cap_timeout_reg);
}
#endif // DEPRECATED_INPUTCAP_REGISTERS
inline void iocx_encode_timer_direction(uint8_t *reg, IOCX_TIMER_DIRECTION val) {
	encode_reg(reg, (uint8_t)val, &timer_direction_reg);
}
inline IOCX_TIMER_DIRECTION iocx_decode_timer_direction(uint8_t *reg) {
	return (IOCX_TIMER_DIRECTION)decode_reg(reg, &timer_direction_reg);
}
inline void iocx_encode_timer_in_ch_activity(uint8_t *reg, IOCX_INPUTCAP_CH_ACTIVITY val) {
	encode_reg(reg, (uint8_t)val, &timer_in_ch_activity_reg);
}
inline IOCX_INPUTCAP_CH_ACTIVITY iocx_decode_timer_in_ch_activity(uint8_t *reg) {
	return (IOCX_INPUTCAP_CH_ACTIVITY)decode_reg(reg, &timer_in_ch_activity_reg);
}
inline void iocx_encode_timer_counter_reset(uint8_t *reg, IOCX_TIMER_COUNTER_RESET val) {
	encode_reg(reg, (uint8_t)val, &timer_counter_reset_reg);
}
inline IOCX_TIMER_COUNTER_RESET iocx_decode_timer_counter_reset(uint8_t *reg) {
	return (IOCX_TIMER_COUNTER_RESET)decode_reg(reg, &timer_counter_reset_reg);
}
inline void iocx_clear_timer_counter_reset(uint8_t *reg) {
	clear_reg(reg, &timer_counter_reset_reg);
}

#endif /* IO_CONTROL_REGISTERS_H_ */
