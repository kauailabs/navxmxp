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
/* GPIO1:  Board PWM/GPIO 1; Timer5, Chan1 */
/* GPIO2:  Board PWM/GPIO 2; Timer5, Chan2 */
/* GPIO3:  Board PWM/GPIO 3; Timer6, Chan1 */
/* GPIO4:  Board PWM/GPIO 3; Timer6, Chan2 */
/* GPIO5:  QE1 IDX; 		               */ /* [REMOVE] */
/* GPIO6:  QE1 A; 			 Timer1, Chan1 */
/* GPIO7:  QE1 B; 			 Timer1, Chan2 */
/* GPIO8:  QE2 IDX; 		               */ /* [REMOVE] */
/* GPIO9:  QE2 A; 			 Timer2, Chan1 */
/* GPIO10: QE2 B; 			 Timer2, Chan2 */
/* GPIO11: QE3 IDX; 		               */ /* [REMOVE] */
/* GPIO12: QE3 A; 			 Timer3, Chan1 */
/* GPIO13: QE3 B; 			 Timer3, Chan2 */
/* GPIO14: QE4 IDX; 		               */ /* [REMOVE] */
/* GPIO15: QE4 A; 			 Timer4, Chan1 */
/* GPIO16: QE4 B; 			 Timer4, Chan2 */
#define IOCX_NUM_GPIOS 16

typedef enum {
	ANALOG_TRIGGER_LOW,
	ANALOG_TRIGGER_HIGH,
	ANALOG_TRIGGER_IN_WINDOW
} ANALOG_TRIGGER_STATE;

typedef enum {
	ANALOG_TRIGGER_DISABLED,
	ANALOG_TRIGGER_MODE_STATE,
	ANALOG_TRIGGER_MODE_RISING_EDGE_PULSE, /* Internal routing to Interrupt only */
	ANALOG_TRIGGER_MODE_FALLING_EDGE_PULSE /* Internal routing to Interrupt only */
} ANALOG_TRIGGER_MODE;

#define IOCX_NUM_ANALOG_TRIGGERS 4
#define IOCX_NUM_INTERRUPTS      16
/* INTERRUPT MAP:  */
/* INT1:  DIO Connector 1  */
/* INT2:  DIO Connector 2  */
/* INT3:  DIO Connector 3  */
/* INT4:  DIO Connector 4  */
/* INT5:  Encoder 1, Ch A  */
/* INT6:  Encoder 1, Ch B  */
/* INT7:  Encoder 2, Ch A  */
/* INT8:  Encoder 2, Ch B  */
/* INT9:  Encoder 3, Ch A  */
/* INT10: Encoder 3, Ch B  */
/* INT11: Encoder 4, Ch A  */
/* INT12: Encoder 4, Ch B  */
/* INT13: Analog Trigger 1 */
/* INT14: Analog Trigger 2 */
/* INT15: Analog Trigger 3 */
/* INT16: Analog Trigger 4 */
#define GPIO_NUMBER_TO_INT_BIT(NUM) 			(1<<(NUM))
#define ANALOG_TRIGGER_NUMBER_TO_INT_BIT(NUM) 	(1<<(NUM+12))

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
} IOCX_TIMER_MODE;

typedef enum _IOCX_TIMER_DIRECTION {
	UP				= 0,
	DOWN		    = 1,
} IOCX_TIMER_DIRECTION;

typedef enum _IOCX_TIMER_COUNTER_RESET {
	NORMAL			= 0,
	RESET_REQUEST	= 1,
} IOCX_TIMER_COUNTER_RESET;

#define IOCX_NUM_ANALOG_INPUTS 4 /* Does not include "internal" ADC inputs */

struct __attribute__ ((__packed__)) IOCX_REGS {
	/****************/
	/* Capabilities */
	/****************/
	uint16_t capability_flags;
	/*****************/
	/* Configuration */
	/*****************/
	uint8_t timer_cfg[IOCX_NUM_TIMERS]; /* IOCX_TIMER_MODE */
	uint8_t timer_ctl[IOCX_NUM_TIMERS]; /* IOCX_TIMER_COUNTER_RESET */
	uint16_t timer_prescaler[IOCX_NUM_TIMERS]; /* Timer Frequency (48Mhz divider) */
	uint16_t timer_aar[IOCX_NUM_TIMERS]; /* PWM:  Frame Period; QE:  auto-set to 0xFFFF */
	/* PWM Mode:  Duty-cycle Period; QE: Mode:  read-only, internal use */
	uint16_t timer_chx_ccr[IOCX_NUM_TIMERS * IOCX_NUM_CHANNELS_PER_TIMER];
	uint16_t int_cfg;			/* Mask for:  GPIO/Analog Trigger Interrupts  */
	uint8_t  gpio_cfg[IOCX_NUM_GPIOS];	 /* IOCX_GPIO_TYPE, _INPUT, _INTERRUPT */
	// uint8_t  analog_trigger_cfg[IOCX_NUM_ANALOG_TRIGGERS];
	// uint16_t analog_trigger_low_threshold[IOCX_NUM_ANALOG_TRIGGERS];
	// uint16_t analog_trigger_high_threshold[IOCX_NUM_ANALOG_TRIGGERS];
	/*****************/
	/* Data/Status   */
	/*****************/
	uint16_t gpio_intstat;  			 /* Bitmask:  1 = int present.  Write 1 to clear. */
	uint8_t  gpio_data[IOCX_NUM_GPIOS];  /* IOCX_GPIO_SET = High, IOCX_GPIO_RESET = Low.  */
	uint16_t ext_pwr_voltage;							/* Signed Thousandths */
	uint16_t analog_in_voltage[IOCX_NUM_ANALOG_INPUTS]; /* Signed Thousandths */
	// uint8_t analog_trigger_status[IOCX_NUM_ANALOG_TRIGGERS];
	uint8_t timer_status[IOCX_NUM_TIMERS]; /* IOCX_TIMER_DIRECTION */
	uint16_t timer_counter[IOCX_NUM_TIMERS]; /* QE Mode:  Encoder Counts */
	uint8_t end_of_bank;
};

static RegEncoding gpio_type_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 0, 0x07 };
static RegEncoding gpio_input_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 3, 0x03 };
static RegEncoding gpio_interrupt_reg = { offsetof(struct IOCX_REGS, gpio_cfg), 5, 0x03 };
static RegEncoding timer_mode_reg = { offsetof(struct IOCX_REGS, timer_cfg), 0, 0x03 };
static RegEncoding timer_counter_reset_reg = { offsetof(struct IOCX_REGS, timer_ctl), 0, 0x01 };
static RegEncoding timer_direction_reg = { offsetof(struct IOCX_REGS, timer_status), 0, 0x01 };

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
inline void iocx_encode_timer_direction(uint8_t *reg, IOCX_TIMER_DIRECTION val) {
	encode_reg(reg, (uint8_t)val, &timer_direction_reg);
}
inline IOCX_TIMER_DIRECTION iocx_decode_timer_direction(uint8_t *reg) {
	return (IOCX_TIMER_DIRECTION)decode_reg(reg, &timer_direction_reg);
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
