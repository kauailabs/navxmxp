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

/* The navX-Family device variants are comprised of different
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

/* PWM Generator Access */
void HAL_PWM_Set_Rate(int channel, uint32_t frequency_us, uint32_t duty_cycle_us);
void HAL_PWM_Enable(int channel, int enable);
int HAL_PWM_Get_Num_Channels();

/* Quadrature Encoder Access */
uint32_t HAL_QuadEncoder_Get_Count(int channel);
void HAL_QuadEncoder_Enable(int channel, int enable);
int HAL_QuadEncoder_Get_Num_Channels();

/* ADC Access */

#define NUM_NAVX_ADC_CHANNELS 4
typedef struct {
	uint16_t data[NUM_NAVX_ADC_CHANNELS];
} ADC_Samples;

void HAL_ADC_Enable(int enable);
void HAL_ADC_Get_Samples( ADC_Samples* p_samples, uint8_t n_samples_to_average );
void HAL_ADC_Get_NumChannels();

/* RN4020 Access */
void HAL_RN4020_IO_Init();
void HAL_RN4020_Wake();
void HAL_RN4020_Sleep();
int HAL_RN4020_Get_MLDP_EV();
int HAL_RN4020_Get_RTS();
void HAL_RN4020_Set_CTS(int value);
void HAL_RN4020_Set_CMD_MLDP(int value);

/* MCP25625 CAN Interface Access */
void HAL_MCP25625_IO_Init();
void HAL_MCP25625_Wake();
void HAL_MCP25625_Sleep();
void HAL_MCP25625_Test();

#ifdef __cplusplus
}
#endif


#endif /* NAVX_MXP_HAL_H_ */
