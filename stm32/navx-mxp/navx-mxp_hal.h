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
#else
#   define NAVX_HARDWARE_REV 33         /* v. 3.3 EXPIO */
#endif

struct unique_id
{
	uint32_t first;
	uint32_t second;
	uint32_t third;
};

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

/* RN4020 Access */
void HAL_RN4020_IO_Init();
void HAL_RN4020_Wake();
void HAL_RN4020_Sleep();
int HAL_RN4020_Get_MLDP_EV();
int HAL_RN4020_Get_RTS();
void HAL_RN4020_Set_CTS(int value);
void HAL_RN4020_Set_CMD_MLDP(int value);

#endif /* NAVX_MXP_HAL_H_ */
