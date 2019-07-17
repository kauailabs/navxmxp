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

#define NAVX_2_RPI_SPI_Comm_Ready_Pin GPIO_PIN_2
#define NAVX_2_RPI_SPI_Comm_Ready_GPIO_Port GPIOE
#define S2_LED_Pin GPIO_PIN_2
#define S2_LED_GPIO_Port GPIOC
#define S1_LED_Pin GPIO_PIN_3
#define S1_LED_GPIO_Port GPIOC
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define _CAN_INT_Pin GPIO_PIN_4
#define _CAN_INT_GPIO_Port GPIOB
#define _I2C_DEV_ON_Pin GPIO_PIN_12
#define _I2C_DEV_ON_GPIO_Port GPIOE
#define CAN_CS_Pin GPIO_PIN_12
#define CAN_CS_Port GPIOB
#define MPU9250_INT_Pin GPIO_PIN_8
#define MPU9250_INT_GPIO_Port GPIOD
#define CAL_LED_Pin GPIO_PIN_9
#define CAL_LED_GPIO_Port GPIOD
#define CAN_STANDBY_Pin GPIO_PIN_11
#define CAN_STANDBY_GPIO_Port GPIOD
#define CAL_BTN_Pin GPIO_PIN_10
#define CAL_BTN_GPIO_Port GPIOD
#define _CAN_RESET_Pin GPIO_PIN_10
#define _CAN_RESET_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
#define QE1_A_Pin GPIO_PIN_9  //PE9,  TIM1 CH1
#define QE1_A_GPIO_Port GPIOE
#define QE1_B_Pin GPIO_PIN_11 //PE11, TIM1 CH2
#define QE1_B_GPIO_Port GPIOE
#define QE2_A_Pin GPIO_PIN_15 //PA15, TIM2 CH1
#define QE2_A_GPIO_Port GPIOA
#define QE2_B_Pin GPIO_PIN_3  //PB3,  TIM2 CH2
#define QE2_B_GPIO_Port GPIOB
#define QE3_A_Pin GPIO_PIN_6  //PC6,  TIM3 CH1
#define QE3_A_GPIO_Port GPIOC
#define QE3_A_Second_Pin GPIO_PIN_2   /* To avoid EXTI conflict w/PWM_GPIO4 */
#define QE3_A_Second_GPIO_Port GPIOD  /* To avoid EXTI conflict w/PWM_GPIO4 */
#define QE3_B_Pin GPIO_PIN_7  //PC7,  TIM3 CH2
#define QE3_B_GPIO_Port GPIOC
#define QE4_A_Pin GPIO_PIN_12 //PD12, TIM4 CH1
#define QE4_A_GPIO_Port GPIOD
#define QE4_B_Pin GPIO_PIN_13 //PD13, TIM4 CH2
#define QE4_B_GPIO_Port GPIOD
#define QE5_A_Pin   GPIO_PIN_0 //PA0, TIM5 CH1
#define QE5_A_GPIO_Port GPIOA
#define QE5_B_Pin   GPIO_PIN_1 //PA1, TIM5 CH2
#define QE5_B_GPIO_Port GPIOA
#define PWM_GPIO3_Pin GPIO_PIN_5 //PE5, TIM9 CH1
#define PWM_GPIO3_GPIO_Port GPIOE
#define PWM_GPIO4_Pin GPIO_PIN_6 //PE6, TIM9 CH2
#define PWM_GPIO4_GPIO_Port GPIOE

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

#define _IO_POWER_FAULT_Pin GPIO_PIN_4
#define _IO_POWER_FAULT_GPIO_Port GPIOD

#define _CAN_RX0BF_Pin GPIO_PIN_14
#define _CAN_RX0BF_GPIO_Port GPIOD

#define _CAN_RX1BF_Pin GPIO_PIN_15
#define _CAN_RX1BF_GPIO_Port GPIOD

#define CAN_OK_LED_Pin GPIO_PIN_15
#define CAN_OK_LED_GPIO_Port GPIOE

/* VMX-pi Board Rev 5.38 added:
 *
 * PD 7, 6 and 5:  3 bits of board Revision, as follows:
 * - 0x111:   Boards previous to v. 5.38 (e.g., 5.35 [Nov. 2017]
 * - 0x110:   Revision 5.38 [Nov, 2018]
 *
 * NOTE:  PD7 is the most significant bit.
 *
 * NOTE:  These signals were disconnected in previous designs.  They can be
 * tested on all designs by pulling them high at startup and reading their state.
 */

#define VMXPI_BOARDREV_BIT2_Pin GPIO_PIN_7
#define VMXPI_BOARDREV_BIT1_Pin GPIO_PIN_6
#define VMXPI_BOARDREV_BIT0_Pin GPIO_PIN_5
#define VMXPI_BOARDREV_Port GPIOD

#define VMXPI_BOARDREV_MAX  0x07
#define VMXPI_BOARDREV_5_35	0x07  /* NOTE:  All pins disconnected on this revision. */
#define VMXPI_BOARDREV_5_38 0x06  /* BIT2, BIT 1 Disconnected.  BIT 2 Pulled to Ground. */
								  /* NOTE:  Bit 2 is not actually pulled to Ground.  So */
								  /*        there is no known reliable method to detect */
								  /*        this board version.  Fortunately, control of*/
								  /*        the 5.38 functionality is backwards-        */
								  /*        compatible.                                 */

/* VMX-pi Board Rev 5.38 added:
 * Output Enable Pin for output-direction CommDIO Signals:
 * - UART: TX
 * - SPI:  CLK CS MOSI
 * This signal is active low.
 * This signal was unconnected in previous designs.
 */

#define _COMM_OE1_Pin GPIO_PIN_3
#define _COMM_OE1_GPIO_Port GPIOA

/* USER CODE END Private defines	 */

#define GPIO_INT(GPIO_INT_NUM) (1 << GPIO_INT_NUM)


/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
