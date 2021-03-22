/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

// NOTE:  This file does not contain definitions for I2C, SPI and UART peripherals

#define S2_LED_Pin GPIO_PIN_2
#define S2_LED_GPIO_Port GPIOC
#define S1_LED_Pin GPIO_PIN_3
#define S1_LED_GPIO_Port GPIOC
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define _CAN_INT_Pin GPIO_PIN_4
#define _CAN_INT_GPIO_Port GPIOB
#define CAN_CS_Pin GPIO_PIN_12
#define CAN_CS_Port GPIOB
#define CAL_LED_Pin GPIO_PIN_9
#define CAL_LED_GPIO_Port GPIOD
#define CAN_STANDBY_Pin GPIO_PIN_11
#define CAN_STANDBY_GPIO_Port GPIOD
#define CAL_BTN_Pin GPIO_PIN_10
#define CAL_BTN_GPIO_Port GPIOD
#define _CAN_RESET_Pin GPIO_PIN_10
#define _CAN_RESET_GPIO_Port GPIOC

#if 0

// Note:  UNUSED EXTI INTERRUPT CHANNELS:  4, 8, 10, 14

#define COMM_OE2_Pin GPIO_PIN_2
#define COMM_OE2_GPIO_Port GPIOA

/* IOCX Interrupt Signal */
#define NAVX_2_RPI_INT4_Pin GPIO_PIN_10
#define NAVX_2_RPI_INT4_GPIO_Port GPIOA

/* AHRS Interrupt Signal */
#define NAVX_2_RPI_INT2_Pin GPIO_PIN_8
#define NAVX_2_RPI_INT2_GPIO_Port GPIOC

#define EEPROM_WP_Pin GPIO_PIN_11
#define EEPROM_WP_GPIO_Port GPIOC

#define _RPI_GPIO_OE1_Pin GPIO_PIN_3
#define _RPI_GPIO_OE1_GPIO_Port GPIOE

#define _RPI_GPIO_OE2_Pin GPIO_PIN_4
#define _RPI_GPIO_OE2_GPIO_Port GPIOE

#define EXT_PWR_SWITCH_ON_Pin GPIO_PIN_7
#define EXT_PWR_SWITCH_ON_GPIO_Port GPIOB

#define RPI_GPIO_DIR_IN_Pin GPIO_PIN_8
#define RPI_GPIO_DIR_IN_GPIO_Port GPIOB

/* CAN Interrupt Signal */
#define NAVX_2_RPI_INT3_Pin GPIO_PIN_1
#define NAVX_2_RPI_INT3_GPIO_Port GPIOD

#endif

#define CAN_OK_LED_Pin GPIO_PIN_15
#define CAN_OK_LED_GPIO_Port GPIOE

// CommDIO Pins (configurable as Inputs, Outputs, or for Digital Comms)
// NOTE:  The pins are semantically named from the perspective of the VMX-pi board, not the test jig!  This impacts the UART pins.
#define COMMDIO_I2C_SDA_Pin	GPIO_PIN_9
#define COMMDIO_I2C_SDA_GPIO_Port GPIOC
#define COMMDIO_I2C_SCL_Pin	GPIO_PIN_8
#define COMMDIO_I2C_SCL_GPIO_Port GPIOA
#define COMMDIO_UART_TX_Pin	GPIO_PIN_7
#define COMMDIO_UART_TX_GPIO_Port GPIOC
#define COMMDIO_UART_RX_Pin	GPIO_PIN_6
#define COMMDIO_UART_RX_GPIO_Port GPIOC
#define COMMDIO_SPI_SCK_Pin	GPIO_PIN_5
#define COMMDIO_SPI_SCK_GPIO_Port GPIOA
#define COMMDIO_SPI_MOSI_Pin	GPIO_PIN_7
#define COMMDIO_SPI_MOSI_GPIO_Port GPIOA
#define COMMDIO_SPI_MISO_Pin	GPIO_PIN_6
#define COMMDIO_SPI_MISO_GPIO_Port GPIOA
#define COMMDIO_SPI_CS_Pin	GPIO_PIN_4
#define COMMDIO_SPI_CS_GPIO_Port GPIOA

/* USER CODE END Private defines	 */

#define GPIO_INT(GPIO_INT_NUM) (1 << GPIO_INT_NUM)


/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
