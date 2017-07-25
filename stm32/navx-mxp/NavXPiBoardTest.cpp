/*
 * NavXPiBoardTest.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: Scott
 */

#include "NavXPiBoardTest.h"
#include "navx-mxp_hal.h"
#include "IOCXRegisters.h"

NavXPiBoardTest::NavXPiBoardTest() {
	/* Configure PWM output frequency */
	uint8_t gpio_cfg = 0;
	iocx_encode_gpio_type(&gpio_cfg, GPIO_TYPE_AF);
	HAL_IOCX_GPIO_Set_Config(0, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(1, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(2, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(3, gpio_cfg);

	HAL_IOCX_TIMER_Set_Prescaler(4,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(4, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(4, 0, 1000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(4, 1, 2000);
	HAL_IOCX_TIMER_Set_Prescaler(5,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(5, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(5, 0, 3000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(5, 1, 4000);
	uint8_t timer_cfg = 0;
	iocx_encode_timer_mode(&timer_cfg, TIMER_MODE_PWM_OUT);
	HAL_IOCX_TIMER_Set_Config(4, timer_cfg);
	HAL_IOCX_TIMER_Set_Config(5, timer_cfg);

	/* Enable all Quad Encoder interfaces */
	iocx_encode_gpio_type(&gpio_cfg, GPIO_TYPE_AF);
	HAL_IOCX_GPIO_Set_Config(4, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(5, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(6, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(7, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(8, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(9, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(10, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(11, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(12, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(13, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(14, gpio_cfg);
	HAL_IOCX_GPIO_Set_Config(15, gpio_cfg);

#if 0
	iocx_encode_timer_mode(&timer_cfg, TIMER_MODE_QUAD_ENCODER);
#else
	iocx_encode_timer_mode(&timer_cfg, TIMER_MODE_PWM_OUT);
	HAL_IOCX_TIMER_Set_Prescaler(0,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(0, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(0, 0, 5000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(0, 1, 6000);
	HAL_IOCX_TIMER_Set_Prescaler(1,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(1, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(1, 0, 7000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(1, 1, 8000);
	HAL_IOCX_TIMER_Set_Prescaler(2,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(2, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(2, 0, 9000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(2, 1,10000);
	HAL_IOCX_TIMER_Set_Prescaler(3,48);
	HAL_IOCX_TIMER_PWM_Set_FramePeriod(3, 20000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(3, 0,11000);
	HAL_IOCX_TIMER_PWM_Set_DutyCycle(3, 1,12000);
#endif
	HAL_IOCX_TIMER_Set_Config(0, timer_cfg);
	HAL_IOCX_TIMER_Set_Config(1, timer_cfg);
	HAL_IOCX_TIMER_Set_Config(2, timer_cfg);
	HAL_IOCX_TIMER_Set_Config(3, timer_cfg);

	/* Enable the ADC */
	HAL_IOCX_ADC_Enable(1);
}

NavXPiBoardTest::~NavXPiBoardTest() {
}

static uint8_t oversample_bits = 4;
static uint8_t average_bits = 4;
static uint32_t adc_oversamples[4];
static uint32_t adc_avg_samples[4];
static int32_t encoder_counts[4];
static bool is_adc_5V = false;
static float ext_pwr_voltage = 0.0f;
void NavXPiBoardTest::loop() {
	HAL_IOCX_ADC_Get_Latest_Samples(0, 4, adc_oversamples, oversample_bits, adc_avg_samples, average_bits);
	HAL_IOCX_TIMER_Get_Count(2, 4, encoder_counts);

	is_adc_5V = (HAL_IOCX_ADC_Voltage_5V() != 0);
	ext_pwr_voltage = HAL_IOCX_Get_ExtPower_Voltage();
}
