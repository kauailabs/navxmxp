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
#include "tim_navx-pi.h"
#include "adc_navx-pi.h"

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


#ifdef ENABLE_PWM_GENERATION
typedef struct {
	TIM_HandleTypeDef *p_tim_handle;
	uint32_t tim_channel_number;
} PWM_Timer_Channel;

#define TIMER_CLOCK_FREQUENCY 48000000
#define TIMER_TICKS_PER_MICROSECOND (TIMER_CLOCK_FREQUENCY/1000000)

static PWM_Timer_Channel pwm_timer_channels[10] =
{
		{&htim2, TIM_CHANNEL_3},
		{&htim2, TIM_CHANNEL_4},
		{&htim5, TIM_CHANNEL_2},
		{&htim5, TIM_CHANNEL_1},
		{&htim1, TIM_CHANNEL_4},
		{&htim1, TIM_CHANNEL_3},
		{&htim3, TIM_CHANNEL_4},
		{&htim3, TIM_CHANNEL_3},
		{&htim4, TIM_CHANNEL_3},
		{&htim4, TIM_CHANNEL_4},
};
#endif

/* PWM Mode signals are generated with a frequency determined by the value  */
/* of the TIMx_ARR register and a duty cycle determined by the value of the */
/* TIMx_CCRx register                                                       */
/* NOTE:  Since the QuadEncoder interface also uses the ARR register, a     */
/* single Timer's QuadEncoder interface and PWM Generator cannot be used    */
/* simultaneously - unless the ARR register value works well for both.      */

void HAL_PWM_Set_Rate(int channel, uint32_t frequency_us, uint32_t duty_cycle_us)
{
#ifdef ENABLE_PWM_GENERATION
	uint32_t frequency = frequency_us * TIMER_TICKS_PER_MICROSECOND;
	uint32_t duty_cycle = duty_cycle_us * TIMER_TICKS_PER_MICROSECOND;
	if ( channel < sizeof(pwm_timer_channels)/sizeof(pwm_timer_channels[0])) {
		__HAL_TIM_SetAutoreload(pwm_timer_channels[channel].p_tim_handle, duty_cycle);
		__HAL_TIM_SetCompare(pwm_timer_channels[channel].p_tim_handle,
				pwm_timer_channels[channel].tim_channel_number, frequency);
	}
#endif
}

void HAL_PWM_Enable(int channel, int enable)
{
#ifdef ENABLE_PWM_GENERATION
	if ( channel < sizeof(pwm_timer_channels)/sizeof(pwm_timer_channels[0])) {
		if ( enable != 0 ) {
			HAL_TIM_PWM_Start(pwm_timer_channels[channel].p_tim_handle,
					pwm_timer_channels[channel].tim_channel_number);
		} else {
			HAL_TIM_PWM_Stop(pwm_timer_channels[channel].p_tim_handle,
					pwm_timer_channels[channel].tim_channel_number);
		}
	}
#endif
}

int HAL_PWM_Get_Num_Channels()
{
#ifdef ENABLE_PWM_GENERATION
	return sizeof(pwm_timer_channels)/sizeof(pwm_timer_channels[0]);
#else
	return 0;
#endif
}

#ifdef ENABLE_QUAD_DECODERS
TIM_HandleTypeDef *p_quad_encoder_channels[4] =
{
		&htim1,
		&htim2,
		&htim3,
		&htim4
};
#endif

uint32_t HAL_QuadEncoder_Get_Count(int channel)
{
#ifdef ENABLE_QUAD_DECODERS
	uint32_t curr_count;
	if (channel < (sizeof(p_quad_encoder_channels)/sizeof(p_quad_encoder_channels[0]))){
		curr_count = __HAL_TIM_GetCounter(p_quad_encoder_channels[channel]);
	} else {
		curr_count = 0;
	}
	return curr_count;
#else
	return 0;
#endif
}

int HAL_QuadEncoder_Get_Num_Channels()
{
#ifdef ENABLE_QUAD_DECODERS
	return (sizeof(p_quad_encoder_channels)/sizeof(p_quad_encoder_channels[0]));
#else
	return 0;
#endif
}

#define BOTH_ENCODER_TIMER_CHANNELS 0xFF /*(TIM_CHANNEL_1 and TIM_CHANNEL_2)*/

/* Quad Encoder pulses are input on Timer Channel 1 and 2.  Direction is    */
/* determined by the DIR bit; the direction cause the count to be increased */
/* or decreased.  The CNT register contains the current count.  The CNT     */
/* register is reset to 0 when it's value reaches that of the ARR register. */
/*                                                                          */
/* NOTE:  Since the PWM Generator also uses the ARR register, a single      */
/* Timer's QuadEncoder interface and PWM Generator cannot be used           */
/* simultaneously - unless the ARR register value works well for both.      */

void HAL_QuadEncoder_Enable(int channel, int enable)
{
#ifdef ENABLE_QUAD_DECODERS
	if (channel < (sizeof(p_quad_encoder_channels)/sizeof(p_quad_encoder_channels[0]))){
		if (enable != 0 ) {
			/* Be sure to configure the auto-reload register (ARR) to the max possible value. */
			p_quad_encoder_channels[channel]->Instance->ARR = 0xFFFF;
			HAL_TIM_Encoder_Start(p_quad_encoder_channels[channel], BOTH_ENCODER_TIMER_CHANNELS);
		} else {
			HAL_TIM_Encoder_Stop(p_quad_encoder_channels[channel], BOTH_ENCODER_TIMER_CHANNELS);
		}
	}
#endif
}

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

#define ADC_CLOCK_FREQUENCY 24000000
#define ADC_CLOCKS_PER_SAMPLE 40
#define ADC_NUM_CHANNELS 4

ADC_Samples adc_samples[100];
int adc_enabled = 0;

/* Danger!!!!!:  as of 8/27/2016, HAL_ADC_Enable() was found to be overwriting */
/* expected memory and causing a hard fault!                                   */

void HAL_ADC_Enable(int enable)
{
#ifdef ENABLE_ADC
	if ( (adc_enabled == 0) && (enable != 0) ) {
		memset(&adc_samples,0, sizeof(adc_samples));
		HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&adc_samples, sizeof(adc_samples));
		adc_enabled = ( status == HAL_OK ) ? 1 : 0;
	} else {
		if (adc_enabled != 0) {
			HAL_ADC_Stop_DMA(&hadc1);
		}
	}
#endif
}

void HAL_ADC_Get_Samples( ADC_Samples* p_samples, uint8_t n_samples_to_average )
{
#ifdef ENABLE_ADC
	if ( n_samples_to_average > (sizeof(adc_samples)/sizeof(adc_samples[0]))) {
		n_samples_to_average = (sizeof(adc_samples)/sizeof(adc_samples[0]));
	}
	uint32_t accum[ADC_NUM_CHANNELS] = {0,0,0,0};
	int i, j;
	for ( i = 0; i < n_samples_to_average; i++ ) {
		for ( j = 0; j < ADC_NUM_CHANNELS; j++ ) {
			accum[j] += adc_samples[i].data[j];
		}
	}
	for ( i = 0; i < ADC_NUM_CHANNELS; i++ ) {
		p_samples->data[i] = accum[i] / n_samples_to_average;
	}
#endif
}

int HAL_ADC_Get_Num_Channels()
{
#ifdef ENABLE_ADC
	return ADC_NUM_CHANNELS;
#else
	return 0;
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

extern SPI_HandleTypeDef hspi2; /* TODO:  Relocate this */

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
