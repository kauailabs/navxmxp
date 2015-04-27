/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file HardwareSerial.cpp
 * @brief Wirish serial port implementation.
 */

#include "HardwareSerial.h"
#include "stm32f4xx_hal.h"
#include <string.h>

HardwareSerial Serial6;

HardwareSerial::HardwareSerial() {
	this->rx_buffer_bytes_available = 0;
	this->rx_buffer_index = 0;
	for ( uint32_t i = 0; i < sizeof(this->rx_buffer); i++ ) {
		this->rx_buffer[i] = 0;
	}
	tx_in_progress = false;
	receive_request_pending = false;
}

/*
 * Set up/tear down
 */

void HardwareSerial::begin(uint32_t baud) {

	HAL_StatusTypeDef status;
	if ( huart6.Init.BaudRate != baud ) {
		HAL_UART_DeInit(&huart6);
		huart6.Init.BaudRate = baud;
		HAL_UART_Init(&huart6);
	}
	this->tx_in_progress = false;
	this->rx_buffer_bytes_available = 0;
	this->rx_buffer_index = 0;
	receive_request_pending = false;
	status = HAL_UART_Receive_IT(&huart6, &this->rx_buffer[this->rx_buffer_index], 1);

}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */

bool full_buffer = false;
bool failed_posted_read = false;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	HAL_StatusTypeDef status;
	Serial6.rx_buffer_index++;
	Serial6.rx_buffer_bytes_available++;
	if ( Serial6.rx_buffer_index >= UART_RX_BUFFER_SIZE) {
		Serial6.rx_buffer_index = 0;
	}

	if ( Serial6.rx_buffer_bytes_available < UART_RX_BUFFER_SIZE ) {
		status = HAL_UART_Receive_IT(UartHandle, &Serial6.rx_buffer[Serial6.rx_buffer_index], 1);
		if ( status == HAL_BUSY ) {
			/* This case can occur if busy transmitting and rx complete interrupt occurs. */
			/* Pend the receive request for later processing.                             */
			Serial6.receive_request_pending = true;
		}
	} else {
		full_buffer = true;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	HAL_StatusTypeDef status;
	Serial6.tx_in_progress = false;
	if ( Serial6.receive_request_pending ) {
		status = HAL_UART_Receive_IT(&huart6, &Serial6.rx_buffer[Serial6.rx_buffer_index], 1);
		if ( status == HAL_OK ) {
			Serial6.receive_request_pending = false;
		}
	}
}

int error_count = 0;

void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
	error_count++;
	/* Reset the UART.  The begin() method only takes effect if */
	/* the baud rate changes, so force a change here.           */
	uint32_t baud = huart6.Init.BaudRate;
	huart6.Init.BaudRate = 0;
	Serial6.begin(baud);
}


void HardwareSerial::end(void) {
	HAL_UART_DeInit(&huart6);
}

/*
 * I/O
 */

uint8_t HardwareSerial::read(void) {

	if ( rx_buffer_bytes_available < 1 ) return -1;

	HAL_NVIC_DisableIRQ(USART6_IRQn);
	int next_index = this->rx_buffer_index - this->rx_buffer_bytes_available;
	if ( next_index < 0 ) {
		next_index += UART_RX_BUFFER_SIZE;
	}
	this->rx_buffer_bytes_available--;

	/* If receive buffer was full, and has been emptied, begin receiving more data now. */
	HAL_StatusTypeDef status;
	if ( Serial6.rx_buffer_bytes_available == (UART_RX_BUFFER_SIZE-1) ) {
		status = HAL_UART_Receive_IT(&huart6, &Serial6.rx_buffer[Serial6.rx_buffer_index], 1);
		if ( HAL_OK == status ) {
			full_buffer = false;
		} else {
			failed_posted_read = false;
		}
	}
    HAL_NVIC_EnableIRQ(USART6_IRQn);

	return this->rx_buffer[next_index];
}

uint32_t HardwareSerial::read(uint8_t *buf, uint32_t len)
{
	uint32_t total_count = 0;
	if ( rx_buffer_bytes_available < 1 ) return -1;
    HAL_NVIC_DisableIRQ(USART6_IRQn);
    int avail_data_count = this->rx_buffer_bytes_available;
    int avail_data_index = this->rx_buffer_index - avail_data_count;
    uint8_t *avail_data_start;
	if ( avail_data_index < 0 ) {
		/* Wraparound case */
		avail_data_start = &rx_buffer[UART_RX_BUFFER_SIZE + avail_data_index];
		avail_data_count = UART_RX_BUFFER_SIZE + avail_data_index;
		memcpy(buf,avail_data_start,avail_data_count);
		total_count += avail_data_count;
		buf += avail_data_count;
		avail_data_index = 0;
		avail_data_count = this->rx_buffer_bytes_available - avail_data_count;
	}
	avail_data_start = &rx_buffer[avail_data_index];
	memcpy(buf,avail_data_start,avail_data_count);
	total_count += avail_data_count;

	this->rx_buffer_bytes_available-= total_count;
    HAL_NVIC_EnableIRQ(USART6_IRQn);
    return total_count;
}

uint8_t HardwareSerial::peek(void)
{
	if ( rx_buffer_bytes_available < 1 ) return -1;

	HAL_NVIC_DisableIRQ(USART6_IRQn);
	int next_index = this->rx_buffer_index - this->rx_buffer_bytes_available;
	if ( next_index < 0 ) {
		next_index += UART_RX_BUFFER_SIZE;
	}
	uint8_t new_byte = this->rx_buffer[next_index];
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    return new_byte;
}

uint32_t HardwareSerial::available(void) {
	uint32_t bytes_avail = rx_buffer_bytes_available;
	return bytes_avail;
}

uint32_t HardwareSerial::pending(void) {
	/*
    return usart_data_pending(usart_device);
    */
	/* Todo:  return number of bytes still to be transmitted. */
	return 0;
}

unsigned long serial_write_start_timestamp = 0;
#define SERIAL_WRITE_TIMEOUT_MS 100

void HardwareSerial::write(unsigned char ch) {
	tx_in_progress = true;
	if ( HAL_UART_Transmit_IT(&huart6, &ch, 1) != HAL_OK ) {
		tx_in_progress = false;
	} else {
		serial_write_start_timestamp = HAL_GetTick();
		while ( tx_in_progress ) {
			if ( (HAL_GetTick() - serial_write_start_timestamp) > (unsigned long)SERIAL_WRITE_TIMEOUT_MS) {
				tx_in_progress = false;
				HAL_UART_ErrorCallback(&huart6);
			}
		}
	}

}

void HardwareSerial::write(const void *buf, uint32_t len) {

	uint8_t *ch = (uint8_t*)buf;
    while (len--) {
        write(*ch++);
    }
	return;

	/*
	if ( tx_in_progress ) return;
	tx_in_progress = true;
	serial_write_start_timestamp = HAL_GetTick();
	if ( HAL_UART_Transmit_IT(&huart6, (uint8_t *)buf, len) != HAL_OK ) {
		tx_in_progress = false;
	} else {
		while ( tx_in_progress ) {
			HAL_Delay(1);
			if ( (HAL_GetTick() - serial_write_start_timestamp) > (unsigned long)SERIAL_WRITE_TIMEOUT_MS) {
				tx_in_progress = false;
			}
		}
	}*/
}


void HardwareSerial::flush(void) {
	/*
    usart_reset_rx(usart_device);
    */
}
