/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 31/10/2014 11:34:34
 * Description        : Main program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2014 STMicroelectronics
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
#include "stm32f4xx_hal.h"
#include "usb_device.h"
/* USER CODE BEGIN Includes */
#include "navx-mxp.h"
#include "iocx.h"
#include "navx-mxp_hal.h"
#ifdef ENABLE_IOCX
#include "gpio_navx-pi.h"
#include "adc_navx-pi.h"
#endif
#ifdef ENABLE_CAN_TRANSCEIVER
#include "CAN.h"
#endif
#ifdef ENABLE_MISC
#include "MISC.h"
#endif
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c3_tx;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_tx;

#ifdef ENABLE_RTC
RTC_HandleTypeDef hrtc;
#endif

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static int MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* NavX-PI has these fundamental differences from previous navX-Family devices:
 *
 * I2C:
 * I2C2 is still used to communicate w/on-board I2C devices.
 * I2C3 is not used on the navX-PI.
 *
 * SPI:
 * SPI1 is still used as a slave interface.  In order to communicate with the
 * Raspberry PI, the mode must be changed to 0 or 2.  (Per online docs in the
 * pigpio library, modes 1 and 3 don't work on the RPI Aux spi interface).
 * SPI2 is new - and is used to communicate to the MCP25625 CAN Interface.
 *
 * UART:
 * The UART is NOT used on the navX-PI.
 *
 * TIMERS:
 * Timers 1 through 5 are used on the navX-PI.
 *
 * ADC:
 * ADC1 (Inputs 8, 9, 14 & 15) are used on the navX-PI.
 */

/* USER CODE BEGIN 0 */

void USB_Soft_Disconnect()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    /* In case of soft reset, ensure the USB host    */
    /* sees the D+ USB signal lines low.  Note that  */
    /* the onboard EMI Filter has a 1.5K ohm pullup, */
    /* therefore this causes 2.2mA of current to be  */
    /* sunk into this pin.                           */

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	//HAL_GPIO_WritePin( GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

    /* Wait a bit for the usb host to discover the reset */
    HAL_Delay(2000);
	//HAL_GPIO_WritePin( GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	//HAL_Delay(3);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE END 0 */

int main(void)
{

    /* USER CODE BEGIN 1 */
    int spi_slave_enabled = 0;
    int uart_slave_enabled = 0;

    /* USER CODE END 1 */

    /* MCU Configuration----------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    int board_rev = MX_GPIO_Init();

    USB_Soft_Disconnect();

    MX_DMA_Init();
    MX_I2C2_Init();

    HAL_DIP_Switches_Init();
    spi_slave_enabled = HAL_SPI_Slave_Enabled();
    uart_slave_enabled = HAL_UART_Slave_Enabled();

    MX_I2C3_Init();

    if (spi_slave_enabled) {
        MX_SPI1_Init();
    }

    if (uart_slave_enabled) {
        MX_USART6_UART_Init();
    }

#ifdef ENABLE_CAN_TRANSCEIVER
    MX_SPI2_Init();
#endif

#if defined(ENABLE_ADC)
    MX_ADC1_Init();
#endif

    MX_USB_DEVICE_Init();
#if defined(ENABLE_RTC)
    MX_RTC_Init();
#endif
    /* USER CODE BEGIN 2 */
#ifdef ENABLE_IOCX
    HAL_IOCX_Init(board_rev);  // Ensure board_rev is registered BEFORE nav10_init() is invoked.
#endif
    nav10_init();
#ifdef ENABLE_IOCX

#define IOCX_BANK_NUMBER 1
#define IOCX_EX_BANK_NUMBER 4
    /* The IOCX handlers process both the IOCX and the IOCX_EX Banks */

#ifdef ENABLE_HIGHRESOLUTION_TIMESTAMP
    HAL_IOCX_HIGHRESTIMER_Init();
#endif
    IOCX_init();
    nav10_set_loop(IOCX_BANK_NUMBER, IOCX_loop);
    nav10_set_register_lookup_func(IOCX_BANK_NUMBER, IOCX_get_reg_addr_and_max_size);
    nav10_set_register_write_func(IOCX_BANK_NUMBER, IOCX_banked_writable_reg_update_func);
    nav10_set_loop(IOCX_EX_BANK_NUMBER, IOCX_loop);
    nav10_set_register_lookup_func(IOCX_EX_BANK_NUMBER, IOCX_get_reg_addr_and_max_size);
    nav10_set_register_write_func(IOCX_EX_BANK_NUMBER, IOCX_banked_writable_reg_update_func);
#endif // ENABLE_IOCX
#ifdef ENABLE_CAN_TRANSCEIVER
#define CAN_BANK_NUMBER 2
    CAN_init();
    nav10_set_loop(CAN_BANK_NUMBER, CAN_loop);
    nav10_set_register_lookup_func(CAN_BANK_NUMBER, CAN_get_reg_addr_and_max_size);
    nav10_set_register_write_func(CAN_BANK_NUMBER, CAN_banked_writable_reg_update_func);
#endif // ENABLE_CAN_TRANSCEIVER
#ifdef ENABLE_MISC
#define MISC_BANK_NUMBER 3
    MISC_init();
    nav10_set_loop(MISC_BANK_NUMBER, MISC_loop);
    nav10_set_register_lookup_func(MISC_BANK_NUMBER, MISC_get_reg_addr_and_max_size);
    nav10_set_register_write_func(MISC_BANK_NUMBER, MISC_banked_writable_reg_update_func);
#endif // ENABLE_MISC
    /* USER CODE END 2 */

    nav10_main();

    return 0; /* Note:  Control will never reach this point. */
}

/** System Clock Configuration
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    __PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
#ifdef ENABLE_LSE
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
#endif // ENABLE_LSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 384;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4; // 24Mhz.  Was RCC_HCLK_DIV2 (48Mhz);
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3 );
#ifdef ENABLE_LSE
#ifdef ENABLE_RTC
    /* Feed Real-time Clock via Low-Speed External (LSE) */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
#endif // ENABLE_RTC
#endif // ENABLE_LSE
}

/* I2C2 init function */
void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 200000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    HAL_I2C_Init(&hi2c2);
}

/* I2C3 init function */
void MX_I2C3_Init(void)
{
#ifndef DISABLE_EXTERNAL_I2C_INTERFACE
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = 400000;
    hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c3.Init.OwnAddress1 = 0x32 << 1;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    HAL_I2C_Init(&hi2c3);
#endif
}

/* SPI1 init function */
/* Note:  Default is SPI Transfer Mode 3 (Clk Polarity = 1, Clk Phase = 1)
 * However in the case of navX-PI, since the Raspberry PI Aux SPI interface
 * only supports SPI Modes 0 and 2, SPI Transfer Mode 0 is used
 * (Clk Polarity = 0, Clk Phase = 0)
 */
void MX_SPI1_Init(void)
{
#ifndef	DISABLE_EXTERNAL_SPI_INTERFACE
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_SLAVE;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
#ifdef EXTERNAL_SPI_INTERFACE_MODE_0
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
#else
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
#endif
    hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    HAL_SPI_Init(&hspi1);
#endif
}

/* SPI2 init function */
void MX_SPI2_Init(void)
{
#ifdef ENABLE_CAN_TRANSCEIVER
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  /* Configure for MODE 0 (which is used by the MCP25625) */
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT; /* NOTE:  Software SPI Chip select used (HW CS doesn't work w/MCP256525) */
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* APB1(24Mhz)/4 = 6Mhz.  MCP25625:  Max 10Mhz */
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  HAL_SPI_Init(&hspi2);
#endif
}

/* USART6 init function */
void MX_USART6_UART_Init(void)
{
#ifndef DISABLE_EXTERNAL_UART_INTERFACE
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 57600;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart6);
#endif
}
/** Configure pins as 
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 * Free pins are configured automatically as Analog (this feature is enabled through
 * the Code Generation settings)
 */
int MX_GPIO_Init(void)
{
	int board_rev = 7; // Board revisions are 3-bit values (0-7).  Default is 7.

#ifdef GPIO_MAP_NAVX_PI
    board_rev = MX_GPIO_Init_NavX_PI();
#else
	GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */

    __GPIOH_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    /*Configure GPIO pins : PC13 PC14 PC15 PC0
     PC1 PC2 PC4 PC10
     PC11 PC12 */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0
            | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_10 | GPIO_PIN_11
            | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : PC3 PC5 */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : PA0 PA1 PA2 PA3
     PA4 PA9 PA10 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
            | GPIO_PIN_4 | GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : PB0 PB1 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PB2 PB12 PB13 PB5
     PB6 PB7 PB8 PB9 */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_5
            | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PB14 PB15 */
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PC9 */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PD2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#endif
    return board_rev;
}

/**
 * Enable DMA controller clock
 */
void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __DMA1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();

    /* DMA interrupt init */
#ifndef DISABLE_EXTERNAL_SPI_INTERFACE
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0); /* SPI */
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 1, 0); /* SPI */
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
#endif
#ifndef DISABLE_EXTERNAL_I2C_INTERFACE
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 2, 0); /* I2C */
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
#endif
#ifndef DISABLE_EXTERNAL_UART_INTERFACE
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 3, 0); /* UART */
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
#endif
#ifdef ENABLE_ADC
    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 6, 0); /* ADC */
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
#endif
}

/* RTC init function */
void MX_RTC_Init(void)
{
#ifdef ENABLE_RTC

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	/**Initialize RTC and set the Time and Date
	*/
	hrtc.Instance = RTC;

	int calendar_initialized = ((hrtc.Instance->ISR & RTC_FLAG_INITS) != 0);

	/* Check if RTC is already initialized (via INITS flag)        */
	/* The RTC "backup domain" is powered by a dedicated battery,  */
	/* and the calendar should only be initialized if it has not   */
	/* yet been programmed.  The calendar is cleared if there is   */
	/* ever a "backup domain" reset.                               */

	if (!calendar_initialized) {

		hrtc.Init.HourFormat = RTC_HOURFORMAT_24;

		/* The LSE is a 32.768 Khz clock, and the Real Time Clock's   */
		/* desired subsecond resolution is sub 1ms.  Therefore, at    */
		/* the cost of some extra current consumption, the predivisor */
		/* is configured to yield 2 ticks per millisecond, using this */
		/* formula, documented in the STM32 datasheet:                */
		/* RTCCLK / ((AsynchPrediv + 1) * (SynchPrediv + 1))          */
		/* RTCCLK=32768, AsynchPrediv=15, SynchPrediv=2047            */
		/* Result:  1Hz total (32768/16*2048)                         */
		/*          Subsecond resolution:  1/2048                     */

		hrtc.Init.AsynchPrediv = 15;
		hrtc.Init.SynchPrediv = 2047;
		hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
		hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
		hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
		HAL_RTC_Init(&hrtc);

		sTime.Hours = 0x0;
		sTime.Minutes = 0x0;
		sTime.Seconds = 0x0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;
		HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);

		sDate.WeekDay = RTC_WEEKDAY_MONDAY;
		sDate.Month = RTC_MONTH_JANUARY;
		sDate.Date = 0x1;
		sDate.Year = 0x1;

		HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN);
	}
	HAL_RTC_InitializeCache();
#endif
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */

}

#endif
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
