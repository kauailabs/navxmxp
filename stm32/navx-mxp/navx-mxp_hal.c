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
#include "gpiomap_navx-pi.h"
#include "adc_navx-pi.h"
#include "IOCXRegisters.h"
#endif
#ifdef ENABLE_RTC
extern RTC_HandleTypeDef hrtc;
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
	return (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) != GPIO_PIN_SET);
#endif
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
	RTC_HandleTypeDef hrtc;
	hrtc.Instance = RTC;
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
	RTC_HandleTypeDef hrtc;
	hrtc.Instance = RTC;
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
	*weekday = sDate.WeekDay;
	*date = sDate.Date;
	*month = sDate.Month;
	*year = sDate.Year;
#endif
}

#ifdef ENABLE_IOCX

typedef enum  {
	INT_EXTI,
	INT_SWED,
	INT_NONE
} GPIO_INTERRUPT_TYPE;

typedef struct {
	GPIO_TypeDef *p_gpio;
	uint16_t pin;
	uint8_t timer_index;
	uint8_t channel_index;
	GPIO_INTERRUPT_TYPE interrupt_type;
	uint8_t interrupt_index;
	uint8_t alt_function;
} GPIO_Channel;

#define MAX_VALID_STM32_AF_VALUE 0x0F
#define GPIO_AF_SW_RESET_COUNTER (MAX_VALID_STM32_AF_VALUE + 1)

static GPIO_Channel gpio_channels[IOCX_NUM_GPIOS] =
{
	{PWM_GPIO1_GPIO_Port, 	PWM_GPIO1_Pin, 	4,  0, INT_EXTI, 0,  GPIO_AF2_TIM5},
	{PWM_GPIO2_GPIO_Port, 	PWM_GPIO2_Pin, 	4,  1, INT_EXTI, 1,  GPIO_AF2_TIM5},
	{PWM_GPIO3_GPIO_Port, 	PWM_GPIO3_Pin, 	5,  0, INT_EXTI, 2,  GPIO_AF3_TIM9},
	{PWM_GPIO4_GPIO_Port, 	PWM_GPIO4_Pin, 	5,  1, INT_EXTI, 3,  GPIO_AF3_TIM9},
	{QE1_A_GPIO_Port, 		QE1_A_Pin, 		0,  0, INT_EXTI, 4,  GPIO_AF1_TIM1},
	{QE1_B_GPIO_Port, 		QE1_B_Pin, 		0,  1, INT_SWED, 5,  GPIO_AF1_TIM1},
	{QE2_A_GPIO_Port, 		QE2_A_Pin, 		1,  0, INT_EXTI, 6,  GPIO_AF1_TIM2},
	{QE2_B_GPIO_Port, 		QE2_B_Pin, 		1,  1, INT_SWED, 7,  GPIO_AF1_TIM2},
	{QE3_A_GPIO_Port, 		QE3_A_Pin, 		2,  0, INT_SWED, 8,  GPIO_AF2_TIM3},
	{QE3_B_GPIO_Port, 		QE3_B_Pin, 		2,  1, INT_SWED, 9,  GPIO_AF2_TIM3},
	{QE4_A_GPIO_Port, 		QE4_A_Pin, 		3,  0, INT_EXTI, 10, GPIO_AF2_TIM5},
	{QE4_B_GPIO_Port, 		QE4_B_Pin, 		3,  1, INT_EXTI, 11, GPIO_AF2_TIM5},
};

#define NUM_GPIO_EXTI_INTERRUPTS 			8
#define NUM_GPIO_SWED_INTERRUPTS 			4
#define NUM_ANALOG_TRIGGER_INTERRUPTS 		4

#define EXTI_INTERRUPT_BIT_MASK				0x0C5F
#define SWED_INTERRUPT_BIT_MASK		    	0x03A0
#define ANALOG_TRIGGER_INTERRUPT_BIT_MASK	0xF000

void HAL_IOCX_Init()
{
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
	  HAL_GPIO_WritePin(_COMM_OE1_GPIO_Port, _COMM_OE1_Pin,
			  (enable ? GPIO_PIN_RESET : GPIO_PIN_SET));
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

static volatile uint32_t int_status = 0;  /* Todo:  Review Race Conditions */
static volatile uint32_t int_mask = 0; /* Todo:  Review Race Conditions */
static volatile uint32_t *int_request_gpio_bank = &GPIOC->ODR; /* Todo: get symbol from map? */
static const uint32_t int_request_bit = GPIO_PIN_8; /* Todo:  get symbol from map? */

void HAL_IOCX_AssertInterruptSignal()
{
	/* Assert Interrupt Pin */
	uint32_t curr_gpio_outdata = *int_request_gpio_bank;
	*int_request_gpio_bank = curr_gpio_outdata | int_request_bit;
	HAL_GPIO_WritePin(NAVX_2_RPI_INT4_GPIO_Port, NAVX_2_RPI_INT4_Pin,GPIO_PIN_RESET);
}

/* Todo:  For performance, inline and move these to header file. */
void HAL_IOCX_AssertInterrupt(uint32_t new_int_status_bits)
{
	/* Mask out any current-disabled bits */
	/* If any bits remain: */
	/*   Set the specified bits in the interrupt status */
	/*   If Interrupt Pin is not Asserted */
	/*     Assert the Interrupt Pin */
	int_status |= new_int_status_bits;
	if (int_status & int_mask) {
		HAL_IOCX_AssertInterruptSignal();
	}
}

void HAL_IOCX_DeassertInterruptSignal()
{
	uint32_t curr_gpio_outdata = *int_request_gpio_bank;
	*int_request_gpio_bank = curr_gpio_outdata & ~int_request_bit;
	HAL_GPIO_WritePin(NAVX_2_RPI_INT4_GPIO_Port, NAVX_2_RPI_INT4_Pin,GPIO_PIN_SET);
}

void HAL_IOCX_DeassertInterrupt(uint32_t int_bits_to_clear) {
	int_status &= ~int_bits_to_clear;
	if (!(int_status & int_mask)) {
		HAL_IOCX_DeassertInterruptSignal();
	}
}

void HAL_IOCX_UpdateInterruptMask(uint32_t int_bits) {
	int_mask = int_bits;
	/* If all actively assert interrupts are now masked    */
	/* then deassert the interrupt signal, and vice versa. */
	if(!(int_status & int_mask)) {
		HAL_IOCX_DeassertInterruptSignal();
	} else {
		HAL_IOCX_AssertInterruptSignal();
	}
	/* Todo:  Disable SoftwareEdgeDetector if none are masked in? */
	/* TOdo:  use mask in GPIO EXTI ISRs to avoid unnecessary dispatch */
}

uint32_t HAL_IOCX_GetInterruptMask() {
	return int_mask;
}

uint32_t HAL_IOCX_GetInterruptStatus() {
	return int_status;
}

/***************************************************************************/
/* Timer section                                                           */
/*                                                                         */
/* Each timer is clocked off of either APB1 or APB2, as follows:           */
/*                                                                         */
/* APB1 Timer Clocks:  24Mhz (TIM2, TIM3, TIM4, TIM5)					   */
/* APB2 Timer Clocks:  96Mhz (TIM1, TIM9, TIM10, TIM11)					   */
/*																		   */
/* To keep things uniform, all 96Mhz clock sources use an internal         */
/* (/4) divider - thus all clocks operate at a clock frequency (fTIM) of   */
/* 24Mhz.                                                                  */
/***************************************************************************/

TIM_HandleTypeDef htim1 = {TIM1};
TIM_HandleTypeDef htim2 = {TIM2};
TIM_HandleTypeDef htim3 = {TIM3};
TIM_HandleTypeDef htim4 = {TIM4};
TIM_HandleTypeDef htim5 = {TIM5};
TIM_HandleTypeDef htim9 = {TIM9};

//#ifdef ENABLE_PWM_GENERATION
typedef struct {
	TIM_HandleTypeDef *p_tim_handle;
	uint32_t core_clock_divider;
	uint32_t first_channel_number;
	uint32_t second_channel_number;
} Timer_Config;

#define TIMER_CLOCK_FREQUENCY 24000000
#define TIMER_TICKS_PER_MICROSECOND (TIMER_CLOCK_FREQUENCY/1000000)

static Timer_Config timer_configs[IOCX_NUM_TIMERS] =
{
	{&htim1, TIM_CLOCKDIVISION_DIV4, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim2, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim3, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim4, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim5, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_3, TIM_CHANNEL_4},
	{&htim9, TIM_CLOCKDIVISION_DIV4, TIM_CHANNEL_1, TIM_CHANNEL_2},
};

uint16_t timer_channel_ccr[IOCX_NUM_TIMERS][IOCX_NUM_CHANNELS_PER_TIMER];

//#endif

void HAL_IOCX_GPIO_Set_Config(uint8_t gpio_index, uint8_t config) {

	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;

	IOCX_GPIO_TYPE type = iocx_decode_gpio_type(&config);
	IOCX_GPIO_INPUT input = iocx_decode_gpio_input(&config);
	IOCX_GPIO_INTERRUPT interrupt_mode = iocx_decode_gpio_interrupt(&config);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = gpio_channels[gpio_index].pin;

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
		if(gpio_channels[gpio_index].interrupt_type == INT_EXTI) {
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
		HAL_GPIO_DeInit(gpio_channels[gpio_index].p_gpio, GPIO_InitStruct.Pin);
		return;
	}
	HAL_GPIO_Init(gpio_channels[gpio_index].p_gpio, &GPIO_InitStruct);
}

void HAL_IOCX_GPIO_Get(uint8_t first_gpio_index, int count, uint8_t *values)
{
	int i;
	if ( first_gpio_index > (IOCX_NUM_GPIOS-1) ) return;
	if ( first_gpio_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_gpio_index;
	}
	for ( i = first_gpio_index; i < first_gpio_index + count; i++ ) {
		*values = (HAL_GPIO_ReadPin(gpio_channels[i].p_gpio, gpio_channels[i].pin) == GPIO_PIN_SET) ?
			IOCX_GPIO_SET : IOCX_GPIO_RESET;
	}
}

void HAL_IOCX_GPIO_Set(uint8_t gpio_index, uint8_t value)
{
	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;
    HAL_GPIO_WritePin( gpio_channels[gpio_index].p_gpio, gpio_channels[gpio_index].pin,
    		(value != IOCX_GPIO_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
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

void HAL_IOCX_TIMER_Set_Config(uint8_t timer_index, uint8_t config)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;

	IOCX_TIMER_MODE mode = iocx_decode_timer_mode(&config);
	IOCX_QUAD_ENCODER_MODE qe_mode = iocx_decode_quad_encoder_mode(&config);
	switch(mode){
	case TIMER_MODE_DISABLED:
		// TODO:  IF was in PWM Mode, stop PWM?
		// TODO:  IF was in QEMode, stop QE?
		__HAL_TIM_DISABLE(timer_configs[timer_index].p_tim_handle);
		break;

	case TIMER_MODE_QUAD_ENCODER:
		{
			TIM_Encoder_InitTypeDef sConfig;
			TIM_MasterConfigTypeDef sMasterConfig;

			TIM_HandleTypeDef * p_htim = timer_configs[timer_index].p_tim_handle;
			p_htim->Init.ClockDivision = timer_configs[timer_index].core_clock_divider;
			p_htim->Init.Prescaler = p_htim->Instance->PSC;
			p_htim->Init.CounterMode = TIM_COUNTERMODE_UP;
			p_htim->Init.Period = p_htim->Instance->ARR;
			if (qe_mode == QUAD_ENCODER_MODE_1x) {
				sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
			} else { /* 2x, 4x */
				sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
			}
			if (qe_mode == QUAD_ENCODER_MODE_4x) {
				sConfig.IC1Polarity = TIM_ICPOLARITY_BOTHEDGE;
			} else {
				sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			}
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Filter = 2;
			if (qe_mode == QUAD_ENCODER_MODE_4x) {
				sConfig.IC2Polarity = TIM_ICPOLARITY_BOTHEDGE;
			} else {
				sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
			}
			sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC2Filter = 2;
			HAL_TIM_Encoder_Init(p_htim, &sConfig);

			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_TIMEx_MasterConfigSynchronization(p_htim, &sMasterConfig);

			/* Be sure to configure the auto-reload register (ARR) to the max possible value. */
			p_htim->Instance->ARR = 0xFFFF;
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
			uint32_t channel;
			IOCX_INPUT_CAPTURE_CHANNEL inputcap_channel = iocx_decode_input_capture_channel(&config);
			IOCX_INPUT_CAPTURE_POLARITY inputcap_polarity = iocx_decode_input_capture_polarity(&config);

			if (inputcap_channel == INPUT_CAPTURE_FROM_CH1) {
				channel = timer_configs[timer_index].first_channel_number;
			} else {
				channel = timer_configs[timer_index].second_channel_number;
			}

			TIM_HandleTypeDef * p_htim = timer_configs[timer_index].p_tim_handle;
			// Configure the Input Capture channel
			TIM_IC_InitTypeDef sConfig;
			sConfig.ICPrescaler = TIM_ICPSC_DIV1;
			sConfig.ICFilter = 0x0;
			if (inputcap_polarity == INPUT_CAPTURE_POLARITY_RISING) {
				sConfig.ICPolarity = TIM_ICPOLARITY_RISING;
			} else {
				sConfig.ICPolarity = TIM_ICPOLARITY_FALLING;
			}
			sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
			HAL_TIM_IC_ConfigChannel(p_htim, &sConfig, channel);
			HAL_TIM_IC_Start (p_htim, channel);
		}
		break;

	default:
		return;
	}
}

void HAL_IOCX_TIMER_Set_Control(uint8_t timer_index, uint8_t* control)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
	IOCX_TIMER_COUNTER_RESET reset = iocx_decode_timer_counter_reset(control);
	if (reset == RESET_REQUEST) {
		/* Generate a Timer Update event - which clears the counter (and prescaler counter) */
		HAL_TIM_GenerateEvent(timer_configs[timer_index].p_tim_handle, TIM_EventSource_Update);
		iocx_clear_timer_counter_reset(control);
	}
}

void HAL_IOCX_TIMER_Set_Prescaler(uint8_t timer_index, uint16_t ticks_per_clock)
{
	if ( timer_index > (IOCX_NUM_TIMERS-1) ) return;
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

void HAL_IOCX_TIMER_Get_Count(uint8_t first_timer_index, int count, int32_t *values)
{
	uint8_t i;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}
	for ( i = first_timer_index; i < first_timer_index + count; i++ ) {
		*values++ = (int32_t)timer_configs[i].p_tim_handle->Instance->CNT;
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
}

void HAL_IOCX_TIMER_PWM_Get_DutyCycle(uint8_t first_timer_index, uint8_t first_channel_index, int channel_count, uint16_t *values)
{
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_channel_index > (IOCX_NUM_CHANNELS_PER_TIMER-1) ) return;
	size_t curr_struct_index = (first_timer_index * IOCX_NUM_CHANNELS_PER_TIMER) + first_channel_index;
	size_t num_entries = (sizeof(timer_channel_ccr) / sizeof(timer_channel_ccr[0]))-1;
	if ( channel_count > (num_entries - curr_struct_index) ) {
		channel_count = num_entries - curr_struct_index;
	}
	uint16_t *p_curr_entry = &(timer_channel_ccr[first_timer_index][first_channel_index]);
	int i;
	for ( i = 0; i < channel_count; i++) {
		*values++ = *p_curr_entry++;;
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

#define MAX_NUM_ADC_OVERSAMPLE_BITS 7
#define MAX_NUM_ADC_SAMPLES (1 << MAX_NUM_ADC_OVERSAMPLE_BITS) /* MUST be power of 2 */
ADC_Samples adc_samples[MAX_NUM_ADC_SAMPLES];
int adc_enabled = 0;
#endif

/* Note:  ADC DMA is configured for 16-bit words.  The "Length" Parameter to HAL_ADC_Start_DMA */
/* below is the number of 16-bit words transferred - NOT the number of bytes.                  */

void HAL_IOCX_ADC_Enable(int enable)
{
#ifdef ENABLE_ADC
	if (enable) {
		if (adc_enabled == 0) {
			memset(&adc_samples,0, sizeof(adc_samples));
			HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1,
					(uint32_t *)&adc_samples, sizeof(adc_samples) / sizeof(uint16_t));
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

#ifdef ENABLE_ADC
static const uint32_t c_total_adc_transfers = sizeof(adc_samples) / sizeof(uint16_t);
static const uint8_t c_num_adc_transfers_per_sampleset = (sizeof(adc_samples[0]) / sizeof(uint16_t));
#endif

void HAL_IOCX_ADC_Get_Latest_Samples(int start_channel, int n_channels, uint32_t *p_oversamples, uint8_t oversample_bits, uint32_t *p_avgsamples, uint8_t avg_bits)
{
#ifdef ENABLE_ADC
	/* Range-check inputs */
	if (oversample_bits > MAX_NUM_ADC_OVERSAMPLE_BITS) {
		oversample_bits = MAX_NUM_ADC_OVERSAMPLE_BITS;
	}
	if (avg_bits > MAX_NUM_ADC_OVERSAMPLE_BITS) {
		avg_bits = MAX_NUM_ADC_OVERSAMPLE_BITS;
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
	uint32_t last_valid_adc_transfer = c_total_adc_transfers - remaining_adc_transfers;
	uint32_t total_adc_transfer_sets = sizeof(adc_samples) / c_num_adc_transfers_per_sampleset;

	uint16_t last_valid_adc_transfer_set = last_valid_adc_transfer / c_num_adc_transfers_per_sampleset;
	uint16_t first_valid_adc_transfer_set;
	if (num_samples_to_accumulate <= last_valid_adc_transfer_set) {
		first_valid_adc_transfer_set = last_valid_adc_transfer_set - num_samples_to_accumulate;
	} else {
		first_valid_adc_transfer_set = total_adc_transfer_sets - num_samples_to_accumulate;
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

#define ADC_VOLTAGE_OVERSAMPLE_BITS 3
#define ADC_VOLTAGE_AVERAGE_BITS ADC_VOLTAGE_OVERSAMPLE_BITS
/* Returns 0 if 3.3V, non-zero of 5V VDA */
int HAL_IOCX_ADC_Voltage_5V( int n_samples_to_average )
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint32_t oversample;
		uint32_t average;
		HAL_IOCX_ADC_Get_Latest_Samples(ADC_CHANNEL_ANALOG_VOLTAGE_SWITCH, 1, &oversample, ADC_VOLTAGE_OVERSAMPLE_BITS,
				&average, ADC_VOLTAGE_AVERAGE_BITS);
		return (average > 3500) ? 1 : 0;
	} else {
		return 0;
	}
#else
    return 0;
#endif
}

#define ADC_EXT_POWER_OVERSAMPLE_BITS 3
#define ADC_EXT_POWER_AVERAGE_BITS ADC_EXT_POWER_OVERSAMPLE_BITS
const float vdiv_ratio = 3.24f / (11.5f + 3.24f);

uint16_t HAL_IOCX_Get_ExtPower_Voltage()
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint32_t oversample;
		uint32_t average;
		HAL_IOCX_ADC_Get_Latest_Samples(ADC_CHANNEL_EXT_POWER_VOLTAGE_DIV, 1, &oversample,
				ADC_EXT_POWER_OVERSAMPLE_BITS, &average, ADC_EXT_POWER_AVERAGE_BITS);
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

