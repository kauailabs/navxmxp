/**
 ******************************************************************************
 * File Name          : stm32f4xx_hal_msp.c
 * Date               : 31/10/2014 11:34:34
 * Description        : This file provides code for the MSP Initialization
 *                      and de-Initialization codes.
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
extern DMA_HandleTypeDef hdma_i2c3_tx;

extern DMA_HandleTypeDef hdma_spi1_tx;

extern DMA_HandleTypeDef hdma_spi1_rx;

extern DMA_HandleTypeDef hdma_usart6_tx;

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Global Priority Scheme */
/*                        */
/* 0 - Systick            */
/* 1 - SPI                */
/* 2 - I2C                */
/* 3 - UART               */
/* 4 - USB                */
/* 5 - MPU                */
/* Lower priority numbers */
/* Should preempt higher  */

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* System interrupt init*/
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

#define GPIO_AF9_I2C2          ((uint8_t)0x09)  /* I2C2 Alternate Function mapping  */
#define GPIO_AF9_I2C3          ((uint8_t)0x09)  /* I2C2 Alternate Function mapping  */

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(hi2c->Instance==I2C2)
    {
        /* USER CODE BEGIN I2C2_MspInit 0 */

        /* USER CODE END I2C2_MspInit 0 */
        /* Peripheral clock enable */
        __I2C2_CLK_ENABLE();
        __I2C2_FORCE_RESET();
        __I2C2_RELEASE_RESET();

        /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB3     ------> I2C2_SDA 
         */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP; // GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        HAL_I2CEx_AnalogFilter_Config(hi2c,0); // Enable Analog Noise Filter
        HAL_I2CEx_DigitalFilter_Config(hi2c,12); // Enable Digital Noise Filter (Range 0 [none] - 15)
    }
    else if(hi2c->Instance==I2C3)
    {
        /* USER CODE BEGIN I2C3_MspInit 0 */

        /* USER CODE END I2C3_MspInit 0 */
        /* Peripheral clock enable */
        __I2C3_CLK_ENABLE();
        __I2C3_FORCE_RESET();
        __I2C3_RELEASE_RESET();

        /**I2C3 GPIO Configuration
    PA8     ------> I2C3_SCL
    PB4     ------> I2C3_SDA 
         */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; // GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_I2C3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_i2c3_tx.Instance = DMA1_Stream4;
        hdma_i2c3_tx.Init.Channel = DMA_CHANNEL_3;
        hdma_i2c3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_i2c3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c3_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c3_tx.Init.Mode = DMA_NORMAL;
        hdma_i2c3_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_i2c3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        hdma_i2c3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
        HAL_DMA_Init(&hdma_i2c3_tx);

        __HAL_LINKDMA(hi2c,hdmatx,hdma_i2c3_tx);

        /* Per Data sheet, in Fast Mode, the max digital filter value at 24Mhz Clock is 7 */
        HAL_I2CEx_AnalogFilter_Config(hi2c,0); // Enable Analog Noise Filter
        HAL_I2CEx_DigitalFilter_Config(hi2c,7); // Enable Digital Noise Filter (Range 0 [none] - 15)

        HAL_NVIC_SetPriority(I2C3_EV_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_SetPriority(I2C3_ER_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
        /* USER CODE BEGIN I2C3_MspInit 1 */

        /* USER CODE END I2C3_MspInit 1 */
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if(hi2c->Instance==I2C2)
    {
        /* USER CODE BEGIN I2C2_MspDeInit 0 */

        /* USER CODE END I2C2_MspDeInit 0 */
        /* Peripheral clock disable */
        __I2C2_CLK_DISABLE();

        /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB3     ------> I2C2_SDA 
         */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_3);

        /* USER CODE BEGIN I2C2_MspDeInit 1 */

        /* USER CODE END I2C2_MspDeInit 1 */
    }
    else if(hi2c->Instance==I2C3)
    {
        /* USER CODE BEGIN I2C3_MspDeInit 0 */

        /* USER CODE END I2C3_MspDeInit 0 */
        /* Peripheral clock disable */
        __I2C3_CLK_DISABLE();

        /**I2C3 GPIO Configuration
    PA8     ------> I2C3_SCL
    PB4     ------> I2C3_SDA 
         */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);

        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(hi2c->hdmatx);

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
        /* USER CODE BEGIN I2C3_MspDeInit 1 */

        /* USER CODE END I2C3_MspDeInit 1 */
    }
}

uint32_t successful_reset_count = 0;
uint32_t failed_reset_count = 0;

void HAL_I2C_Reset(I2C_HandleTypeDef* hi2c)
{
    int retry_count = 5;
    int i;
    int scl_busy = 0;
    int sda_busy = 0;
    GPIO_InitTypeDef GPIO_InitStruct;
    while ( retry_count > 0 ) {
        int is_busy = (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) == SET) ? 1 : 0;

        /**I2C2 GPIO Configuration
		PB10     ------> I2C2_SCL
		PB3     ------> I2C2_SDA
         */

        if ( is_busy ) {

            /* Reconfigure GPIO for direct IO */
            HAL_I2C_DeInit(hi2c);

            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
            GPIO_InitStruct.Pin = GPIO_PIN_10;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);

            /* Try to send out a pseudo-stop bit.  */
            if ( ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET ) &&
                 ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET ) ) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
                HAL_Delay(1);
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
                HAL_Delay(1);
            } else {
                /* One more clock in case device was trying to ack address */
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
                HAL_Delay(1);
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
                HAL_Delay(1);
                if ( ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET ) &&
                     ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET ) ) {
                    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
                    HAL_Delay(1);
                    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
                    HAL_Delay(1);
                }
            }
#if 0
            /* Detect if SDA is high, and if so toggle clock pin */
            int max_retry_count = 1000;
            while ( max_retry_count > 0 && (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET) ) {
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_10);
                HAL_Delay(1);
                max_retry_count--;
            }
#endif
            /* Algorithm taken from Analog Device AN-686, page 2*/
            /* Up to 9 data bits may be pending transmission by the I2C slave */
            for ( i = 0; i < 9; i++ ) {
                    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
                    HAL_Delay(1);
                    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_10);
                    HAL_Delay(1);
                }
            }

            if ( ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET ) &&
                 ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET ) ) {
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
                HAL_Delay(1);
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
                HAL_Delay(1);
            }

            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);


        /* Set SDA Pin as INPUT */
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        sda_busy = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET);

        /* Set SCL Pin as INPUT */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        scl_busy = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_RESET);

        /* Force a software reset */
        hi2c->Instance->CR1 &= 0x8000;
        /* Clear all registers */
        hi2c->Instance->CR2 = 0;
        hi2c->Instance->CCR = 0;
        hi2c->Instance->TRISE = 0;
        hi2c->Instance->SR1 = 0;
        hi2c->Instance->SR2 = 0;
        hi2c->Instance->OAR1 = 0;
        hi2c->Instance->OAR2 = 0;
        HAL_Delay(1);
        /* Release software reset */
        hi2c->Instance->CR1 &= ~0x8000;

        HAL_I2C_Init(hi2c);

        if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != SET) {
            successful_reset_count++;
            break;
        }
        retry_count--;
    }
    if ( retry_count == 0) {
        failed_reset_count++;
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(hspi->Instance==SPI1)
    {
        /* USER CODE BEGIN SPI1_MspInit 0 */

        /* USER CODE END SPI1_MspInit 0 */
        /* Peripheral clock enable */
        __SPI1_CLK_ENABLE();

        /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    PA15     ------> SPI1_NSS 
         */
        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_spi1_tx.Instance = DMA2_Stream2;
        hdma_spi1_tx.Init.Channel = DMA_CHANNEL_2;
        hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi1_tx.Init.Mode = DMA_NORMAL;
        hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        hdma_spi1_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
        hdma_spi1_tx.Init.MemBurst = DMA_MBURST_SINGLE;
        hdma_spi1_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
        HAL_DMA_Init(&hdma_spi1_tx);

        __HAL_LINKDMA(hspi,hdmatx,hdma_spi1_tx);

        hdma_spi1_rx.Instance = DMA2_Stream0;
        hdma_spi1_rx.Init.Channel = DMA_CHANNEL_3;
        hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_spi1_rx.Init.Mode = DMA_NORMAL;
        hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_spi1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_spi1_rx);

        __HAL_LINKDMA(hspi,hdmarx,hdma_spi1_rx);

        HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(SPI1_IRQn);
        /* USER CODE BEGIN SPI1_MspInit 1 */

        /* USER CODE END SPI1_MspInit 1 */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance==SPI1)
    {
        /* USER CODE BEGIN SPI1_MspDeInit 0 */

        /* USER CODE END SPI1_MspDeInit 0 */
        /* Peripheral clock disable */
        __SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    PA15     ------> SPI1_NSS 
         */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15);

        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(hspi->hdmatx);
        HAL_DMA_DeInit(hspi->hdmarx);

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(SPI1_IRQn);
        /* USER CODE BEGIN SPI1_MspDeInit 1 */

        /* USER CODE END SPI1_MspDeInit 1 */
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(huart->Instance==USART6)
    {
        /* USER CODE BEGIN USART6_MspInit 0 */

        /* USER CODE END USART6_MspInit 0 */
        /* Peripheral clock enable */
        __USART6_CLK_ENABLE();

        /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
         */
        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_usart6_tx.Instance = DMA2_Stream6;
        hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
        hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart6_tx.Init.Mode = DMA_NORMAL;
        hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_usart6_tx);

        __HAL_LINKDMA(huart,hdmatx,hdma_usart6_tx);

        HAL_NVIC_SetPriority(USART6_IRQn, 3,0); /* Raised Priority to group 2 */
        HAL_NVIC_EnableIRQ(USART6_IRQn);
        /* USER CODE BEGIN USART6_MspInit 1 */

        /* USER CODE END USART6_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if(huart->Instance==USART6)
    {
        /* USER CODE BEGIN USART6_MspDeInit 0 */

        /* USER CODE END USART6_MspDeInit 0 */
        /* Peripheral clock disable */
        __USART6_CLK_DISABLE();

        /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
         */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(huart->hdmatx);

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(USART6_IRQn);
        /* USER CODE BEGIN USART6_MspDeInit 1 */

        /* USER CODE END USART6_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
