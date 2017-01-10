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

#ifdef ENABLE_IOCX
#include "gpiomap_navx-pi.h"
#include "adc_navx-pi.h"
#include "IOCXRegisters.h"
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

#ifdef ENABLE_IOCX
typedef struct {
	GPIO_TypeDef *p_gpio;
	uint16_t pin;
	uint8_t timer_index;
	uint8_t channel_index;
	uint8_t alt_function;
} GPIO_Channel;

#define MAX_VALID_STM32_AF_VALUE 0x0F
#define GPIO_AF_SW_RESET_COUNTER (MAX_VALID_STM32_AF_VALUE + 1)

static GPIO_Channel gpio_channels[IOCX_NUM_GPIOS] =
{
	{PWM_GPIO1_GPIO_Port, 	PWM_GPIO1_Pin, 	4,  0, GPIO_AF2_TIM5},
	{PWM_GPIO2_GPIO_Port, 	PWM_GPIO2_Pin, 	4,  1, GPIO_AF2_TIM5},
	{PWM_GPIO3_GPIO_Port, 	PWM_GPIO3_Pin, 	5,  0, GPIO_AF3_TIM9},
	{PWM_GPIO4_GPIO_Port, 	PWM_GPIO4_Pin, 	5,  1, GPIO_AF3_TIM9},
	{QE1_IDX_GPIO_Port, 	QE1_IDX_Pin, 	0, -1, GPIO_AF_SW_RESET_COUNTER},
	{QE1_A_GPIO_Port, 		QE1_A_Pin, 		0,  0, GPIO_AF1_TIM1},
	{QE1_B_GPIO_Port, 		QE1_B_Pin, 		0,  1, GPIO_AF1_TIM1},
	{QE2_IDX_GPIO_Port, 	QE2_IDX_Pin, 	1, -1, GPIO_AF_SW_RESET_COUNTER},
	{QE2_A_GPIO_Port, 		QE2_A_Pin, 		1,  0, GPIO_AF1_TIM2},
	{QE2_B_GPIO_Port, 		QE2_B_Pin, 		1,  1, GPIO_AF1_TIM2},
	{QE3_IDX_GPIO_Port, 	QE3_IDX_Pin, 	2, -1, GPIO_AF_SW_RESET_COUNTER},
	{QE3_A_GPIO_Port, 		QE3_A_Pin, 		2,  0, GPIO_AF2_TIM3},
	{QE3_B_GPIO_Port, 		QE3_B_Pin, 		2,  1, GPIO_AF2_TIM3},
	{QE4_IDX_GPIO_Port, 	QE4_IDX_Pin, 	3, -1, GPIO_AF_SW_RESET_COUNTER},
	{QE4_A_GPIO_Port, 		QE4_A_Pin, 		3,  0, GPIO_AF2_TIM5},
	{QE4_B_GPIO_Port, 		QE4_B_Pin, 		3,  1, GPIO_AF2_TIM5},
};

/***************************************************************************/
/* Timer section                                                           */
/*                                                                         */
/* Each timer is clocked off of either APB1 or APB2, as follows:           */
/*                                                                         */
/* APB1 Timer Clocks:  48Mhz (TIM2, TIM3, TIM4, TIM5)					   */
/* APB2 Timer Clocks:  96Mhz (TIM1, TIM9, TIM10, TIM11)					   */
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

//#ifdef ENABLE_PWM_GENERATION
typedef struct {
	TIM_HandleTypeDef *p_tim_handle;
	uint32_t core_clock_divider;
	uint32_t first_channel_number;
	uint32_t second_channel_number;
} Timer_Config;

#define TIMER_CLOCK_FREQUENCY 48000000
#define TIMER_TICKS_PER_MICROSECOND (TIMER_CLOCK_FREQUENCY/1000000)

static Timer_Config timer_configs[IOCX_NUM_TIMERS] =
{
	{&htim1, TIM_CLOCKDIVISION_DIV2, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim2, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim3, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim4, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
	{&htim5, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_3, TIM_CHANNEL_4},
	{&htim9, TIM_CLOCKDIVISION_DIV1, TIM_CHANNEL_1, TIM_CHANNEL_2},
};

uint16_t timer_channel_ccr[IOCX_NUM_TIMERS][IOCX_NUM_CHANNELS_PER_TIMER];

//#endif

void HAL_IOCX_GPIO_Set_Config(uint8_t gpio_index, uint8_t config) {

	if ( gpio_index > (IOCX_NUM_GPIOS-1) ) return;

	IOCX_GPIO_TYPE type = iocx_decode_gpio_type(&config);
	IOCX_GPIO_INPUT input = iocx_decode_gpio_input(&config);
	//IOCX_GPIO_INTERRUPT interrupt = iocx_decode_gpio_interrupt(&config); // TODO:  Implement

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
			sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
			sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Filter = 2;
			sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
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

void HAL_IOCX_TIMER_Get_Count(uint8_t first_timer_index, int count, uint16_t *values)
{
	uint8_t i;
	if ( first_timer_index > (IOCX_NUM_TIMERS-1) ) return;
	if ( first_timer_index + count > IOCX_NUM_TIMERS) {
		count = IOCX_NUM_TIMERS - first_timer_index;
	}
	for ( i = first_timer_index; i < first_timer_index + count; i++ ) {
		*values++ = (uint16_t)timer_configs[i].p_tim_handle->Instance->CNT;
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
/* PCLK2, which is configured for 48Mhz, and an internal divide-by-8       */
/* configuration yields an ADC clock rate (fADC) of 6Mhz.                  */
/*                                                                         */
/* The STM32 is a Successive-Approximation ADC, configured for 12-bit      */
/* resolution.  Each sample requires 1 clock for each bit of resolution    */
/* plus a sample period.  Tests have shown longer sample periods reduce    */
/* the impact of noise, and the currently-selected sample period is 28     */
/* clocks.  This yields an overall sample rate of 150Ksps.                 */
/*                                                                         */
/* Since navX features 4 ADC input channels, the sample rate for each      */
/* channel is 37.5ksps.                                                    */
/***************************************************************************/
/* Input Channel Mapping:                                                  */
/* An Connector 1:  ADC1_IN9                                               */
/* An Connector 2:  ADC1_IN8                                               */
/* An Connector 3:  ADC1_IN15                                              */
/* An Connector 4:  ADC1_IN14                                              */
/***************************************************************************/
/* ADC1 is configured in scan conversion mode, scanning each channel in    */
/* order of connector number.                                              */
/* DMA Transfer of data to SRAM is used.  The DMA transfer is configured   */
/* in circular mode, and a double buffer is used.  In this way, ADC data   */
/* is continually transferred into the buffers, and software can read out  */
/* data from the currently inactive buffer, avoiding contention.           */
/***************************************************************************/

#ifdef ENABLE_ADC
#define ADC_CLOCK_FREQUENCY 24000000
#define ADC_CLOCKS_PER_SAMPLE 40
#define ADC_NUM_CHANNELS 6
#define ADC_CHANNEL_ANALOG_VOLTAGE_SWITCH 4
#define ADC_CHANNEL_EXT_POWER_VOLTAGE_DIV 5
/* NOTE:  The last two ADC channels are reserved for internal use; the preceding  */
/* ADC channels are user GPIOs.                                                   */
/* The next-to-last channel is the 5V vs 3.3V Analog Input Voltage measure        */
/*   (if this value is > 2.5V, then the Analog Voltage Input High Level is 5V)    */
/* The last channel measures the 12VDC input voltage (w/a voltage divider 4.45:1, */
/*   to ensure that the max voltage does not exceed 3.3V).                        */

typedef struct {
	uint16_t data[ADC_NUM_CHANNELS];
} ADC_Samples;

#define MAX_NUM_ADC_SAMPLES 100
ADC_Samples adc_samples[MAX_NUM_ADC_SAMPLES];
int adc_enabled = 0;
#endif

/* Note:  ADC DMA is configured for 16-bit words.  The "Length" Parameter to HAL_ADC_Start_DMA */
/* below is the number of 16-bit words transferred - NOT the number of bytes.                  */

void HAL_IOCX_ADC_Enable(int enable)
{
#ifdef ENABLE_ADC
	if ( (adc_enabled == 0) && (enable != 0) ) {
		memset(&adc_samples,0, sizeof(adc_samples));
		HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1,
				(uint32_t *)&adc_samples, sizeof(adc_samples) / sizeof(uint16_t));
		adc_enabled = ( status == HAL_OK ) ? 1 : 0;
	} else {
		if (adc_enabled != 0) {
			HAL_ADC_Stop_DMA(&hadc1);
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

#define ADC_VOLTAGE_SAMPLES_TO_AVG 10
/* Returns 0 if 3.3V, non-zero of 5V VDA */
int HAL_IOCX_ADC_Voltage_5V( int n_samples_to_average )
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint16_t sample;
		HAL_IOCX_ADC_Get_Samples(ADC_CHANNEL_ANALOG_VOLTAGE_SWITCH, 1, &sample, ADC_VOLTAGE_SAMPLES_TO_AVG);
		return (sample > 3500) ? 1 : 0;
	} else {
		return 0;
	}
#else
    return 0;
#endif
}

#define ADC_EXT_POWER_SAMPLES_TO_AVG 10
const float vdiv_ratio = 3.24f / (11.5f + 3.24f);

float HAL_IOCX_Get_ExtPower_Voltage()
{
#ifdef ENABLE_ADC
	if ( adc_enabled ) {
		uint16_t sample;
		HAL_IOCX_ADC_Get_Samples(ADC_CHANNEL_EXT_POWER_VOLTAGE_DIV, 1, &sample, ADC_EXT_POWER_SAMPLES_TO_AVG);
		float ext_pwr_voltage = (((float)sample) / 4096) * 3.3f;
		ext_pwr_voltage *= (1.0f/vdiv_ratio);
		return ext_pwr_voltage;
	} else {
		return 0.0f;
	}
#else
    return 0.0f;
#endif
}

/* MCP25625 CAN Interface Access */
/* The CAN is an SPI slave, connected to the
 * STM32 via SPI2, using Mode 0.
 */


void HAL_MCP25625_Wake()
{
#ifdef ENABLE_CAN_TRANSCEIVER
	/* To enter the normal mode of operation, apply a low level to the STBY pin. */
	/* Also, make the sure the CAN RESET signal is High. */
	/* After this, the MCP25625 should be reachable on the SPI2 bus. */
	/* NOTE:  AFter power on, the MCP25625 automatically enters configuration mode. */

    HAL_GPIO_WritePin(_CAN_RESET_GPIO_Port,_CAN_RESET_Pin, GPIO_PIN_SET );
    HAL_GPIO_WritePin(CAN_STANDBY_GPIO_Port,CAN_STANDBY_Pin, GPIO_PIN_RESET );
#endif // ENABLE_CAN_TRANSCEIVER
}

#define MCP25625_SPI_READ  0x03 /* Followed by address, then 1 or more read bytes */
#define MCP25625_SPI_WRITE 0x02 /* Followed by address, then 1 or more written bytes */
#define MCP25625_STATUS_REG 0x0E
#define MCP25625_CTL_REG    0x0F

#ifdef ENABLE_CAN_TRANSCEIVER
extern SPI_HandleTypeDef hspi2; /* TODO:  Relocate this */
#endif

void HAL_MCP25625_Test()
{
#ifdef ENABLE_CAN_TRANSCEIVER
	uint8_t spi_cmd[4];
	spi_cmd[0] = MCP25625_SPI_READ;
	spi_cmd[1] = MCP25625_STATUS_REG;

	uint8_t spi_rx[4];
	spi_rx[0] = 0;
	spi_rx[1] = 0;
	spi_rx[2] = 0;
	spi_rx[3] = 0;

	// Perform SPI transaction, sending the first 2 bytes, then reading back two bytes, corresponding to
	// the first (status) register and the second (control) register.
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2, spi_cmd, spi_rx, 4, 1000);
	if ( spi_status == HAL_OK ) {
		uint8_t status_reg = spi_rx[2];
		uint8_t ctl_reg = spi_rx[3];

		spi_cmd[0] = MCP25625_SPI_WRITE;
		spi_cmd[1] = MCP25625_CTL_REG;
		spi_cmd[2] = (ctl_reg & 0x1F) | 0x20; /* Current settings, but change mode to sleep */

		spi_status = HAL_SPI_Transmit(&hspi2, spi_cmd, 3, 1000);

		if ( spi_status == HAL_OK ) {
			spi_cmd[0] = MCP25625_SPI_READ;
			spi_cmd[1] = MCP25625_STATUS_REG;
			spi_status = HAL_SPI_TransmitReceive(&hspi2, spi_cmd, spi_rx, 4, 1000);
			status_reg = spi_rx[2];
			ctl_reg = spi_rx[3];
			spi_rx[0] = spi_rx[3];
		}
	}
#endif // ENABLE_CAN_TRANSCEIVER
}

void HAL_MCP25625_Sleep()
{
#ifdef ENABLE_CAN_TRANSCEIVER
#endif // ENABLE_CAN_TRANSCEIVER
}
