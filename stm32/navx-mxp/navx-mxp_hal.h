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

#ifndef NAVX_MXP_HAL_H_
#define NAVX_MXP_HAL_H_

#include <stdint.h>

/* The navX-Model device variants are comprised of different
 * hardware features, which are not available on each device.
 * All of these variants are defined below.  Change them
 * very carefully!
 */

#if defined(NAVX_BOARDTYPE_MICRO_1_0)
#   define NAVX_HARDWARE_REV 40
#   define ENABLE_USB_VBUS
#   define DISABLE_EXTERNAL_SPI_INTERFACE
#elif defined(NAVX_BOARDTYPE_BLUE_1_0)
#   define NAVX_HARDWARE_REV 50
#   define ENABLE_USB_VBUS
#   define DISABLE_EXTERNAL_I2C_INTERFACE
#   define DISABLE_EXTERNAL_SPI_INTERFACE
#   define ENABLE_RN4020
#elif defined(NAVX_BOARDTYPE_PI_1_0)
#	define NAVX_HARDWARE_REV 60
#	define ENABLE_USB_VBUS
#   define DISABLE_COMM_DIP_SWITCHES
#   define DISABLE_EXTERNAL_I2C_INTERFACE /* ??? */
#   define DISABLE_EXTERNAL_UART_INTERFACE
#	define ENABLE_CAN_TRANSCEIVER
#	define ENABLE_QUAD_DECODERS
#	define ENABLE_PWM_GENERATION
#	define ENABLE_ADC
#	define GPIO_MAP_NAVX_PI
#	define EXTERNAL_SPI_INTERFACE_MODE_0
#	define NAVX_PI_BOARD_TEST /* Undef this for production. */
#   define ENABLE_BANKED_REGISTERS
#   define ENABLE_IOCX
#	define ENABLE_SPI_COMM_READY_INTERRUPT
#   define ENABLE_LSE
#   define ENABLE_RTC
#   define ENABLE_MISC
#	define IOCX_INTERRUPT
#	define CAN_INTERRUPT
#	define AHRS_INTERRUPT
#else
#   define NAVX_HARDWARE_REV 33         /* v. 3.3 EXPIO, v3.4, v3.5 */
//  !ENABLE_USB_VBUS
//  !DISABLE_EXTERNAL_SPI_INTERFACE
//  !DISABLE_EXTERNAL_I2C_INTERFACE
//  !ENABLE_RN4020
//  !DISABLE_COMM_DIP_SWITCHES
//  !ENABLE_CAN_TRANSCEIVER
//  !ENABLE_QUAD_DECODERS
//  !GPIO_MAP_NAVX_PI
//  !EXTERNAL_SPI_INTERFACE_MODE_0
#endif

#ifdef GPIO_MAP_NAVX_PI
#include "gpiomap_navx-pi.h"
#else
#include "gpiomap_navx-mxp.h"
#endif

struct unique_id
{
	uint32_t first;
	uint32_t second;
	uint32_t third;
};

/*check if the compiler is of C++*/
#ifdef __cplusplus
extern "C" {
#endif

void read_unique_id(struct unique_id *id);
void HAL_LED_Init();
void HAL_I2C_Power_Init();
void HAL_CAL_Button_Init();
void HAL_DIP_Switches_Init();
void HAL_LED1_Toggle();
void HAL_LED2_Toggle();
void HAL_LED1_On(int on);
void HAL_LED2_On(int on);
void HAL_CAL_LED_Toggle();
void HAL_CAL_LED_On(int on);
int  HAL_CAL_Button_Pressed();
void HAL_I2C_Power_On();
void HAL_I2C_Power_Off();
int  HAL_SPI_Slave_Enabled();
int  HAL_UART_Slave_Enabled();

void HAL_SPI_Comm_Ready_Assert();
void HAL_SPI_Comm_Ready_Deassert();
void HAL_CAN_Int_Assert();
void HAL_CAN_Int_Deassert();
void HAL_AHRS_Int_Assert();
void HAL_AHRS_Int_Deassert();

void HAL_CAN_Status_LED_On(int on);

/**********************/
/* RTC                */
/**********************/

void HAL_RTC_InitializeCache();
void HAL_RTC_Get_Time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint32_t *subseconds);
void HAL_RTC_Get_Date(uint8_t *weekday, uint8_t *date, uint8_t *month, uint8_t *year);
void HAL_RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds);
void HAL_RTC_Set_Date(uint8_t day, uint8_t date, uint8_t month, uint8_t year);

/* Daylight savings:  0:  No adjustment, 1:  Add 1 hour, 2:  Subtract 1 hour */
void HAL_RTC_Get_DaylightSavings(uint8_t *daylight_savings);
void HAL_RTC_Set_DaylightSavings(uint8_t daylight_savings);

/**********************/
/* Reconfigurable IOs */
/**********************/

/* External IO Power Management */
void HAL_IOCX_Ext_Power_Enable(int enable);

/* GPIOs */
void HAL_IOCX_GPIO_Set_Config(uint8_t gpio_index, uint8_t config);
void HAL_IOCX_GPIO_Get_Config(uint8_t first_gpio_index, int count, uint8_t *values);
void HAL_IOCX_GPIO_Set(uint8_t gpio_index, uint8_t value);
void HAL_IOCX_GPIO_Get(uint8_t first_gpio_index, int count, uint8_t *values);
int HAL_IOCX_RPI_GPIO_Output(); /* Returns 0 if pins are input, non-zero if output */
int HAL_IOCX_Ext_Power_Fault(); /* Returns 0 if no ext power fault has occurred, non-zero indicates fault has occurred. */

#ifdef ENABLE_IOCX
void HAL_IOCX_RPI_GPIO_Driver_Enable(int enable);
void HAL_IOCX_RPI_COMM_Driver_Enable(int enable);
void HAL_IOCX_Init();
void HAL_IOCX_AssertInterrupt(uint16_t int_bits_to_set);
void HAL_IOCX_DeassertInterrupt(uint16_t int_bits_to_clear);
void HAL_IOCX_UpdateInterruptMask(uint16_t int_new_mask);
uint16_t HAL_IOCX_GetInterruptMask();
uint16_t HAL_IOCX_GetInterruptStatus();
uint16_t HAL_IOCX_GetLastInterruptEdges();
#endif

/* Timers (QuadEncoder/PWM) */
void HAL_IOCX_TIMER_Set_Config(uint8_t timer_index, uint8_t config);
void HAL_IOCX_TIMER_Set_Control(uint8_t timer_index, uint8_t* control); /* E.g., Reset Counter */
void HAL_IOCX_TIMER_Enable_Clocks(uint8_t timer_index, int enable); /* Internal use only? */
void HAL_IOCX_TIMER_Get_Config(uint8_t first_timer_index, int count, uint8_t *values);
void HAL_IOCX_TIMER_Set_Prescaler(uint8_t timer_index, uint16_t ticks_per_clock);
void HAL_IOCX_TIMER_Get_Prescaler(uint8_t first_timer_index, int count, uint16_t *values);

/* Quad Encoder Data */
void HAL_IOCX_TIMER_Get_Count(uint8_t first_timer_index, int count, int32_t *values);
void HAL_IOCX_TIMER_Get_Direction(uint8_t first_timer_index, int count, uint8_t *values);

/* PWM Configuration */
void HAL_IOCX_TIMER_PWM_Set_FramePeriod(uint8_t timer_index, uint16_t clocks_per_frame_period);
void HAL_IOCX_TIMER_PWM_Get_FramePeriod(uint8_t first_timer_index, int count, uint16_t* values);
void HAL_IOCX_TIMER_PWM_Set_DutyCycle(uint8_t timer_index, uint8_t channel_index, uint16_t clocks_per_active_period);
void HAL_IOCX_TIMER_PWM_Get_DutyCycle(uint8_t first_timer_index, uint8_t first_channel_index, int count, uint16_t *values);

/* ADC Access */

void HAL_IOCX_ADC_Enable(int enable);
void HAL_IOCX_ADC_Get_Samples( int start_channel, int n_channels, uint16_t* p_samples, uint8_t n_samples_to_average );
void HAL_IOCX_ADC_Get_Latest_Samples(int start_channel, int n_channels, uint32_t *p_oversamples,
		uint8_t num_oversample_bits, uint32_t *p_avgsamples, uint8_t num_avg_bits);
int HAL_IOCX_ADC_Voltage_5V(); /* Returns 0 if 3.3V, non-zero of 5V VDA */
uint16_t HAL_IOCX_Get_ExtPower_Voltage();

void HAL_IOCX_ADC_AWDG_Disable();
void HAL_IOCX_ADC_AWDG_Enable(uint8_t channel);
int HAL_IOCX_ADC_AWDG_Is_Enabled(uint8_t* p_channel); /* Returns non-zero if enabled */
uint16_t HAL_IOCX_ADC_AWDG_Get_Threshold_High();
uint16_t HAL_IOCX_ADC_AWDG_Get_Threshold_Low();
void HAL_IOCX_ADC_AWDG_Set_Threshold_High(uint16_t threshold);
void HAL_IOCX_ADC_AWDG_Set_Threshold_Low(uint16_t threshold);
int HAL_IOCX_ADC_AWDG_Get_Interrupt_Enable();
void HAL_IOCX_ADC_AWDG_Set_Interrupt_Enable(int enable);

/* RN4020 Access */
void HAL_RN4020_IO_Init();
void HAL_RN4020_Wake();
void HAL_RN4020_Sleep();
int HAL_RN4020_Get_MLDP_EV();
int HAL_RN4020_Get_RTS();
void HAL_RN4020_Set_CTS(int value);
void HAL_RN4020_Set_CMD_MLDP(int value);

struct WritableRegSet
{
	uint8_t start_offset;
	uint8_t num_bytes;
	void (*changed)(uint8_t first_offset, uint8_t count);
};

struct WritableRegSetGroup
{
	uint8_t first_offset;
	uint8_t last_offset;
	struct WritableRegSet *p_sets;
	uint8_t set_count;
};

#define SIZEOF_STRUCT(s) (sizeof(s)/sizeof(s[0]))

#ifdef __cplusplus
}
#endif


#endif /* NAVX_MXP_HAL_H_ */
