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

#include "stm32f4xx_hal.h"
#include "navx-mxp_hal.h"
#include <string.h>
#include <stdlib.h>
#ifdef ENABLE_IOCX
#include "gpio_navx-pi.h"
#include "gpiomap_navx-pi.h"
#include "adc_navx-pi.h"
#include "IOCXRegisters.h"
#include "IOCXExRegisters.h"
#include "MISCRegisters.h"
#include "ext_interrupts.h"
#include "DWTDelay.h"
#endif
#ifdef ENABLE_RTC
extern RTC_HandleTypeDef hrtc;
static uint8_t s_daylight_savings = 0; /* Default: no daylight savings adjust */
#endif

void read_unique_id(struct unique_id *id)
{
	id->first  = *((uint32_t *)0x1FFF7A10);
	id->second = *((uint32_t *)0x1FFF7A14);
	id->third  = *((uint32_t *)0x1FFF7A18);
}

void HAL_LED_Init()
{
	GPIO_InitTypeDef LED_InitStruct;
	LED_InitStruct.Pin = S1_LED_Pin|S2_LED_Pin;
	LED_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStruct.Pull = GPIO_NOPULL;
	LED_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(S1_LED_GPIO_Port, &LED_InitStruct);

	LED_InitStruct.Pin = CAL_LED_Pin;
	LED_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStruct.Pull = GPIO_NOPULL;
	LED_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(CAL_LED_GPIO_Port, &LED_InitStruct);
}

void HAL_I2C_Power_Init()
{
	GPIO_InitTypeDef I2C_Power_InitStruct;
	I2C_Power_InitStruct.Pin = _I2C_DEV_ON_Pin;
	I2C_Power_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	I2C_Power_InitStruct.Pull = GPIO_NOPULL;
	I2C_Power_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(_I2C_DEV_ON_GPIO_Port, &I2C_Power_InitStruct);
}

void HAL_CAL_Button_Init()
{
	GPIO_InitTypeDef BOOT0_InitStruct;
	BOOT0_InitStruct.Pin = CAL_BTN_Pin;
	BOOT0_InitStruct.Mode = GPIO_MODE_INPUT;
	BOOT0_InitStruct.Pull = GPIO_NOPULL;
	BOOT0_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(CAL_BTN_GPIO_Port, &BOOT0_InitStruct);
}

void HAL_DIP_Switches_Init()
{
#ifdef DISABLE_COMM_DIP_SWITCHES
	return;
#endif
	// PB 14, 15
	GPIO_InitTypeDef DIP_Switch_InitStruct;

	DIP_Switch_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
	DIP_Switch_InitStruct.Mode = GPIO_MODE_INPUT;
	DIP_Switch_InitStruct.Pull = GPIO_PULLUP;
	DIP_Switch_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &DIP_Switch_InitStruct);
}

/* SD Power FET is active low. */

void HAL_LED1_Toggle()
{
	HAL_GPIO_TogglePin( S1_LED_GPIO_Port, S1_LED_Pin);
}

void HAL_LED2_Toggle()
{
	HAL_GPIO_TogglePin( S2_LED_GPIO_Port, S2_LED_Pin);
}

void HAL_LED1_On(int on)
{
	HAL_GPIO_WritePin( S1_LED_GPIO_Port, S1_LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_LED2_On(int on)
{
	HAL_GPIO_WritePin( S2_LED_GPIO_Port, S2_LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


void HAL_CAL_LED_Toggle()
{
	HAL_GPIO_TogglePin( CAL_LED_GPIO_Port, CAL_LED_Pin);
}

void HAL_CAL_LED_On(int on)
{
	HAL_GPIO_WritePin( CAL_LED_GPIO_Port, CAL_LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int HAL_CAL_Button_Pressed()
{
	return (HAL_GPIO_ReadPin(CAL_BTN_GPIO_Port,CAL_BTN_Pin) == GPIO_PIN_SET);
}

int HAL_SPI_Slave_Enabled()
{
#ifdef DISABLE_EXTERNAL_SPI_INTERFACE
    return 0;
#else
#ifdef GPIO_MAP_NAVX_PI
    return 1;
#else
	return (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) != GPIO_PIN_SET);
#endif // GPIO_MAP_NAVX_PI
#endif // DISABLE_EXTERNAL_SPI_INTERFACE
}

int HAL_UART_Slave_Enabled()
{
#ifdef DISABLE_EXTERNAL_UART_INTERFACE
	return 0;
#endif
	return (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15) != GPIO_PIN_SET);
}

/* I2C Power FET is active low. */

void HAL_I2C_Power_On()
{
	HAL_I2C_Power_Off();
	HAL_GPIO_WritePin(_I2C_DEV_ON_GPIO_Port, _I2C_DEV_ON_Pin, GPIO_PIN_RESET);
	HAL_Delay(50); /* Give devices time to power up */
}

void HAL_I2C_Power_Off()
{
	HAL_GPIO_WritePin(_I2C_DEV_ON_GPIO_Port, _I2C_DEV_ON_Pin, GPIO_PIN_SET);
}


void HAL_RN4020_IO_Init(void)
{
#ifdef ENABLE_RN4020
    // BLUE_MLDP_EV:  PA0 (Input), Active High
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA &GPIO_InitStruct);

    // BLUE_WAKE_HW:  PA1 (Output, Active High, Internal Pulldown on RN4020)
    // BLUE_CTS:      PA2 (Output)
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA &GPIO_InitStruct)

    // BLUE_RTS:      PC1 (Input)

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC &GPIO_InitStruct);

    // BLUE_WAKE_SW:  PC0 (Output), Weak pulldown on RN4020
    // BLUE_CMD_MLDP: PC2 (Output), Active High
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC &GPIO_InitStruct);
#endif
}

void HAL_RN4020_Wake()
{
#ifdef ENABLE_RN4020
    // a1, c0 high
    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
#endif
}

void HAL_RN4020_Sleep()
{
#ifdef ENABLE_RN4020
    // a1, c0 low
    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
#endif
}

int HAL_RN4020_Get_MLDP_EV()
{
#ifdef ENABLE_RN4020
    return (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == GPIO_PIN_SET);
#else
    return 0;
#endif
}

int HAL_RN4020_Get_RTS()
{
#ifdef ENABLE_RN4020
    return (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1) == GPIO_PIN_SET);
#else
    return 0;
#endif
}

void HAL_RN4020_Set_CTS(int value)
{
#ifdef ENABLE_RN4020
    return (HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2, value ? GPIO_PIN_SET : GPIO_PIN_RESET );
#endif
}

void HAL_RN4020_Set_CMD_MLDP(int value)
{
#ifdef ENABLE_RN4020
    return (HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2, value ? GPIO_PIN_SET : GPIO_PIN_RESET );
#endif
}

void HAL_SPI_Comm_Ready_Assert() /* Active:  Low */
{
#ifdef ENABLE_SPI_COMM_READY_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_SPI_Comm_Ready_GPIO_Port, NAVX_2_RPI_SPI_Comm_Ready_Pin, GPIO_PIN_RESET);
#endif
}

void HAL_SPI_Comm_Ready_Deassert()
{
#ifdef ENABLE_SPI_COMM_READY_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_SPI_Comm_Ready_GPIO_Port, NAVX_2_RPI_SPI_Comm_Ready_Pin, GPIO_PIN_SET);
#endif
}

void HAL_CAN_Int_Assert()
{
#ifdef CAN_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_INT3_GPIO_Port, NAVX_2_RPI_INT3_Pin, GPIO_PIN_RESET);
#endif
}

void HAL_CAN_Int_Deassert()
{
#ifdef CAN_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_INT3_GPIO_Port, NAVX_2_RPI_INT3_Pin, GPIO_PIN_SET);
#endif
}

void HAL_Ensure_CAN_EXTI_Configuration()
{
	MX_CAN_Interrupt_Verify();
}

void HAL_AHRS_Int_Assert()
{
#ifdef AHRS_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_INT2_GPIO_Port, NAVX_2_RPI_INT2_Pin, GPIO_PIN_RESET);
#endif
}

void HAL_AHRS_Int_Deassert()
{
#ifdef AHRS_INTERRUPT
	  HAL_GPIO_WritePin(NAVX_2_RPI_INT2_GPIO_Port, NAVX_2_RPI_INT2_Pin, GPIO_PIN_SET);
#endif
}

void HAL_CAN_Status_LED_On(int on)
{
#ifdef ENABLE_CAN_TRANSCEIVER
	HAL_GPIO_WritePin( CAN_OK_LED_GPIO_Port, CAN_OK_LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void HAL_RTC_InitializeCache()
{
#ifdef ENABLE_RTC
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	switch (sTime.DayLightSaving)
	{
	case RTC_DAYLIGHTSAVING_SUB1H:
		s_daylight_savings = 2;
		break;
	case RTC_DAYLIGHTSAVING_ADD1H:
		s_daylight_savings = 1;
		break;
	default:
		s_daylight_savings = 0;
		break;
	}
#endif
}

void HAL_RTC_Get_DaylightSavings(uint8_t *daylight_savings)
{
#ifdef ENABLE_RTC
	*daylight_savings = s_daylight_savings;
#endif
}

void HAL_RTC_Set_DaylightSavings(uint8_t daylight_savings)
{
#ifdef ENABLE_RTC
	/* NOTE:  This will take effect the next time SetTime() occurs. */
	s_daylight_savings = daylight_savings;
#endif
}


void HAL_RTC_Get_Time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint32_t *subseconds)
{
#ifdef ENABLE_RTC
	RTC_TimeTypeDef sTime;
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	*hours = sTime.Hours;
	*minutes = sTime.Minutes;
	*seconds = sTime.Seconds;
	*subseconds = sTime.SubSeconds;
#endif
}

void HAL_RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
#ifdef ENABLE_RTC
	RTC_TimeTypeDef sTime;
	switch (s_daylight_savings)
	{
	case 2:
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_SUB1H;
		break;
	case 1:
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_ADD1H;
		break;
	default:
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		break;
	}
	sTime.TimeFormat = 0;
	sTime.StoreOperation = RTC_STOREOPERATION_SET;
	sTime.Hours = hours;
	sTime.Minutes = minutes;
	sTime.Seconds = seconds;
	HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);
#endif
}

void HAL_RTC_Set_Date(uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
#ifdef ENABLE_RTC
	RTC_DateTypeDef sDate;
	sDate.WeekDay = day;
	sDate.Date = date;
	sDate.Month = month;
	sDate.Year = year;
	HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN);
#endif
}


void HAL_RTC_Get_Date(uint8_t *weekday, uint8_t *date, uint8_t *month, uint8_t *year)
{
#ifdef ENABLE_RTC
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
	*weekday = sDate.WeekDay;
	*date = sDate.Date;
	*month = sDate.Month;
	*year = sDate.Year;
#endif
}

#ifdef ENABLE_IOCX

typedef enum  {
	INT_EXTI,	/* STM32 External Interrupt Line */
	INT_ALTPIN, /* This GPIO also connected to a secondary INT input pin. */
	INT_NONE
} GPIO_INTERRUPT_TYPE;

typedef struct {
	GPIO_TypeDef *p_gpio;
	uint16_t pin;
	uint8_t timer_index;
	GPIO_INTERRUPT_TYPE interrupt_type;
	uint8_t interrupt_index;
	uint8_t alt_function;
} GPIO_Channel;

#define MAX_VALID_STM32_AF_VALUE 0x0F
#define GPIO_AF_SW_RESET_COUNTER (MAX_VALID_STM32_AF_VALUE + 1)
#define INVALID_INTERRUPT_INDEX 255

/* Due to STM32 EXTI Interrupt line routing limitations, Interrupts for one */
/* of the timer input channels is mapped to a secondary, alternate EXTI line. */
static GPIO_TypeDef *p_altint_port = QE3_A_Second_GPIO_Port;
static uint16_t altint_pin = QE3_A_Second_Pin;

static GPIO_Channel gpio_channels[IOCX_NUM_GPIOS] =
{
	{QE1_A_GPIO_Port, 		QE1_A_Pin, 		0,  INT_EXTI, 	0,  GPIO_AF1_TIM1},
	{QE1_B_GPIO_Port, 		QE1_B_Pin, 		0,  INT_EXTI, 	1,  GPIO_AF1_TIM1},
	{QE2_A_GPIO_Port, 		QE2_A_Pin, 		1,  INT_EXTI, 	2,  GPIO_AF1_TIM2},
	{QE2_B_GPIO_Port, 		QE2_B_Pin, 		1,  INT_EXTI, 	3,  GPIO_AF1_TIM2},
	{QE3_A_GPIO_Port, 		QE3_A_Pin, 		2,  INT_ALTPIN, 4,  GPIO_AF2_TIM3},
	{QE3_B_GPIO_Port, 		QE3_B_Pin, 		2,  INT_EXTI, 	5,  GPIO_AF2_TIM3},
	{QE4_A_GPIO_Port, 		QE4_A_Pin, 		3,  INT_EXTI, 	6,  GPIO_AF2_TIM4},
	{QE4_B_GPIO_Port, 		QE4_B_Pin, 		3,  INT_EXTI, 	7,  GPIO_AF2_TIM4},
	{QE5_A_GPIO_Port, 		QE5_A_Pin, 		4,  INT_EXTI, 	8,  GPIO_AF2_TIM5},
	{QE5_B_GPIO_Port, 		QE5_B_Pin, 		4,  INT_EXTI, 	9,  GPIO_AF2_TIM5},
	{PWM_GPIO3_GPIO_Port, 	PWM_GPIO3_Pin, 	5,  INT_EXTI, 	10, GPIO_AF3_TIM9},
	{PWM_GPIO4_GPIO_Port, 	PWM_GPIO4_Pin, 	5,  INT_EXTI, 	11, GPIO_AF3_TIM9},
};

#define TIMER_RESET_DISABLED -1

static uint8_t IOCX_gpio_pin_to_interrupt_index_map[IOCX_NUM_INTERRUPTS];
static uint8_t IOCX_gpio_pin_to_gpio_channel_index_map[IOCX_NUM_INTERRUPTS];
static uint32_t IOCX_gpio_pin_mode_configuration[IOCX_NUM_INTERRUPTS];
static int IOCX_gpio_interrupt_timer_reset_targets[IOCX_NUM_INTERRUPTS];
static volatile uint16_t int_status = 0;  /* Todo:  Review Race Conditions */
static volatile uint16_t last_int_edge = 0;
static volatile uint16_t int_mask = 0; /* Todo:  Review Race Conditions */
int gpio_suspended = 0;

void HAL_IOCX_FlexDIO_Suspend(int suspend) {
	uint32_t temp;
	uint32_t position;
	int i;

	if (suspend == gpio_suspended) return;

	for (i = 0; i < IOCX_NUM_GPIOS; i++) {
		GPIO_TypeDef *p_gpio = gpio_channels[i].p_gpio;
		uint16_t pin = gpio_channels[i].pin;

		position = POSITION_VAL(pin);
		temp = p_gpio->MODER;
		temp &= ~(GPIO_MODER_MODER0 << (position * 2));
		if (suspend) {
			temp |= ((GPIO_MODE_INPUT) << (position * 2));
		} else {
			temp |= ((IOCX_gpio_pin_mode_configuration[POSITION_VAL(gpio_channels[i].pin)]) << (position * 2));
		}
		p_gpio->MODER = temp;
	}

	gpio_suspended = suspend;
}

static void HAL_IOCX_EXTI_ISR(uint8_t gpio_pin)
{
	if (gpio_pin < IOCX_NUM_INTERRUPTS) {
		uint8_t gpio_interrupt_index = IOCX_gpio_pin_to_interrupt_index_map[gpio_pin];
		if (gpio_interrupt_index != INVALID_INTERRUPT_INDEX) {
			uint8_t gpio_channel_index = IOCX_gpio_pin_to_gpio_channel_index_map[gpio_pin];
			uint16_t interrupt_bit = (1 << gpio_channel_index);
			if (HAL_GPIO_ReadPin(gpio_channels[gpio_channel_index].p_gpio, gpio_channels[gpio_channel_index].pin) == GPIO_PIN_SET) {
				if (IOCX_gpio_pin_mode_configuration[gpio_pin] & GPIO_MODE_IT_RISING) {
					last_int_edge |= interrupt_bit;
					HAL_IOCX_AssertInterrupt(interrupt_bit);
				}
			} else {
				if (IOCX_gpio_pin_mode_configuration[gpio_pin] & GPIO_MODE_IT_FALLING) {
					last_int_edge &= ~interrupt_bit;
					HAL_IOCX_AssertInterrupt(interrupt_bit);
				}
			}
		} else {
			gpio_interrupt_index = 0; /* Error case breakpoint location. */
		}
	}
}

void HAL_IOCX_Init()
{
	uint8_t i;
	for (i = 0; i < IOCX_NUM_INTERRUPTS; i++) {
		IOCX_gpio_pin_to_interrupt_index_map[i] = INVALID_INTERRUPT_INDEX;
		IOCX_gpio_interrupt_timer_reset_targets[i] = TIMER_RESET_DISABLED;
	}
	for (i = 0; i < IOCX_NUM_GPIOS; i++) {
		if (gpio_channels[i].interrupt_type == INT_EXTI) {
			IOCX_gpio_pin_to_interrupt_index_map[POSITION_VAL(gpio_channels[i].pin)] = gpio_channels[i].interrupt_index;
			IOCX_gpio_pin_to_gpio_channel_index_map[POSITION_VAL(gpio_channels[i].pin)] = i;
			IOCX_gpio_pin_mode_configuration[POSITION_VAL(gpio_channels[i].pin)] = GPIO_MODE_INPUT;
			attachInterrupt(gpio_channels[i].pin, &HAL_IOCX_EXTI_ISR, FALLING);
		} else if (gpio_channels[i].interrupt_type == INT_ALTPIN) {
			IOCX_gpio_pin_to_interrupt_index_map[POSITION_VAL(altint_pin)] = gpio_channels[i].interrupt_index;
			IOCX_gpio_pin_to_gpio_channel_index_map[POSITION_VAL(altint_pin)] = i;
			IOCX_gpio_pin_mode_configuration[POSITION_VAL(altint_pin)] = GPIO_MODE_INPUT;
			attachInterrupt(altint_pin, &HAL_IOCX_EXTI_ISR, FALLING);
		}
	}
	DWT_Init();	/* Initialize microsecond delay ARM DWT mechanism. */
}

void HAL_IOCX_Ext_Power_Enable(int enable)
{
	HAL_GPIO_WritePin(EXT_PWR_SWITCH_ON_GPIO_Port, EXT_PWR_SWITCH_ON_Pin,
			(enable ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

void HAL_IOCX_RPI_GPIO_Driver_Enable(int enable)
{
	  HAL_GPIO_WritePin(_RPI_GPIO_OE1_GPIO_Port, _RPI_GPIO_OE1_Pin,
			  (enable ? GPIO_PIN_RESET : GPIO_PIN_SET));
	  HAL_GPIO_WritePin(_RPI_GPIO_OE2_GPIO_Port, _RPI_GPIO_OE2_Pin,
			  (enable ? GPIO_PIN_RESET : GPIO_PIN_SET));
}

void HAL_IOCX_RPI_COMM_Driver_Enable(int enable)
{
	  HAL_GPIO_WritePin(COMM_OE2_GPIO_Port, COMM_OE2_Pin,
			  (enable ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

/* Returns 0 if pins are input, non-zero if output */
int HAL_IOCX_RPI_GPIO_Output()
{
	if (HAL_GPIO_ReadPin(RPI_GPIO_DIR_IN_GPIO_Port, RPI_GPIO_DIR_IN_Pin) == GPIO_PIN_SET) {
		return 1;
	}
	return 0;
}

/* Returns 0 if no ext power fault has occurred, non-zero indicates fault has occurred. */
int HAL_IOCX_Ext_Power_Fault()
{
	if (HAL_GPIO_ReadPin(_IO_POWER_FAULT_GPIO_Port, _IO_POWER_FAULT_Pin) == GPIO_PIN_RESET) {
		return 1;
	}
	return 0;
}

/* IOCX GPIO Interrupt Generation:
 *
 * Each of the 16 IOCX GPIO Pins (12 Digital + 4 Analog when Triggers are Used)
 * may be optionally configured to generate an interrupt.  This is managed
 * by a simple mechanism consisting of:
 *
 * - Interrupt Mask
 * - Interrupt Status
 * - Interrupt Assertion GPIO Pin (Active Low)
 *
 * Two separate interrupt "edge detection" methods are used:
 *
 * 1) IOCX Hardware Interrupts:
 *
 * An ISR handles STM32 EXTI interrupts on configured GPIO edges.
 *
 * 3) Analog Triggering:
 *
 * The VMX Analog Triggers may be used to detect edges on any of the
 * VMX Analog Inputs, and are implemented external to this file. An
 * interface is provided allowing GPIO Interrupt Assertion/Deassertion
 * in these cases.
 */

void HAL_IOCX_AssertInterruptSignal()
{
	/* Assert Interrupt Pin */
	HAL_GPIO_WritePin(NAVX_2_RPI_INT4_GPIO_Port, NAVX_2_RPI_INT4_Pin,GPIO_PIN_RESET);
}

/* Todo:  For performance, inline and move these to header file. */
void HAL_IOCX_AssertInterrupt(uint16_t int_status_bit)
{
	if (int_status_bit & int_mask) {
		uint8_t gpio_index;
		int_status |= int_status_bit;
		HAL_IOCX_AssertInterruptSignal();
		// Clear timer counter, if configured to do so
		gpio_index = POSITION_VAL(int_status_bit);
		if (gpio_index < IOCX_NUM_INTERRUPTS) {
			uint8_t timer_index = IOCX_gpio_interrupt_timer_reset_targets[gpio_index];
			if (timer_index != TIMER_RESET_DISABLED) {
				uint8_t reset_control;
				iocx_encode_timer_counter_reset(&reset_control, RESET_REQUEST);
				HAL_IOCX_TIMER_Set_Control(timer_index, &reset_control);
			}
		}
	}
}

void HAL_IOCX_DeassertInterruptSignal()
{
	HAL_GPIO_WritePin(NAVX_2_RPI_INT4_GPIO_Port, NAVX_2_RPI_INT4_Pin,GPIO_PIN_SET);
}

void HAL_IOCX_DeassertInterrupt(uint16_t int_bits_to_clear) {
	int_status &= ~int_bits_to_clear;
	if (!(int_status & int_mask)) {
		HAL_IOCX_DeassertInterruptSignal();
	}
}

void HAL_IOCX_UpdateInterruptMask(uint16_t int_bits) {
	int_mask = int_bits;
	/* If all actively assert interrupts are now masked    */
	/* then deassert the interrupt signal, and vice versa. */
	if(!(int_status & int_mask)) {
		HAL_IOCX_DeassertInterruptSignal();
	} else {
		HAL_IOCX_AssertInterruptSignal();
	}
	/* TOdo:  use mask in GPIO EXTI ISRs to avoid unnecessary dispatch */
}

uint16_t HAL_IOCX_GetInterruptMask() {
	return int_mask;
}

uint16_t HAL_IOCX_GetInterruptStatus() {
	return int_status;
}

uint16_t HAL_IOCX_GetLastInterruptEdges() {
	return last_int_edge;
}

/***************************************************************************/
/* Timer section                                                           */
/*                                                                         */
/* Each timer is clocked off of either APB1 or APB2, as follows:           */
/*                                                                         */
/* APB1 Timer Clocks:  48Mhz (TIM1, TIM9, TIM10, TIM11)					   */
/* APB2 Timer Clocks:  96Mhz (TIM2, TIM3, TIM4, TIM5)					   */
/*																		   */
/* To keep things uniform, all 96Mhz clock sources use an internal         */
/* (/2) divider - thus all clocks operate at a clock frequency (fTIM) of   */
/* 48Mhz.                                                                  */
/***************************************************************************/

TIM_HandleTypeDef htim1 = {TIM1};
TIM_HandleTypeDef htim2 = {TIM2};
TIM_HandleTypeDef htim3 = {TIM3};
TIM_HandleTypeDef htim4 = {TIM4};
TIM_HandleTypeDef htim5 = {TIM5};
TIM_HandleTypeDef htim9 = {TIM9};

#define NUM_IOCX_QUAD_ENCODERS 5

int qe_enabled[NUM_IOCX_QUAD_ENCODERS] = { 0, 0, 0, 0, 0 };

int qe_curr_mode[NUM_IOCX_QUAD_ENCODERS] = {
		QUAD_ENCODER_MODE_4x,
		QUAD_ENCODER_MODE_4x,
		QUAD_ENCODER_MODE_4x,
		QUAD_ENCODER_MODE_4x,
		QUAD_ENCODER_MODE_4x,
};
uint16_t qe0_last_count = 0;
uint32_t qe1_last_count = 0;
uint16_t qe2_last_count = 0;
uint16_t qe3_last_count = 0;
uint32_t qe4_last_count = 0;

volatile int32_t qe_total_count[NUM_IOCX_QUAD_ENCODERS] = { 0, 0, 0, 0, 0 };

#define SYSTICK_HANDLER_PROCESSING_FREQUENCY 2 /* Every other tick */
#define NUM_QE_COUNT_DELTAS 25
#define QE_COUNT_DELTA_PERIOD_MS (NUM_QE_COUNT_DELTAS * SYSTICK_HANDLER_PROCESSING_FREQUENCY)

volatile uint16_t qe_count_index = 0; // Range 0 - (NUM_QE_COUNT_DELTAS-1)
volatile int16_t qe_count_deltas[NUM_IOCX_QUAD_ENCODERS][NUM_QE_COUNT_DELTAS];
volatile uint32_t qe_last_delta_timestamp[NUM_IOCX_QUAD_ENCODERS];

typedef enum  {
	LEVEL_RESET_NONE,
	LEVEL_RESET_HIGH,
	LEVEL_RESET_LOW
} IOCX_COUNTER_LEVEL_RESET_TYPE;

#define INVALID_GPIO_AND_ANALOGTRIGGER_INDEX 255

typedef struct {
	IOCX_COUNTER_LEVEL_RESET_TYPE level_reset_type;
	uint8_t gpio_index;
} TIMER_LevelResetConfig;

static TIMER_LevelResetConfig timer_level_set_configs[IOCX_NUM_TIMERS] =
{
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
	{ LEVEL_RESET_NONE, INVALID_GPIO_AND_ANALOGTRIGGER_INDEX },
};

static uint32_t GetTotalQECountDelta(uint8_t qe_index) {
	if (qe_index >= NUM_IOCX_QUAD_ENCODERS) return 0;

    uint32_t total_qe_count_delta = 0;
	NVIC_DisableIRQ(SysTick_IRQn);
	volatile int16_t *p_count_deltas = &qe_count_deltas[qe_index][qe_count_index];
    int i;
	for (i = qe_count_index; i < NUM_QE_COUNT_DELTAS; i++) {
    	total_qe_count_delta += abs(*p_count_deltas);
    	p_count_deltas++;
    }
    p_count_deltas = &qe_count_deltas[qe_index][0];
    for (i = 0; i < qe_count_index; i++) {
    	total_qe_count_delta += abs(*p_count_deltas);
    	p_count_deltas++;
    }
    NVIC_EnableIRQ(SysTick_IRQn);
    return total_qe_count_delta;
}

static uint16_t GetRecentQEPulseLengthMicrosecondsAverage(uint8_t qe_index) {
	if (qe_index >= NUM_IOCX_QUAD_ENCODERS) return 0;

	uint32_t qe_count = GetTotalQECountDelta(qe_index);
	uint32_t qe_count_per_sec = qe_count * (1000 / QE_COUNT_DELTA_PERIOD_MS);
	uint32_t qe_pulse_per_sec;
	switch (qe_curr_mode[qe_index]) {
	default:
	case QUAD_ENCODER_MODE_4x:
		qe_pulse_per_sec = qe_count_per_sec / 4;
		break;
	case QUAD_ENCODER_MODE_2x:
		qe_pulse_per_sec = qe_count_per_sec / 2;
		break;
	case QUAD_ENCODER_MODE_1x:
		qe_pulse_per_sec = qe_count_per_sec;
		break;
	}
	uint32_t qe_pulse_avg_microseconds =
			(uint32_t)1000000 / qe_pulse_per_sec;
	return (uint16_t)qe_pulse_avg_microseconds;
}

static int QEIntegratorEnabled(uint8_t qe_index) {
	if (qe_index < NUM_IOCX_QUAD_ENCODERS) {
		return qe_enabled[qe_index];
	} else {
		return 0;
	}
}

static void ResetQEIntegrator(uint8_t qe_index, int enabled) {
	if (qe_index < NUM_IOCX_QUAD_ENCODERS) {
        NVIC_DisableIRQ(SysTick_IRQn);
		qe_enabled[qe_index] = enabled;
		qe_total_count[qe_index] = 0;
		switch (qe_index) {
		case 0:  qe0_last_count = 0; break;
		case 1:  qe1_last_count = 0; break;
		case 2:  qe2_last_count = 0; break;
		case 3:  qe3_last_count = 0; break;
		case 4:  qe4_last_count = 0; break;
		default: break;
		}
        NVIC_EnableIRQ(SysTick_IRQn);
	}
}

// Systick interrupt handler (invoked every millisecond).
// To limit system processing overhead, this handler only performs
// processing at a tick frequency of SYSTICK_HANDLER_PROCESSING_FREQUENCY
void HAL_IOCX_SysTick_Handler()
{
	uint32_t ts_now = HAL_GetTick();
	if ((ts_now % SYSTICK_HANDLER_PROCESSING_FREQUENCY) != 0) return;

	/* NOTE:  Timers 1, 3, 4 are 16-bit; Timers 2, 5 are 32-bit. */
	if (qe_enabled[0]) {
		uint16_t qe0_curr_count = (uint16_t)TIM1->CNT;
		int16_t count_delta = (int16_t)(qe0_curr_count - qe0_last_count);
		qe_total_count[0] += count_delta;
		qe0_last_count = qe0_curr_count;
		qe_count_deltas[0][qe_count_index] = count_delta;
		if (count_delta != 0) {
			qe_last_delta_timestamp[0] = ts_now;
		}
	}
	if (qe_enabled[1]) {
		uint32_t qe1_curr_count = (uint32_t)TIM2->CNT;
		int16_t count_delta = (int16_t)(qe1_curr_count - qe1_last_count);
		qe_total_count[1] += count_delta;
		qe1_last_count = qe1_curr_count;
		qe_count_deltas[1][qe_count_index] = count_delta;
		if (count_delta != 0) {
			qe_last_delta_timestamp[1] = ts_now;
		}
	}
	if (qe_enabled[2]) {
		uint16_t qe2_curr_count = (uint16_t)TIM3->CNT;
		int16_t count_delta = (int16_t)(qe2_curr_count - qe2_last_count);
		qe_total_count[2] += count_delta;
		qe2_last_count = qe2_curr_count;
		qe_count_deltas[2][qe_count_index] = count_delta;
		if (count_delta != 0) {
			qe_last_delta_timestamp[2] = ts_now;
		}
	}
	if (qe_enabled[3]) {
		uint16_t qe3_curr_count = (uint16_t)TIM4->CNT;
		int16_t count_delta = (int16_t)(qe3_curr_count - qe3_last_count);
		qe_total_count[3] += count_delta;
		qe3_last_count = qe3_curr_count;
		qe_count_deltas[3][qe_count_index] = count_delta;
		if (count_delta != 0) {
			qe_last_delta_timestamp[3] = ts_now;
		}
	}
	if (qe_enabled[4]) {
		uint32_t qe4_curr_count = (uint32_t)TIM5->CNT;
		int16_t count_delta = (int16_t)(qe4_curr_count - qe4_last_count);
		qe_total_count[4] += count_delta;
		qe4_last_count = qe4_curr_count;
		qe_count_deltas[4][qe_count_index] = count_delta;
		if (count_delta != 0) {
			qe_last_delta_timestamp[4] = ts_now;
		}
	}
	if (qe_count_index >= NUM_QE_COUNT_DELTAS) {
		qe_count_index = 0;
	} else {
		qe_count_index++;
	}
}

typedef struct {
	TIM_HandleTypeDef *p_tim_handle;
	uint32_t core_clock_divider;
	uint32_t first_channel_number;
	uint32_t second_channel_number;
	uint32_t max_auto_reload_value;
	int cc_irqn;
} Timer_Config;

static Timer_Config timer_configs[IOCX_NUM_TIMERS] =
{
	{&htim1, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFF, TIM1_CC_IRQn},
	{&htim2, TIM_CLOCKDIVISION_DIV2, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFFFFFF, TIM2_IRQn},
	{&htim3, TIM_CLOCKDIVISION_DIV2, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFF, TIM3_IRQn},
	{&htim4, TIM_CLOCKDIVISION_DIV2, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFF, TIM4_IRQn},
	{&htim5, TIM_CLOCKDIVISION_DIV2, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFFFFFF, TIM5_IRQn},
	{&htim9, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2, 0xFFFF, TIM1_BRK_TIM9_IRQn},
};

uint8_t curr_timer_cfg[IOCX_NUM_TIMERS]; /* All defaults have 0 value (disabled) */

uint16_t timer_channel_ccr[IOCX_NUM_TIMERS][IOCX_NUM_CHANNELS_PER_TIMER];

uint32_t timer_last_capture_interrupt_timestamp_ms[IOCX_NUM_TIMERS];

typedef struct {
	TIMER_COUNTER_CLK_SOURCE clk_source;
	TIMER_COUNTER_DIRECTION direction;
	TIMER_SLAVE_MODE slavemode;
	TIMER_SLAVE_MODE_TRIGGER_SOURCE slavemode_trigger;
	uint32_t stall_timeout_ms;
	TIMER_INPUT_CAPTURE_STALL_ACTION stall_action;
} Timer_InputCap_Config;

static Timer_InputCap_Config timer_inputcap_configs[IOCX_NUM_TIMERS] =
{
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
	{ TIMER_COUNTER_CLK_INTERNAL, TIMER_COUNTER_DIRECTION_UP, TIMER_SLAVE_MODE_DISABLED,
			TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1, 0, TIMER_STALL_ACTION_NONE },
};

typedef struct {
	TIMER_INPUT_CAPTURE_CH_SOURCE source;
	TIMER_INPUT_CAPTURE_CH_ACTIVE_EDGE active_edge;
	TIMER_INPUT_CAPTURE_CH_PRESCALER prescaler;
	TIMER_INPUT_CAPTURE_CH_FILTER filter;
} Timer_InputCap_Channel_Config;

static Timer_InputCap_Channel_Config timer_input_ch_configs[IOCX_NUM_TIMERS][IOCX_NUM_CHANNELS_PER_TIMER] =
{
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
	{{INPUT_CAPTURE_FROM_SIGNAL_A, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x },
	 {INPUT_CAPTURE_FROM_SIGNAL_B, INPUT_CAPTURE_CH_ACTIVE_RISING, INPUT_CAPTURE_PRESCALE_1x, INPUT_CAPTURE_CH_FILTER_16x }},
};

void HAL_IOCX_GPIO_Set_Config(uint8_t gpio_index, uint8_t config) {

	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;

	/* Before configuring, deinit; this clears the GPIO to default state */
	HAL_GPIO_DeInit(gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin);
	IOCX_gpio_pin_mode_configuration[POSITION_VAL(gpio_channels[gpio_index].pin)] = 0;
	if (gpio_channels[gpio_index].interrupt_type == INT_ALTPIN ) {
		/* Deinit the altpin gpio */
		HAL_GPIO_DeInit(p_altint_port, altint_pin);
		IOCX_gpio_pin_mode_configuration[POSITION_VAL(altint_pin)] = GPIO_MODE_INPUT;
	}

	IOCX_GPIO_TYPE type = iocx_decode_gpio_type(&config);
	IOCX_GPIO_INPUT input = iocx_decode_gpio_input(&config);
	IOCX_GPIO_INTERRUPT interrupt_mode = iocx_decode_gpio_interrupt(&config);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = gpio_channels[gpio_index].pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	GPIO_InitTypeDef GPIO_InitStructAltpin;
	int altpin_int_enabled = 0;

	switch(type){
	case GPIO_TYPE_INPUT:
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		/* Enable the Pin as Input */
		switch(input){
		default:
		case GPIO_INPUT_FLOAT:
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			break;
		case GPIO_INPUT_PULLUP:
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			break;
		case GPIO_INPUT_PULLDOWN:
			GPIO_InitStruct.Pull = GPIO_PULLDOWN;
			break;
		}
		if((gpio_channels[gpio_index].interrupt_type == INT_EXTI) ||
		   (gpio_channels[gpio_index].interrupt_type == INT_ALTPIN)) {
			switch(interrupt_mode) {
			case GPIO_INTERRUPT_RISING_EDGE:
				GPIO_InitStruct.Mode |= GPIO_MODE_IT_RISING;
				break;
			case GPIO_INTERRUPT_FALLING_EDGE:
				GPIO_InitStruct.Mode |= GPIO_MODE_IT_FALLING;
				break;
			case GPIO_INTERRUPT_BOTH_EDGES:
				GPIO_InitStruct.Mode |= GPIO_MODE_IT_RISING_FALLING;
				break;
			case GPIO_INTERRUPT_DISABLED:
				break;
			}
		}
		if (gpio_channels[gpio_index].interrupt_type == INT_ALTPIN) {
			GPIO_InitStructAltpin.Speed = GPIO_InitStruct.Speed;
			GPIO_InitStructAltpin.Pin = altint_pin;
			GPIO_InitStructAltpin.Mode = GPIO_InitStruct.Mode;
			GPIO_InitStructAltpin.Pull = GPIO_InitStruct.Pull;
			altpin_int_enabled = 1;
		}
		break;
	case GPIO_TYPE_OUTPUT_PUSHPULL:
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		break;
	case GPIO_TYPE_OUTPUT_OPENDRAIN:
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		break;
	case GPIO_TYPE_AF:
		if ( gpio_channels[gpio_index].alt_function <= MAX_VALID_STM32_AF_VALUE ) {
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Alternate = gpio_channels[gpio_index].alt_function;
		} else {
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			GPIO_InitStruct.Pull = GPIO_PULLDOWN; // ???
			// TODO:  Enable Interrupt if alt_function == GPIO_AF_SW_RESET_COUNTER
			// TODO:  Enable Software function
		}
		/* TODO:  If the timer is running, stop it? */
		/* TODO:  If the timer was stopped above, restart it? */
		break;
	case GPIO_TYPE_DISABLED:
	default:
		// TODO:  If a software alternate function, disable that function
		return;
	}
	if (!altpin_int_enabled) {
		IOCX_gpio_pin_mode_configuration[POSITION_VAL(gpio_channels[gpio_index].pin)] = GPIO_InitStruct.Mode;
		if (gpio_suspended) {
			/* Force Mode (while suspended) to INPUT */
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		}
		HAL_GPIO_Init(gpio_channels[gpio_index].p_gpio, &GPIO_InitStruct);
	} else {
		IOCX_gpio_pin_mode_configuration[POSITION_VAL(altint_pin)] = GPIO_InitStructAltpin.Mode;
		if (gpio_suspended) {
			/* Force Mode (while suspended) to INPUT */
			GPIO_InitStructAltpin.Mode = GPIO_MODE_INPUT;
		}
		HAL_GPIO_Init(p_altint_port, &GPIO_InitStructAltpin);
	}
}

void HAL_IOCX_GPIO_Get(uint8_t first_gpio_index, int count, uint8_t *values)
{
	int i;
	if ( first_gpio_index > (IOCX_NUM_GPIOS-1) ) return;
	if ( first_gpio_index + count > IOCX_NUM_GPIOS) {
		count = IOCX_NUM_GPIOS - first_gpio_index;
	}
	for ( i = first_gpio_index; i < first_gpio_index + count; i++ ) {
		*values++ = (HAL_GPIO_ReadPin(gpio_channels[i].p_gpio, gpio_channels[i].pin) == GPIO_PIN_SET) ?
			IOCX_GPIO_SET : IOCX_GPIO_RESET;
	}
}

/* Requested GPIO state is lowest bit (1:  HIGH; 0:  LOW).                    */
/* If upper 7 bits are not zero, a pulse is generated, and these bits define  */
/* the pulse period (in microseconds).                                        */
void HAL_IOCX_GPIO_Set(uint8_t gpio_index, uint8_t value)
{
	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;

	int requested_level = value & 0x01;
	uint8_t pulse_length_microseconds = value >> 1;

	if (pulse_length_microseconds > 0) {
		HAL_IOCX_GPIO_Pulse(gpio_index, requested_level, pulse_length_microseconds);
	} else {
		HAL_GPIO_WritePin( gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin,
    		(requested_level != IOCX_GPIO_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

/* Generates a GPIO pulse, as follows: */
/* - If current state is same as pulse level, force GPIO to opposite state, wait 1 us.    */
/* - Then, set the GPIO to the requested state.                                           */
/* - Then, delay (blocking) the requested number of microseconds.                         */
/* - Finally, set the GPIO to the opposite of the requested state.                        */
void HAL_IOCX_GPIO_Pulse(uint8_t gpio_index, int high, uint8_t pulse_length_microseconds)
{
	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;
	GPIO_PinState new_gpio_state = (high ? GPIO_PIN_SET : GPIO_PIN_RESET);

	/* Force a leading edge transition, for a minimum of 1 microsecond */
	GPIO_PinState curr_gpio_state =
			HAL_GPIO_ReadPin(gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin);
	if (curr_gpio_state == new_gpio_state) {
		HAL_GPIO_WritePin( gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin,
				(new_gpio_state == IOCX_GPIO_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		DWT_Delay(1);
	}

	/* Begin pulse */
	HAL_GPIO_WritePin( gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin,
			(new_gpio_state == IOCX_GPIO_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	/* Delay */
	if (pulse_length_microseconds > 0) {
		DWT_Delay(pulse_length_microseconds);
	}

	/* End Pulse */
	HAL_GPIO_WritePin( gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin,
		(new_gpio_state == IOCX_GPIO_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_IOCX_TIMER_Enable_Clocks(uint8_t timer_index, int enable)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;

	switch(timer_index){
	case 0: enable ? __TIM1_CLK_ENABLE() : __TIM1_CLK_DISABLE(); break;
	case 1: enable ? __TIM2_CLK_ENABLE() : __TIM2_CLK_DISABLE(); break;
	case 2: enable ? __TIM3_CLK_ENABLE() : __TIM3_CLK_DISABLE(); break;
	case 3: enable ? __TIM4_CLK_ENABLE() : __TIM4_CLK_DISABLE(); break;
	case 4: enable ? __TIM5_CLK_ENABLE() : __TIM5_CLK_DISABLE(); break;
	case 5: enable ? __TIM9_CLK_ENABLE() : __TIM9_CLK_DISABLE(); break;
	}
}

#define BOTH_ENCODER_TIMER_CHANNELS 0xFF /*(TIM_CHANNEL_1 and TIM_CHANNEL_2)*/

/* Timer Capture Complete Interrupt Service Routine Callback */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *p_htim) {
	int i;
	for ( i = 0; i < IOCX_NUM_TIMERS; i++ ) {
		if (p_htim == timer_configs[i].p_tim_handle) {
			timer_last_capture_interrupt_timestamp_ms[i] = HAL_GetTick();
			break;
		}
	}
}

void HAL_IOCX_TIMER_ConfigureInterruptPriorities(uint8_t timer_index) {
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	int cc_irqn = timer_configs[timer_index].cc_irqn;
    HAL_NVIC_SetPriority(cc_irqn, 7, 0);
}

void HAL_IOCX_TIMER_Enable_Capture_Interrupt(uint8_t timer_index, int enable) {
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	int cc_irqn = timer_configs[timer_index].cc_irqn;
	if (enable) {
		HAL_NVIC_EnableIRQ(cc_irqn);
	} else {
		HAL_NVIC_DisableIRQ(cc_irqn);
	}
}

void HAL_IOCX_TIMER_Set_Counter_Cfg(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	timer_inputcap_configs[timer_index].clk_source = iocx_ex_timer_decode_counter_clk_src(&config);
	timer_inputcap_configs[timer_index].direction = iocx_ex_timer_decode_counter_direction(&config);
}

void HAL_IOCX_TIMER_Set_SlaveMode_Cfg(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	timer_inputcap_configs[timer_index].slavemode = iocx_ex_timer_decode_slavemode(&config);
	timer_inputcap_configs[timer_index].slavemode_trigger = iocx_ex_timer_decode_slavemode_trigger_source(&config);
}

void HAL_IOCX_TIMER_INPUTCAP_Set_Cfg(uint8_t timer_index, uint8_t channel_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( channel_index > (IOCX_NUM_CHANNELS_PER_TIMER-1) ) return;
	timer_input_ch_configs[timer_index][channel_index].source = iocx_ex_timer_decode_ic_ch_src(&config);
	timer_input_ch_configs[timer_index][channel_index].active_edge = iocx_ex_timer_decode_ic_ch_active_edge(&config);
	timer_input_ch_configs[timer_index][channel_index].prescaler = iocx_ex_timer_decode_ic_ch_prescaler(&config);
}

void HAL_IOCX_TIMER_INPUTCAP_Set_Cfg2(uint8_t timer_index, uint8_t channel_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( channel_index > (IOCX_NUM_CHANNELS_PER_TIMER-1) ) return;
	timer_input_ch_configs[timer_index][channel_index].filter = iocx_ex_timer_decode_ic_ch_filter(&config);
}

void HAL_IOCX_TIMER_INPUTCAP_Set_StallCfg(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	timer_inputcap_configs[timer_index].stall_timeout_ms = iocx_ex_timer_decode_ic_stall_timeout(&config) * TIMER_INPUT_CH_SHALL_TIMEOUT_UNIT_MS;
	timer_inputcap_configs[timer_index].stall_action = iocx_ex_timer_decode_ic_stall_action(&config);
}

static void HAL_IOCX_TIMER_ConfigureInputCaptureChannel(uint8_t timer_index, uint8_t channel_index, TIM_IC_InitTypeDef *psConfigIC)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( channel_index > (IOCX_NUM_CHANNELS_PER_TIMER-1) ) return;

	/* Configure active edge(s) */
	switch (timer_input_ch_configs[timer_index][channel_index].active_edge) {
	case INPUT_CAPTURE_CH_ACTIVE_BOTH:
		psConfigIC->ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
		break;
	case INPUT_CAPTURE_CH_ACTIVE_FALLING:
		psConfigIC->ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
		break;
	case INPUT_CAPTURE_CH_ACTIVE_RISING:
	default:
		psConfigIC->ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
		break;
	}

	/* Configure Capture Source */
	TIMER_INPUT_CAPTURE_CH_SOURCE input_src = timer_input_ch_configs[timer_index][channel_index].source;
	if (((channel_index == 0) && (input_src == INPUT_CAPTURE_FROM_SIGNAL_B))||
		((channel_index == 1) && (input_src == INPUT_CAPTURE_FROM_SIGNAL_A))) {
		psConfigIC->ICSelection = TIM_ICSELECTION_INDIRECTTI;
	} else {
		psConfigIC->ICSelection = TIM_ICSELECTION_DIRECTTI;
	}

	/* Configure Input Prescaler */
	switch (timer_input_ch_configs[timer_index][channel_index].prescaler) {
	case INPUT_CAPTURE_PRESCALE_8x:
		psConfigIC->ICPrescaler = TIM_ICPSC_DIV8;
		break;
	case INPUT_CAPTURE_PRESCALE_4x:
		psConfigIC->ICPrescaler = TIM_ICPSC_DIV4;
		break;
	case INPUT_CAPTURE_PRESCALE_2x:
		psConfigIC->ICPrescaler = TIM_ICPSC_DIV2;
		break;
	case INPUT_CAPTURE_PRESCALE_1x:
	default:
		psConfigIC->ICPrescaler = TIM_ICPSC_DIV1;
		break;
	}
	/* Configure Input Filter */
	psConfigIC->ICFilter = (uint32_t)timer_input_ch_configs[timer_index][channel_index].filter;
}

void HAL_IOCX_TIMER_INPUTCAP_Set_TimerCounterResetCfg(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	TIMER_COUNTER_INTERRUPT_RESET reset = iocx_ex_timer_decode_counter_interrupt_reset_mode(&config);
	uint8_t reset_src = iocx_ex_timer_decode_counter_reset_source(&config);
	if (reset == TIMER_COUNTER_INTERRUPT_RESET_ENABLED) {
		if (reset_src < IOCX_NUM_INTERRUPTS) {
			IOCX_gpio_interrupt_timer_reset_targets[reset_src] = timer_index;
		}
	} else {
		IOCX_gpio_interrupt_timer_reset_targets[reset_src] = TIMER_RESET_DISABLED;
	}

	TIMER_COUNTER_LEVEL_RESET level_reset = iocx_ex_timer_decode_counter_level_reset_mode(&config);
	switch (level_reset) {
	case TIMER_COUNTER_LEVEL_RESET_HIGH:
		timer_level_set_configs[timer_index].level_reset_type = LEVEL_RESET_HIGH;
		timer_level_set_configs[timer_index].gpio_index = reset_src;
		break;
	case TIMER_COUNTER_LEVEL_RESET_LOW:
		timer_level_set_configs[timer_index].level_reset_type = LEVEL_RESET_LOW;
		timer_level_set_configs[timer_index].gpio_index = reset_src;
		break;
	default:
	case TIMER_COUNTER_LEVEL_RESET_NONE:
		timer_level_set_configs[timer_index].level_reset_type = LEVEL_RESET_NONE;
		timer_level_set_configs[timer_index].gpio_index = INVALID_GPIO_AND_ANALOGTRIGGER_INDEX;
		break;
	}
}

void HAL_IOCX_TIMER_Set_Config(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;

	IOCX_TIMER_MODE mode = iocx_decode_timer_mode(&config);
	IOCX_QUAD_ENCODER_MODE qe_mode = iocx_decode_quad_encoder_mode(&config);

	if (mode != TIMER_MODE_QUAD_ENCODER) {
		ResetQEIntegrator(timer_index, 0 /* Disable */ );
	}

	switch(mode){
	case TIMER_MODE_DISABLED:
		HAL_IOCX_TIMER_Enable_Capture_Interrupt(timer_index,0);
		__HAL_TIM_DISABLE(timer_configs[timer_index].p_tim_handle);
		switch (iocx_decode_timer_mode(&curr_timer_cfg[timer_index])) {
		case TIMER_MODE_QUAD_ENCODER:
			HAL_TIM_Encoder_Stop(timer_configs[timer_index].p_tim_handle, BOTH_ENCODER_TIMER_CHANNELS);
			break;
		case TIMER_MODE_PWM_OUT:
			HAL_TIM_PWM_Stop(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].first_channel_number);
			HAL_TIM_PWM_Stop(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].second_channel_number);
			break;
		case TIMER_MODE_INPUT_CAPTURE:
		{
			HAL_TIM_IC_Stop_IT(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].first_channel_number);
			HAL_TIM_IC_Stop(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].second_channel_number);
			break;
		}
		default:
			break;
		}
		break;

	case TIMER_MODE_QUAD_ENCODER:
		{

			TIM_Encoder_InitTypeDef sConfig;
			TIM_MasterConfigTypeDef sMasterConfig;

			TIM_HandleTypeDef * p_htim = timer_configs[timer_index].p_tim_handle;
			p_htim->Init.Prescaler = 0; // Prescaler should ~always~ be 0 in Quad Encoder Mode
			p_htim->Init.ClockDivision = timer_configs[timer_index].core_clock_divider;
			p_htim->Init.CounterMode = TIM_COUNTERMODE_UP;
			p_htim->Instance->ARR = timer_configs[timer_index].max_auto_reload_value;
			p_htim->Init.Period = p_htim->Instance->ARR;
			if (qe_mode == QUAD_ENCODER_MODE_4x) {
				// A & B signal, both edges
				sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
			} else /* 1x or 2x */ {
				// A signal only.  Both edges are counted,
				// and if 1x the count is divided by two (later)
				sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
			}
			qe_curr_mode[timer_index] = qe_mode;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Polarity  = TIM_ICPOLARITY_RISING;
			sConfig.IC2Polarity  = TIM_ICPOLARITY_RISING;
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Filter    = (uint32_t)timer_input_ch_configs[timer_index][0].filter;
			sConfig.IC2Filter    = (uint32_t)timer_input_ch_configs[timer_index][1].filter;
			HAL_TIM_Encoder_Init(p_htim, &sConfig);

			/* Disable MasterSlave mode */
			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_TIMEx_MasterConfigSynchronization(p_htim, &sMasterConfig);

			/* Be sure to configure the auto-reload register (ARR) to the max possible value. */
			p_htim->Instance->ARR = timer_configs[timer_index].max_auto_reload_value;

			ResetQEIntegrator(timer_index, 1 /* Enable */);

			HAL_TIM_Encoder_Start(p_htim, BOTH_ENCODER_TIMER_CHANNELS);
		}
		break;

	case TIMER_MODE_PWM_OUT:
		{
			TIM_OC_InitTypeDef sConfigOC;
			TIM_HandleTypeDef * p_htim = timer_configs[timer_index].p_tim_handle;

			// Prescaler and Period must be already set before enabling.
			p_htim->Init.CounterMode = TIM_COUNTERMODE_UP;
			p_htim->Init.ClockDivision = timer_configs[timer_index].core_clock_divider;
			HAL_TIM_PWM_Init(p_htim);

			// Periods (CCR) for both channels must be already set before enabling.
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = timer_channel_ccr[timer_index][0];
			sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
			sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
			HAL_TIM_PWM_ConfigChannel(p_htim, &sConfigOC, timer_configs[timer_index].first_channel_number);
			sConfigOC.Pulse = timer_channel_ccr[timer_index][1];
			HAL_TIM_PWM_ConfigChannel(p_htim, &sConfigOC, timer_configs[timer_index].second_channel_number);

			HAL_TIM_PWM_Start(p_htim, timer_configs[timer_index].first_channel_number);
			HAL_TIM_PWM_Start(p_htim, timer_configs[timer_index].second_channel_number);
		}
		break;

	case TIMER_MODE_INPUT_CAPTURE:
		{
			// Inputcap, Inputcap channel settings must already be already set before enabling.
			// Prescaler must be already set before enabling.
			TIM_ClockConfigTypeDef sClockSourceConfig;
			TIM_SlaveConfigTypeDef sSlaveConfig;
			TIM_IC_InitTypeDef sConfigIC;
			TIM_MasterConfigTypeDef sMasterConfig;

			TIM_HandleTypeDef * p_htim = timer_configs[timer_index].p_tim_handle;

			switch (timer_inputcap_configs[timer_index].direction) {
			case TIMER_COUNTER_DIRECTION_DN:
				p_htim->Init.CounterMode = TIM_COUNTERMODE_DOWN;
				break;
			case TIMER_COUNTER_DIRECTION_UP:
			default:
				p_htim->Init.CounterMode = TIM_COUNTERMODE_UP;
				break;
			}
			p_htim->Init.Period = timer_configs[timer_index].max_auto_reload_value;
			p_htim->Init.ClockDivision = timer_configs[timer_index].core_clock_divider;
			p_htim->Init.RepetitionCounter = 0;

			HAL_TIM_IC_Init(p_htim);

			switch (timer_inputcap_configs[timer_index].clk_source) {
			case TIMER_COUNTER_CLK_INTERNAL:
				sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
				/* Note:  If internal clock is used, the maximum capture period is fixed. */
				/* max_period_ms = 1 * (65535 /(48000/HAL_IOCX_TIMER_Get_Normalized_Prescaler(timer_index)) */
				break;
			case TIMER_COUNTER_CLK_EDGEDETECT_CH1:
				sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_TI1ED;
				break;
			case TIMER_COUNTER_CLK_FILTERED_CH2:
				sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_TI2;
				break;
			case TIMER_COUNTER_CLK_FILTERED_CH1:
			default:
				sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_TI1;
				break;
			}
			HAL_TIM_ConfigClockSource(p_htim, &sClockSourceConfig);

			switch (timer_inputcap_configs[timer_index].slavemode_trigger) {
			case TIMER_SLAVE_MODE_TRIGGER_EDGEDETECT_CH1:
				sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;
				break;
			case TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH2:
				sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
				break;
			case TIMER_SLAVE_MODE_TRIGGER_FILTERED_CH1:
			default:
				sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
				break;
			}

			switch (timer_inputcap_configs[timer_index].slavemode) {
			case TIMER_SLAVE_MODE_RESET:
				sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
				break;
			case TIMER_SLAVE_MODE_GATED:
				sSlaveConfig.SlaveMode = TIM_SLAVEMODE_GATED;
				break;
			case TIMER_SLAVE_MODE_TRIGGER:
				sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
				break;
			case TIMER_SLAVE_MODE_DISABLED:
			default:
				sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
				break;
			}
			/* ETR triggering is not supported; thus, these values are set to reasonable defaults. */
			sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
			sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
			sSlaveConfig.TriggerFilter = 0;
			HAL_TIM_SlaveConfigSynchronization(p_htim, &sSlaveConfig);

			/* Configure Timer Input Channel 1 */
			HAL_IOCX_TIMER_ConfigureInputCaptureChannel(timer_index, 0, &sConfigIC);
			HAL_TIM_IC_ConfigChannel(p_htim, &sConfigIC, TIM_CHANNEL_1);

			/* Configure Timer Input Channel 2 */
			HAL_IOCX_TIMER_ConfigureInputCaptureChannel(timer_index, 1, &sConfigIC);
			HAL_TIM_IC_ConfigChannel(p_htim, &sConfigIC, TIM_CHANNEL_2);

			/* Disable MasterSlave Mode */
			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_TIMEx_MasterConfigSynchronization(p_htim, &sMasterConfig);

			/* Start channels; first channel should generate capture interrupts */
			HAL_TIM_IC_Start_IT(p_htim, timer_configs[timer_index].first_channel_number);
			HAL_TIM_IC_Start(p_htim, timer_configs[timer_index].second_channel_number);

			HAL_IOCX_TIMER_Enable_Capture_Interrupt(timer_index,1);
		}
		break;

	default:
		return;
	}
	curr_timer_cfg[timer_index] = config;
}

void HAL_IOCX_TIMER_Set_Control(uint8_t timer_index, uint8_t* control)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	IOCX_TIMER_COUNTER_RESET reset = iocx_decode_timer_counter_reset(control);
	if (reset == RESET_REQUEST) {
		/* Generate a Timer Update event - which clears the counter (and prescaler counter) */
		HAL_TIM_GenerateEvent(timer_configs[timer_index].p_tim_handle, TIM_EventSource_Update);
		iocx_clear_timer_counter_reset(control);
		if (QEIntegratorEnabled(timer_index)) {
			ResetQEIntegrator(timer_index, 1 /* Enable */ );
		}
	}
}

void HAL_IOCX_TIMER_Set_Prescaler(uint8_t timer_index, uint16_t ticks_per_clock)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	/* If one of the 96Mhz timers, multiply ticks/clock by 2 to mimic 48Mhz timers */
	if (timer_configs[timer_index].core_clock_divider == TIM_CLOCKDIVISION_DIV1) {
		/* NOTE:  When doubling, carefully take into account that the prescaler */
		/* register is 0-based, not 1-based.                                    */
		ticks_per_clock = ((ticks_per_clock + 1)* 2) -1;
	}
	timer_configs[timer_index].p_tim_handle->Init.Prescaler = (uint32_t)ticks_per_clock;
}

void HAL_IOCX_TIMER_Get_Prescaler(uint8_t first_timer_index, int count, uint16_t *values)
{
	uint8_t i;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}

	for ( i = first_timer_index; i < first_timer_index + count; i++ ) {
		*values++ = (uint16_t)timer_configs[i].p_tim_handle->Init.Prescaler;
	}
}

/* Gets prescaler value in microseconds */
void HAL_IOCX_TIMER_Get_Normalized_Prescaler(uint8_t timer_index, uint16_t *prescaler_normalized)
{
	*prescaler_normalized = 0;
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;

	uint16_t raw_prescaler_value = (uint16_t)timer_configs[timer_index].p_tim_handle->Init.Prescaler;
	/* Normalize prescaler value to be based on 48Mhz clock */
	if (timer_configs[timer_index].core_clock_divider == TIM_CLOCKDIVISION_DIV1) {
		raw_prescaler_value /= 2;
	}
	*prescaler_normalized = raw_prescaler_value;
}

static int analog_trigger_state[MISC_NUM_ANALOG_TRIGGERS] = {
	IOCX_ANALOG_TRIGGER_STATE_LOW,
	IOCX_ANALOG_TRIGGER_STATE_LOW,
	IOCX_ANALOG_TRIGGER_STATE_LOW,
	IOCX_ANALOG_TRIGGER_STATE_LOW,
};

void HAL_IOCX_TIMER_Get_Count(uint8_t first_timer_index, int count, int32_t *values)
{
	uint8_t i;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}
	for ( i = first_timer_index; i < first_timer_index + count; i++ ) {
		int level_reset;
		uint8_t value;
		uint8_t gpio_index = timer_level_set_configs[i].gpio_index;
		uint8_t antrig_index;
		if (gpio_index >= IOCX_NUM_GPIOS) {
			antrig_index = gpio_index - IOCX_NUM_GPIOS;
			gpio_index = INVALID_GPIO_AND_ANALOGTRIGGER_INDEX;
			if (antrig_index >= MISC_NUM_ANALOG_TRIGGERS) {
				antrig_index = INVALID_GPIO_AND_ANALOGTRIGGER_INDEX;
			}
		} else {
			antrig_index = INVALID_GPIO_AND_ANALOGTRIGGER_INDEX;
		}
		switch (timer_level_set_configs[i].level_reset_type) {
		default:
		case LEVEL_RESET_NONE:
			level_reset = 0;
			break;
		case LEVEL_RESET_HIGH:
			value = 0;
			if (gpio_index != INVALID_GPIO_AND_ANALOGTRIGGER_INDEX) {
				HAL_IOCX_GPIO_Get(gpio_index, 1, &value);
				level_reset = (value == 1) ? 1 : 0;
			} else {
				if (antrig_index != INVALID_GPIO_AND_ANALOGTRIGGER_INDEX) {
					level_reset =
							(analog_trigger_state[antrig_index] ==
									IOCX_ANALOG_TRIGGER_STATE_HIGH) ? 1 : 0;
				} else {
					level_reset = 0;
				}
			}
			break;
		case LEVEL_RESET_LOW:
			value = 1;
			if (gpio_index != INVALID_GPIO_AND_ANALOGTRIGGER_INDEX) {
				HAL_IOCX_GPIO_Get(gpio_index, 1, &value);
				level_reset = (value == 0) ? 1 : 0;
			} else {
				if (antrig_index != INVALID_GPIO_AND_ANALOGTRIGGER_INDEX) {
					level_reset =
							(analog_trigger_state[antrig_index] ==
									IOCX_ANALOG_TRIGGER_STATE_LOW) ? 1 : 0;
				} else {
					level_reset = 0;
				}
			}
			break;
		}
		if (!level_reset) {
			if (QEIntegratorEnabled(i)) {
				if (qe_curr_mode[i] == QUAD_ENCODER_MODE_1x) {
					*values++ = qe_total_count[i] >> 1; /* Divide by 2 */
				} else {
					*values++ = qe_total_count[i];
				}
			} else {
				*values++ = (int32_t)timer_configs[i].p_tim_handle->Instance->CNT;
			}
		} else {
			*values++ = 0;
		}
	}
}

static int HAL_IOCX_TIMER_IsCaptureInputActive(uint8_t timer_index) {
	int input_active = 0;
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return input_active;

	uint8_t timer_config = curr_timer_cfg[timer_index];
	IOCX_TIMER_MODE timer_mode = iocx_decode_timer_mode(&timer_config);
	if ((timer_mode == TIMER_MODE_INPUT_CAPTURE) ||
		(timer_mode == TIMER_MODE_QUAD_ENCODER)) {
		uint32_t stall_timeout_ms = timer_inputcap_configs[timer_index].stall_timeout_ms;
		if (stall_timeout_ms == TIMER_INPUT_CH_STALL_TIMEOUT_DISABLED) {
			input_active = 1;
		} else {
			uint32_t curr_timestamp = HAL_GetTick();
			uint32_t last_capture_timestamp =
				(timer_mode == TIMER_MODE_INPUT_CAPTURE ?
						timer_last_capture_interrupt_timestamp_ms[timer_index] :
						qe_last_delta_timestamp[timer_index]);
			uint32_t ts_delta = curr_timestamp - last_capture_timestamp;
			input_active = (ts_delta < stall_timeout_ms);
		}
	}
	return input_active;
}

void HAL_IOCX_TIMER_Get_Status(uint8_t first_timer_index, int count, uint8_t *values)
{
	uint8_t iTimer;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}
	for ( iTimer = first_timer_index; iTimer < first_timer_index + count; iTimer++ ) {
		/* Timer Direction */
		uint32_t cr1_reg = timer_configs[iTimer].p_tim_handle->Instance->CR1;
		*values = ((cr1_reg & 0x00000010) ? 1 : 0);
		if (HAL_IOCX_TIMER_IsCaptureInputActive(iTimer)) {
			iocx_encode_timer_in_ch_activity(values, TIMER_INPUT_CH_ACTIVE);
		}
		values++;
	}
}

void HAL_IOCX_TIMER_PWM_Set_FramePeriod(uint8_t timer_index, uint16_t clocks_per_frame_period)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	timer_configs[timer_index].p_tim_handle->Init.Period = clocks_per_frame_period;
}

void HAL_IOCX_TIMER_PWM_Get_FramePeriod(uint8_t first_timer_index, int count, uint16_t* values)
{
	uint8_t i;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}
	for ( i = first_timer_index; i < first_timer_index + count; i++ ) {
		*values++ = (uint16_t)timer_configs[i].p_tim_handle->Init.Period;
	}
}

void HAL_IOCX_TIMER_PWM_Set_DutyCycle(uint8_t timer_index, uint8_t channel_index, uint16_t clocks_per_active_period)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	timer_channel_ccr[timer_index][channel_index] = clocks_per_active_period;
	if (iocx_decode_timer_mode(&curr_timer_cfg[timer_index]) != TIMER_MODE_DISABLED) {
	    if (channel_index == 0) {
	        __HAL_TIM_SetCompare(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].first_channel_number, clocks_per_active_period);
	    } else {
            __HAL_TIM_SetCompare(timer_configs[timer_index].p_tim_handle, timer_configs[timer_index].second_channel_number, clocks_per_active_period);
	    }
	}
}

void HAL_IOCX_TIMER_PWM_Get_DutyCycle(uint8_t first_timer_index, uint8_t first_channel_index, int channel_count, uint16_t *values)
{
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_channel_index > (IOCX_NUM_CHANNELS_PER_TIMER-1) ) return;
	size_t curr_struct_index = (first_timer_index * IOCX_NUM_CHANNELS_PER_TIMER) + first_channel_index;
	size_t num_entries = (sizeof(timer_channel_ccr) / sizeof(timer_channel_ccr[0][0]));
	if ( channel_count > (num_entries - curr_struct_index) ) {
		channel_count = num_entries - curr_struct_index;
	}
	int iTimer;
	int iChannel;
	int iValueIndex = 0;
	int iChannelCount = 0;
	for ( iTimer = 0; iTimer < IOCX_NUM_TIMERS; iTimer++ ) {
		uint8_t timer_cfg = curr_timer_cfg[iTimer];
		IOCX_TIMER_MODE mode = iocx_decode_timer_mode(&timer_cfg);
		int ic_input_active = HAL_IOCX_TIMER_IsCaptureInputActive(iTimer);
		TIM_HandleTypeDef * p_htim = timer_configs[iTimer].p_tim_handle;
		for ( iChannel = 0; iChannel < IOCX_NUM_CHANNELS_PER_TIMER; iChannel++ ) {
			if (((iTimer == first_timer_index) && (iChannel >= first_channel_index)) ||
			    (iTimer > first_timer_index)) {
				TIMER_INPUT_CAPTURE_STALL_ACTION stall_action =
						timer_inputcap_configs[iTimer].stall_action;
				TIMER_INPUT_CAPTURE_CH_SOURCE source =
						timer_input_ch_configs[iTimer][iChannel].source;
				int clear_ccr_reg =
						((stall_action == TIMER_STALL_ACTION_CLEAR_COUNTER) &&
						 !ic_input_active);
				if (iChannel == 0) {
					if (mode == TIMER_MODE_INPUT_CAPTURE) {
						if (!clear_ccr_reg) {
							if (source == INPUT_CAPTURE_FROM_SIGNAL_A) {
								values[iValueIndex] = (uint16_t)p_htim->Instance->CCR1;
							} else {
								values[iValueIndex] = (uint16_t)p_htim->Instance->CCR2;
							}
						} else {
							values[iValueIndex] = 0; // No recent data received
						}
					} else if (mode == TIMER_MODE_QUAD_ENCODER) {
						uint16_t qe_pulse_avg_microseconds =
								GetRecentQEPulseLengthMicrosecondsAverage(iTimer);
						values[iValueIndex] = qe_pulse_avg_microseconds;
					} else {
						values[iValueIndex] = (uint16_t)p_htim->Instance->CCR1;
					}
				} else if (iChannel == 1) {
					if (mode == TIMER_MODE_INPUT_CAPTURE) {
						if (!clear_ccr_reg) {
							if (source == INPUT_CAPTURE_FROM_SIGNAL_A) {
								values[iValueIndex] = (uint16_t)p_htim->Instance->CCR2;
							} else {
								values[iValueIndex] = (uint16_t)p_htim->Instance->CCR1;
							}
						} else {
							values[iValueIndex] = 0; // No recent data received
						}
					} else {
						values[iValueIndex] = (uint16_t)p_htim->Instance->CCR2;
					}
				}
				iValueIndex++;
				iChannelCount++;
			}
			if (iChannelCount >= channel_count) {
				break;
			}
		}
	}
}
#endif

/* PWM Mode signals are generated with a frequency determined by the value  */
/* of the TIMx_ARR register and a duty cycle determined by the value of the */
/* TIMx_CCRx register                                                       */


/***************************************************************************/
/* ADC section                                                             */
/*                                                                         */
/* A single ADC with multiple channels is used.  The ADC is clocked by     */
/* PCLK2, which is configured for 96Mhz, and an internal divide-by-8       */
/* configuration yields an ADC clock rate (fADC) of 12Mhz.                 */
/*                                                                         */
/* The STM32 is a Successive-Approximation ADC, configured for 12-bit      */
/* resolution.  Each sample requires 1 clock for each bit of resolution    */
/* +3 ADC Clocks - plus a sample period.  Longer sample periods provide    */
/* more time for the input capacitor to charge and stabilize before        */
/* sampling begins.  Tests have shown longer sample periods reduce the     */
/* impact of noise, and the currently-selected sample period is 28 clocks. */
/* This yields an overall sample rate of 150Ksps, as follows:              */
/*                                                                         */
/* ADCCLK = 96Mhz / 8 = 12 Mhz                                             */
/* NumChannels = 6                                                         */
/* SamplePeriodClks = 28                                                   */
/* ResolutionClks = 15                                                     */
/* sample_rate = ADCCLK/(NumChannels * (SamplePeriodClks + ResolutionClks) */
/* sample_rate = 46.5ksps                                                  */
/***************************************************************************/
/* Input Channel Mapping:                                                  */
/* An Connector 1:  ADC1_IN9                                               */
/* An Connector 2:  ADC1_IN8                                               */
/* An Connector 3:  ADC1_IN15                                              */
/* An Connector 4:  ADC1_IN14                                              */
/* Analog Input 5V/3.3V Jumper:  ADC1_IN10                                 */
/* Ext 12VDC Battery Input:  ADC1_IN11                                     */
/***************************************************************************/
/* ADC1 is configured in scan conversion mode, scanning each channel in    */
/* order of connector number.                                              */
/* DMA Transfer of data to SRAM is used.  The DMA transfer is configured   */
/* in circular mode.  In this way, ADC data is continually transferred     */
/* into the buffer, and software can read out data from the buffer while   */
/* transfers are in progress.                                              */
/***************************************************************************/

#ifdef ENABLE_ADC
#define ADC_CLOCK_FREQUENCY 12000000
#define ADC_CLOCKS_PER_SAMPLE 43
#define ADC_NUM_CHANNELS 6
#define ADC_CHANNEL_SAMPLE_PERIOD_US (1000000 / (ADC_CLOCK_FREQUENCY / (ADC_NUM_CHANNELS * ADC_CLOCKS_PER_SAMPLE)))
#define ADC_NUM_EXTERNAL_CHANNELS 4
#define ADC_CHANNEL_ANALOG_VOLTAGE_SWITCH 4
#define ADC_CHANNEL_EXT_POWER_VOLTAGE_DIV 5
#define INVALID_ADC_CHANNEL_NUMBER 6

/* NOTE:  The last two ADC channels are reserved for internal use; the preceding  */
/* ADC channels are user GPIOs.                                                   */
/* The next-to-last channel is the 5V vs 3.3V Analog Input Voltage measure        */
/*   (if this value is > 2.5V, then the Analog Voltage Input High Level is 5V)    */
/* The last channel measures the 12VDC input voltage (w/a voltage divider 4.45:1, */
/*   to ensure that the max voltage does not exceed 3.3V).                        */

typedef struct {
	uint16_t data[ADC_NUM_CHANNELS];
} ADC_Samples;

uint32_t adc_channels[ADC_NUM_CHANNELS] = {
		ADC_CHANNEL_9,  // Ext Analog Input 1
		ADC_CHANNEL_8,  // Ext Analog Input 2
		ADC_CHANNEL_15, // Ext Analog Input 3
		ADC_CHANNEL_14, // Ext Analog Input 4
		ADC_CHANNEL_10,	// Analog Voltage Switch
		ADC_CHANNEL_11  // Ext Power Voltage
};

#define MAX_NUM_ADC_SAMPLES (1 << (MAX_NUM_ADC_SAMPLESIZE_POWER_BITS + NUM_ADC_SAMPLE_SET_BITS)) /* MUST be power of 2 */
ADC_Samples adc_samples[MAX_NUM_ADC_SAMPLES];
int adc_enabled = 0;
#endif

static uint32_t adc_dma_completion_count_atomic = 0;
static int adc_dma_error_callback_count = 0;

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {
	adc_dma_error_callback_count++;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance != hadc1.Instance) {
		return;
	}
	if (hadc->State != HAL_ADC_STATE_EOC_REG) {
		return;
	}
	adc_dma_completion_count_atomic++;
}

#ifdef ENABLE_ADC
static const uint32_t c_num_adc_samples_per_transfer_cycle = sizeof(adc_samples) / sizeof(uint16_t);
static const uint8_t c_num_adc_samples_per_channel_per_transfer = (sizeof(adc_samples[0]) / sizeof(uint16_t));
static const uint16_t c_num_adc_samples_per_channel = sizeof(adc_samples) / sizeof(adc_samples[0]);
// The oldest quarter of all adc samples are not accessed, because they may be modified by active DMA transfer.
static const uint16_t c_num_reserved_adc_samples_per_channel = sizeof(adc_samples) / sizeof(adc_samples[0]) / 4;
#endif

// Returns the total number of samples acquired (one for each ADC channel) since startup, and also
// a count of the number of full dma transfer cycles and the number of samples currently acquired
// for the current dma transfer cycle.
uint64_t HAL_IOCX_ADC_GetLastCompleteSampleCount(uint32_t *full_dma_transfer_cycle_count, uint32_t *num_completed_per_channel_transfers) {
    uint64_t adc_total_num_samples_per_channel = adc_dma_completion_count_atomic;
    uint32_t curr_adc_dma_ndtr = hadc1.DMA_Handle->Instance->NDTR;

    // If DMA Transfer Complete interrupt is pending, increment completion count,
    // since ISR which would have incremented it has not yet fired.
    // (This is only necessary to handle the case where this function
    // is invoked from a higher-priority which delays the invocation of the ISR).
    if(__HAL_DMA_GET_FLAG(hadc1.DMA_Handle, DMA_FLAG_TCIF0_4) != RESET) {
    	adc_total_num_samples_per_channel++;
    }

    *full_dma_transfer_cycle_count = adc_total_num_samples_per_channel;

    adc_total_num_samples_per_channel *= c_num_adc_samples_per_channel;

	uint16_t num_curr_transfer_completed_samples_per_channel =
			(c_num_adc_samples_per_transfer_cycle -
				curr_adc_dma_ndtr) / ADC_NUM_CHANNELS;

	adc_total_num_samples_per_channel += num_curr_transfer_completed_samples_per_channel;
	*num_completed_per_channel_transfers = num_curr_transfer_completed_samples_per_channel;

	return adc_total_num_samples_per_channel;
}

static const uint64_t c_num_clocks_per_microsecond = (ADC_CLOCK_FREQUENCY / 1000000);

// Retrieves a timestamp derived from the ADC DMA Engine, which continuously transfers
// ADC Data to the DMA Transfer buffer.  Given the total number of transfers and the
// transfer a, a current timestamp (since the dma transfer engine started) is returned,
// with a granularity of microseconds.
// NOTE:  Since DMA FIFO mode is disabled, NTDR directly tracks to-memory transfers
uint64_t HAL_IOCX_ADC_GetCurrTimestampMicroseconds() {
    uint64_t dma_num_samples = adc_dma_completion_count_atomic;
    uint32_t curr_adc_dma_ndtr = hadc1.DMA_Handle->Instance->NDTR;

    // If DMA Transfer Complete interrupt is pending, increment completion count,
    // since ISR which would have incremented it has not yet fired.
    // (This is only necessary to handle the case where this function
    // is invoked from a higher-priority which delays the invocation of the ISR).
    if(__HAL_DMA_GET_FLAG(hadc1.DMA_Handle, DMA_FLAG_TCIF0_4) != RESET) {
    	dma_num_samples++;
    }

    dma_num_samples *= c_num_adc_samples_per_transfer_cycle;
	uint16_t num_curr_transfer_completed_samples =
			c_num_adc_samples_per_transfer_cycle -
				curr_adc_dma_ndtr;
    dma_num_samples += num_curr_transfer_completed_samples;
    uint64_t dma_num_clockticks = (dma_num_samples * ADC_CLOCKS_PER_SAMPLE);
    uint64_t dma_timestamp_us = dma_num_clockticks / c_num_clocks_per_microsecond;
    return dma_timestamp_us;
}

// Returns the position (within the dma transfer buffer) of the last sample in the most recent
// "average sample set".  Also returns the number of "sample average sets" which can be safely accessed
// that the moment, and the absolute sample position.
uint64_t HAL_IOCX_ADC_GetAverageSetFinalSamplePosition(uint64_t complete_sample_count, uint8_t oversample_bits, uint8_t average_bits, uint16_t *num_avg_sample_sets, uint64_t *absolute_sample_position)
{
	uint32_t complete_transfer_cycle_count = complete_sample_count / c_num_adc_samples_per_channel;
	uint64_t available_sample_count =
			complete_sample_count % c_num_adc_samples_per_channel;
	uint16_t num_samples_in_average = (1 << (oversample_bits + average_bits));
	// This count now represents the number of samples in the current transfer buffer
	available_sample_count &= ~(num_samples_in_average - 1);
	uint16_t final_average_sample_position;
	uint64_t abs_sample_position;
	if (available_sample_count > 0) {
		final_average_sample_position = available_sample_count - 1;
		abs_sample_position = complete_transfer_cycle_count * c_num_adc_samples_per_channel;
		abs_sample_position += final_average_sample_position;
	} else {
		final_average_sample_position = c_num_adc_samples_per_channel - 1;
		abs_sample_position = (complete_transfer_cycle_count - 1) * c_num_adc_samples_per_channel;
		abs_sample_position += final_average_sample_position;
	}
	*num_avg_sample_sets = (c_num_adc_samples_per_channel - c_num_reserved_adc_samples_per_channel) / num_samples_in_average;
	*absolute_sample_position = abs_sample_position;
	// This count now represents last sample in the most recent "average" sample set.
	return final_average_sample_position;
}

/* Note:  ADC DMA is configured for 16-bit words.  The "Length" Parameter to HAL_ADC_Start_DMA */
/* below is the number of 16-bit words transferred - NOT the number of bytes.                  */

void HAL_IOCX_ADC_Enable(int enable)
{
#ifdef ENABLE_ADC
	if (enable) {
		if (adc_enabled == 0) {
			memset(&adc_samples,0, sizeof(adc_samples));
			HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1,
					(uint32_t *)&adc_samples, c_num_adc_samples_per_transfer_cycle);
			adc_enabled = ( status == HAL_OK ) ? 1 : 0;
		}
	} else {
		if (adc_enabled) {
			HAL_ADC_Stop_DMA(&hadc1);
			adc_enabled = 0;
		}
	}
#endif
}

void HAL_IOCX_ADC_Get_OversampledAveraged_Request_Samples(int channel, uint16_t latest_pos, uint32_t *p_latest_oversample, uint8_t oversample_bits, uint32_t *p_avg, uint8_t avg_bits, uint16_t *prev_sample_average_set_pos)
{
	// Range check requested position, to ensure the max # of power bits is not exceeded
	uint16_t count_total = (1 << (MAX_NUM_ADC_SAMPLESIZE_POWER_BITS + NUM_ADC_SAMPLE_SET_BITS));
	if (latest_pos >= count_total) {
		return;
	}

	// Ensure the total number of power bits doesn't exceed the maximum
	if ((oversample_bits + avg_bits) > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
		if (oversample_bits > avg_bits) {
			if (oversample_bits > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
				oversample_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS;
				avg_bits = 0;
			} else if (avg_bits > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
				avg_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS;
				oversample_bits = 0;
			} else {
				oversample_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS / 2;
				avg_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS - oversample_bits;
			}
		}
	}

	uint16_t count_requested = (1 << (oversample_bits + avg_bits));
	uint16_t count_current = latest_pos;

	uint16_t count_requested_1;
	uint16_t data_requested_1_idx;
	uint16_t count_requested_2;
	uint16_t data_requested_2_idx;

	if (count_requested > count_current) {
		// Wrap around case; some needed (older) data is at end of buffer
		count_requested_1 = count_requested - count_current;
		data_requested_1_idx = count_total - count_requested_1;
		count_requested_2 = count_requested - count_requested_1;
		data_requested_2_idx = 0;
		if (data_requested_1_idx > 0) {
			*prev_sample_average_set_pos = data_requested_1_idx - 1;
		} else {
			*prev_sample_average_set_pos = count_total - 1;
		}
	} else {
		// Sufficient most-recent data is at the beginning of the buffer.
		count_requested_1 = 0;
		data_requested_1_idx = 0;
		count_requested_2 = count_requested;
		data_requested_2_idx = count_current - count_requested;
		if (data_requested_2_idx > 0) {
			*prev_sample_average_set_pos = data_requested_2_idx - 1;
		} else {
			*prev_sample_average_set_pos = count_total - 1;
		}
	}

	register uint16_t ovsMask = ((1 << oversample_bits) - 1);
	register uint32_t ovsData = 0;
	register uint32_t avgData = 0;
	// Process oldest portion, if any
	register uint16_t *pData;
	register uint16_t i;
	if (count_requested_1 > 0) {
		pData = (uint16_t *)&(adc_samples[data_requested_1_idx].data[channel]);
		for (i = 0; i < count_requested_1; i++) {
			// Reset Oversample if at beginning of oversample set
			if (!(i & ovsMask)) {
				ovsData = *pData;
			} else {
				ovsData += *pData;
			}
			// Add Oversample to average if at end of oversample set
			if (!((i+1) & ovsMask)) {
				avgData += ovsData;
			}
			pData += ADC_NUM_CHANNELS;
		}
	}
	// Process newest portion;
	pData = (uint16_t *)&(adc_samples[data_requested_2_idx].data[channel]);
	for (i = count_requested_1; i < count_requested_1 + count_requested_2; i++) {
		// Reset Oversample if at beginning of oversample set
		if (!(i & ovsMask)) {
			ovsData = *pData;
		} else {
			ovsData += *pData;
		}
		// Add Oversample to average if at end of oversample set
		if (!((i+1) & ovsMask)) {
			avgData += ovsData;
		}
		pData += ADC_NUM_CHANNELS;
	}

	avgData /= (1 << avg_bits);

	*p_latest_oversample = ovsData;
	*p_avg = avgData;
}


void HAL_IOCX_ADC_Get_Latest_Samples(int start_channel, int n_channels, uint32_t *p_oversamples, uint8_t oversample_bits, uint32_t *p_avgsamples, uint8_t avg_bits)
{
#ifdef ENABLE_ADC
	/* Range-check inputs */
	// Ensure the total number of power bits doesn't exceed the maximum
	if ((oversample_bits + avg_bits) > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
		if (oversample_bits > avg_bits) {
			if (oversample_bits > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
				oversample_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS;
				avg_bits = 0;
			} else if (avg_bits > MAX_NUM_ADC_SAMPLESIZE_POWER_BITS) {
				avg_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS;
				oversample_bits = 0;
			} else {
				oversample_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS / 2;
				avg_bits = MAX_NUM_ADC_SAMPLESIZE_POWER_BITS - oversample_bits;
			}
		}
	}

	if (start_channel > (ADC_NUM_CHANNELS-1)) {
		return;
	}
	if ((start_channel + n_channels) > (ADC_NUM_CHANNELS-1)) {
		n_channels = ADC_NUM_CHANNELS - start_channel;
	}

	uint8_t num_samples_to_accumulate = 1 << oversample_bits;

	/* Determine which contiguous region of the ADC DMA transfer buffer     */
	/* contains the most recent N samples, where N is defined by the number */
	/* of oversample bits.                                                  */

	/* Note each transfer is a 16-bit ADC Sample (2 bytes) */
	uint32_t remaining_adc_transfers = hadc1.DMA_Handle->Instance->NDTR;
	uint32_t last_valid_adc_transfer = c_num_adc_samples_per_transfer_cycle - remaining_adc_transfers;
	if (last_valid_adc_transfer == 0) {
		last_valid_adc_transfer = c_num_adc_samples_per_transfer_cycle - 1;
	} else{
		last_valid_adc_transfer -= 1;
	}
	uint32_t total_adc_transfer_sets = sizeof(adc_samples) / sizeof(adc_samples[0]);

	uint16_t last_valid_adc_transfer_set = last_valid_adc_transfer / c_num_adc_samples_per_channel_per_transfer;
	uint16_t first_valid_adc_transfer_set;
	if (num_samples_to_accumulate <= last_valid_adc_transfer_set) {
		first_valid_adc_transfer_set = last_valid_adc_transfer_set - num_samples_to_accumulate + 1;
	} else {
		/* Buffer wrap around case; accumulate from the end of the buffer. */
		last_valid_adc_transfer_set = (total_adc_transfer_sets - 1);
		first_valid_adc_transfer_set = last_valid_adc_transfer_set - num_samples_to_accumulate + 1;
	}

	/* Zero Accumulator */
	int i, j;
	for ( i = 0; i < n_channels; i++ ) {
		p_oversamples[i] = 0;
	}
	/* Accumulate */
	for ( i = first_valid_adc_transfer_set; i <= last_valid_adc_transfer_set; i++ ) {
		uint16_t *p_sample_set = &(adc_samples[i].data[start_channel]);
		for ( j = 0; j < n_channels; j++ ) {
			p_oversamples[j] += *p_sample_set++;
		}
	}
	/* Average */
	for ( i = 0; i < n_channels; i++ ) {
		p_avgsamples[i] = p_oversamples[i] >> avg_bits;
		if (p_avgsamples[i] > 5000) {
			p_avgsamples[i] = 1; // debug hook
		}
	}
#endif
}

void HAL_IOCX_ADC_Get_Samples( int start_channel, int n_channels, uint16_t* p_samples, uint8_t n_samples_to_average )
{
#ifdef ENABLE_ADC
	/* Range-check inputs */
	if ( n_samples_to_average > (sizeof(adc_samples)/sizeof(adc_samples[0]))) {
		n_samples_to_average = (sizeof(adc_samples)/sizeof(adc_samples[0]));
	}
	if (start_channel > (ADC_NUM_CHANNELS-1)) {
		return;
	}
	if ((start_channel + n_channels) > (ADC_NUM_CHANNELS-1)) {
		n_channels = ADC_NUM_CHANNELS - start_channel;
	}
	if (n_samples_to_average > MAX_NUM_ADC_SAMPLES ) {
		n_samples_to_average = MAX_NUM_ADC_SAMPLES;
	}
	/* Zero Accumulator */
	uint32_t accum[ADC_NUM_CHANNELS];
	int i, j;
	for ( i = 0; i < ADC_NUM_CHANNELS; i++ ) {
		accum[i] = 0;
	}
	/* Accumulate */
	for ( i = 0; i < n_samples_to_average; i++ ) {
		uint16_t *p_samples = &(adc_samples[i].data[start_channel]);
		for ( j = 0; j < n_channels; j++ ) {
			accum[j] += *p_samples++;
		}
	}
	/* Average */
	for ( i = 0; i < n_channels; i++ ) {
		p_samples[i] = accum[i] / n_samples_to_average;
	}
#endif
}

#define ADC_VOLTAGE_OVERSAMPLE_BITS 0
#define ADC_VOLTAGE_AVERAGE_BITS 3
/* Returns 0 if 3.3V, non-zero of 5V VDA */
int HAL_IOCX_ADC_Voltage_5V( int n_samples_to_average )
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint32_t last_oversample;
		uint32_t average;
		uint32_t full_dma_transfer_cycle_count;
		uint32_t num_completed_per_channel_transfers;
		uint16_t last_sample_pos_in_prev_average_sample_set;
		HAL_IOCX_ADC_GetLastCompleteSampleCount(&full_dma_transfer_cycle_count, &num_completed_per_channel_transfers);
		HAL_IOCX_ADC_Get_OversampledAveraged_Request_Samples(ADC_CHANNEL_ANALOG_VOLTAGE_SWITCH, num_completed_per_channel_transfers, &last_oversample, ADC_VOLTAGE_OVERSAMPLE_BITS, &average, ADC_VOLTAGE_AVERAGE_BITS, &last_sample_pos_in_prev_average_sample_set);
		return (average > 3500) ? 1 : 0;
	} else {
		return 0;
	}
#else
    return 0;
#endif
}

#define ADC_EXT_POWER_OVERSAMPLE_BITS 0
#define ADC_EXT_POWER_AVERAGE_BITS 3
const float vdiv_ratio = 3.24f / (11.5f + 3.24f);

uint16_t HAL_IOCX_Get_ExtPower_Voltage()
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint32_t last_oversample;
		uint32_t average;
		uint32_t full_dma_transfer_cycle_count;
		uint32_t num_completed_per_channel_transfers;
		uint16_t last_sample_pos_in_prev_average_sample_set;
		HAL_IOCX_ADC_GetLastCompleteSampleCount(&full_dma_transfer_cycle_count, &num_completed_per_channel_transfers);
		HAL_IOCX_ADC_Get_OversampledAveraged_Request_Samples(ADC_CHANNEL_EXT_POWER_VOLTAGE_DIV, num_completed_per_channel_transfers, &last_oversample, ADC_EXT_POWER_OVERSAMPLE_BITS, &average, ADC_EXT_POWER_AVERAGE_BITS, &last_sample_pos_in_prev_average_sample_set);
		return average;
	} else {
		return 0;
	}
#else
    return 0;
#endif
}

#ifdef ENABLE_ADC
static int awdg_enabled = 0;
uint8_t awdg_channel = INVALID_ADC_CHANNEL_NUMBER;
uint16_t awdg_low_threshold = 0;
uint16_t awdg_high_threshold = 0;
int awdg_interrupt_enabled = 0;
#endif

void HAL_IOCX_ADC_AWDG_Disable()
{
#ifdef ENABLE_ADC
	ADC_AnalogWDGConfTypeDef AnalogWDGConfig;

	/**Configure the analog watchdog
	*/
	AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_NONE;
	if(HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) == HAL_OK) {
		awdg_enabled = 0;
		awdg_channel = INVALID_ADC_CHANNEL_NUMBER;
	}

#endif
}

void HAL_IOCX_ADC_AWDG_Enable(uint8_t channel)
{
#ifdef ENABLE_ADC
	if ( channel < ADC_NUM_EXTERNAL_CHANNELS ) {
		ADC_AnalogWDGConfTypeDef AnalogWDGConfig;

		/**Configure the analog watchdog
		*/
		AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
		AnalogWDGConfig.HighThreshold = awdg_high_threshold;
		AnalogWDGConfig.LowThreshold = awdg_low_threshold;
		AnalogWDGConfig.Channel = adc_channels[channel];
		AnalogWDGConfig.ITMode = awdg_interrupt_enabled ? ENABLE : DISABLE;
		if(HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) == HAL_OK) {
			awdg_enabled = !0;
			awdg_channel = channel;
		}
	}
#endif
}

/* Returns non-zero if enabled */
int HAL_IOCX_ADC_AWDG_Is_Enabled(uint8_t* p_channel)
{
#ifdef ENABLE_ADC
	return awdg_enabled;
#else
	return 0;
#endif
}

uint16_t HAL_IOCX_ADC_AWDG_Get_Threshold_High()
{
#ifdef ENABLE_ADC
	return awdg_high_threshold;
#else
	return 0;
#endif
}

uint16_t HAL_IOCX_ADC_AWDG_Get_Threshold_Low()
{
#ifdef ENABLE_ADC
	return awdg_low_threshold;
#else
	return 0;
#endif
}

void HAL_IOCX_ADC_AWDG_Set_Threshold_High(uint16_t threshold)
{
#ifdef ENABLE_ADC
	awdg_high_threshold = threshold;
	if(awdg_enabled) {
		hadc1.Instance->HTR = threshold;
	}
#endif
}

void HAL_IOCX_ADC_AWDG_Set_Threshold_Low(uint16_t threshold)
{
#ifdef ENABLE_ADC
	awdg_low_threshold = threshold;
	if(awdg_enabled) {
		hadc1.Instance->LTR = threshold;
	}
#endif
}

int HAL_IOCX_ADC_AWDG_Get_Interrupt_Enable()
{
#ifdef ENABLE_ADC
	return awdg_interrupt_enabled;
#else
	return 0;
#endif
}

void HAL_IOCX_ADC_AWDG_Set_Interrupt_Enable(int enable)
{
#ifdef ENABLE_ADC
	awdg_interrupt_enabled = enable;
	if(awdg_enabled) {
	/* Enable the ADC Analog watchdog interrupt */
		__HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_AWD);
	} else {
	/* Disable the ADC Analog watchdog interrupt */
		__HAL_ADC_DISABLE_IT(&hadc1, ADC_IT_AWD);
	}
#endif
}

void HAL_IOCX_ADC_SetCurrentAnalogTriggerState(uint8_t analog_trigger_index, uint8_t curr_analog_trigger_state)
{
#ifdef ENABLE_ADC
	if (analog_trigger_index >= MISC_NUM_ANALOG_TRIGGERS) return;
	if (curr_analog_trigger_state >= IOCX_ANALOG_TRIGGER_STATE_LAST) return;
	analog_trigger_state[analog_trigger_index] = curr_analog_trigger_state;
#endif
}
