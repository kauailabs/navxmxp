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

#define BUFFERED_WRITE /* Define this symbol to enable buffer writing */

HardwareSerial::HardwareSerial() {
    this->rx_buffer_bytes_available = 0;
    this->rx_buffer_index = 0;
    this->rx_buffer_next_readable_byte_index = 0;
    for ( uint32_t i = 0; i < sizeof(this->rx_buffer); i++ ) {
        this->rx_buffer[i] = 0;
    }
    this->tx_buffer_index = 0;
    this->tx_buffer_bytes_pending = 0;
    for ( uint32_t i = 0; i < sizeof(this->tx_buffer); i++ ) {
        this->tx_buffer[i] = 0;
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
    this->rx_buffer_next_readable_byte_index = 0;
    this->tx_buffer_bytes_pending = 0;
    this->tx_buffer_index = 0;
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
    Serial6.rx_buffer_next_readable_byte_index = Serial6.rx_buffer_index - Serial6.rx_buffer_bytes_available;
    if ( Serial6.rx_buffer_next_readable_byte_index < 0 ) {
        Serial6.rx_buffer_next_readable_byte_index += UART_RX_BUFFER_SIZE;
    }
    if ( Serial6.rx_buffer_index >= UART_RX_BUFFER_SIZE) {
        Serial6.rx_buffer_index = 0;
    }

    /* If another byte is already available, read it out now */
    /* and place in buffer, rather than wait for interrupt.  */
    if ( Serial6.rx_buffer_bytes_available < UART_RX_BUFFER_SIZE ) {
        uint32_t uart_status_reg = UartHandle->Instance->SR;
        if ( ( uart_status_reg & 0x20 ) != 0 ) {
            uint32_t rx_data_reg = UartHandle->Instance->DR;
            uint8_t rx_data = (uint8_t)rx_data_reg & 0x000000FF;
            Serial6.rx_buffer[Serial6.rx_buffer_index] = rx_data;
            Serial6.rx_buffer_index++;
            Serial6.rx_buffer_bytes_available++;
        }
    }

    if ( Serial6.rx_buffer_bytes_available < UART_RX_BUFFER_SIZE ) {
        status = HAL_UART_Receive_IT(UartHandle, &Serial6.rx_buffer[Serial6.rx_buffer_index], 1);
        if ( status != HAL_OK ) {
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

#ifdef BUFFERED_WRITE
    uint16_t num_bytes_transmitted = UartHandle->TxXferSize - UartHandle->TxXferCount;
    Serial6.tx_buffer_bytes_pending -= num_bytes_transmitted;
    Serial6.PossiblyStartNextTransmitUnsafe();
#endif

}

uint32_t error_count = 0;
uint32_t overrun_error_count = 0;
uint32_t parity_error_count = 0;
uint32_t noise_error_count = 0;
uint32_t framing_error_count = 0;
uint32_t dma_transfer_error_count = 0;

static void ResetUart()
{
    /* Reset the UART.  The begin() method only takes effect if */
    /* the baud rate changes, so force a change here.           */
    uint32_t baud = huart6.Init.BaudRate;
    huart6.Init.BaudRate = 0;
    Serial6.begin(baud);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{

    HAL_StatusTypeDef status;
    error_count++;
    if ( UartHandle->ErrorCode & HAL_UART_ERROR_PE ) {
        parity_error_count++;
    }
    if ( UartHandle->ErrorCode & HAL_UART_ERROR_NE ) {
        noise_error_count++;
    }
    if ( UartHandle->ErrorCode & HAL_UART_ERROR_FE ) {
        framing_error_count++;
    }
    if ( UartHandle->ErrorCode & HAL_UART_ERROR_ORE ) {
        overrun_error_count++;
    }
    if ( UartHandle->ErrorCode & HAL_UART_ERROR_DMA ) {
        dma_transfer_error_count++;
    }

    // Since whenever the error callback is invoked, the
    // uart state is set to ready, this means any transfers
    // in progress were aborted.  The packet that was
    // being transferred was likely not completely sent.
    // In any case, clear the tx_in_progress flag.

    Serial6.tx_in_progress = false;

    // Additonally, unless the receive buffer was already full,
    // a new receive must be posted each time an error occurs.

    if ( Serial6.rx_buffer_bytes_available < UART_RX_BUFFER_SIZE ) {
        status = HAL_UART_Receive_IT(UartHandle, &Serial6.rx_buffer[Serial6.rx_buffer_index], 1);
        if ( status != HAL_OK ) {
            /* This case can occur if busy transmitting and rx complete interrupt occurs. */
            /* Pend the receive request for later processing.                             */
            Serial6.receive_request_pending = true;
        }
    }
    //ResetUart();
}


void HardwareSerial::end(void) {
    HAL_UART_DeInit(&huart6);
}

/*
 * I/O
 */

uint8_t HardwareSerial::read(void) {

    if ( rx_buffer_bytes_available < 1 ) return -1;

    NVIC_DisableIRQ(USART6_IRQn);
    int next_index = this->rx_buffer_index - this->rx_buffer_bytes_available;
    if ( next_index < 0 ) {
        next_index += UART_RX_BUFFER_SIZE;
    }
    this->rx_buffer_bytes_available--;
    int curr_buffer_index = Serial6.rx_buffer_index;
    Serial6.rx_buffer_next_readable_byte_index++;
    if ( Serial6.rx_buffer_next_readable_byte_index >= sizeof(Serial6.rx_buffer)) {
        Serial6.rx_buffer_next_readable_byte_index = 0;
    }
    NVIC_EnableIRQ(USART6_IRQn);

    /* If receive buffer was full, and has been emptied, begin receiving more data now. */
    HAL_StatusTypeDef status;
    if ( Serial6.rx_buffer_bytes_available == (UART_RX_BUFFER_SIZE-1) ) {
        status = HAL_UART_Receive_IT(&huart6, &Serial6.rx_buffer[curr_buffer_index], 1);
        if ( HAL_OK == status ) {
            full_buffer = false;
        } else {
            receive_request_pending = true;
            failed_posted_read = false;
        }
    }

    return this->rx_buffer[next_index];
}

uint32_t HardwareSerial::read(uint8_t *buf, uint32_t len)
{
    uint32_t total_count = 0;
    if ( rx_buffer_bytes_available < 1 ) return -1;
    NVIC_DisableIRQ(USART6_IRQn);
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
    NVIC_EnableIRQ(USART6_IRQn);
    return total_count;
}

uint8_t HardwareSerial::peek(void)
{
    if ( rx_buffer_bytes_available < 1 ) return -1;

    return this->rx_buffer[rx_buffer_next_readable_byte_index];
#if 0
    /* This is legacy code that requires interrupts to be masked during peek. */
    NVIC_DisableIRQ(USART6_IRQn);
    int next_index = this->rx_buffer_index - this->rx_buffer_bytes_available;
    NVIC_EnableIRQ(USART6_IRQn);
    if ( next_index < 0 ) {
        next_index += UART_RX_BUFFER_SIZE;
    }
    uint8_t new_byte = this->rx_buffer[next_index];

    return new_byte;
#endif
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
#ifdef BUFFERED_WRITE
    int err = 0;
    NVIC_DisableIRQ(USART6_IRQn);
    int buffer_bytes_free = sizeof(this->tx_buffer) - this->tx_buffer_bytes_pending;
    if ( len > (uint32_t)buffer_bytes_free ) {
        /* This is an error, transmit buffer overflow. */
        err = 1;
        ResetUart();
    } else {
        int bytes_til_end_of_buffer = sizeof(this->tx_buffer) - this->tx_buffer_index;
        if ( bytes_til_end_of_buffer > (int)len ) {
            memcpy(&this->tx_buffer[this->tx_buffer_index],buf,len);
            this->tx_buffer_bytes_pending += len;
            this->tx_buffer_index += len;
        } else {
            memcpy(&this->tx_buffer[this->tx_buffer_index],buf,bytes_til_end_of_buffer);
            int bytes_remaining = len - bytes_til_end_of_buffer;
            memcpy(&this->tx_buffer[0], (uint8_t *)buf + bytes_til_end_of_buffer, bytes_remaining);
            this->tx_buffer_bytes_pending += len;
            this->tx_buffer_index = bytes_remaining;
        }
        PossiblyStartNextTransmitUnsafe();
    }
    NVIC_EnableIRQ(USART6_IRQn);
#else
    /* Blocking Write */
    if ( tx_in_progress ) return;
    tx_in_progress = true;
    serial_write_start_timestamp = HAL_GetTick();
    if ( HAL_UART_Transmit_IT(&huart6, (uint8_t *)buf, len) != HAL_OK ) {
        tx_in_progress = false;
    } else {
        while ( tx_in_progress ) {
            //HAL_Delay(1);
            if ( (HAL_GetTick() - serial_write_start_timestamp) > (unsigned long)SERIAL_WRITE_TIMEOUT_MS) {
                tx_in_progress = false;
            }
        }
    }
#endif

}

bool HardwareSerial::PossiblyStartNextTransmitUnsafe()
{
    bool ok = true;

    /* If a transmit is not already in progress, then start one now */
    if ( !tx_in_progress ) {
        // start transfer from next_buffer_index til end of packet or end of buffer, which ever comes first.
        if ( this->tx_buffer_bytes_pending > 0 ) {
            int start_index = this->tx_buffer_index - this->tx_buffer_bytes_pending;
            uint8_t *start_address;
            if ( start_index >= 0 ) {
                start_address = &this->tx_buffer[start_index];
            } else {
                start_address = &this->tx_buffer[(sizeof(this->tx_buffer) + start_index)];
                start_index += sizeof(this->tx_buffer);
            }
            int xfer_len = this->tx_buffer_bytes_pending;
            if ( ( start_index + this->tx_buffer_bytes_pending ) > sizeof(this->tx_buffer) ) {
                xfer_len = sizeof(this->tx_buffer) - start_index;
            }
            if ( HAL_UART_Transmit_DMA(&huart6, &this->tx_buffer[start_index], xfer_len) != HAL_OK ) {
                tx_in_progress = false;
                ok = false;
            } else {
                tx_in_progress = true;
            }
        }
    }
    return ok;
}


void HardwareSerial::flush(void) {
    /*
    usart_reset_rx(usart_device);
     */
}
