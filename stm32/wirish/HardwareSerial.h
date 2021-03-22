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
 * @file HardwareSerial.h
 * @brief Serial port interface.
 */

#ifndef _HARDWARESERIAL_H_
#define _HARDWARESERIAL_H_

#include <stdint.h>
#include "SerialReceiver.h"

extern "C" {
	#include "stm32f4xx_hal_dma.h"
	#include "stm32f4xx_hal_uart.h"
}

#define UART_RX_BUFFER_SIZE 512
#define UART_TX_BUFFER_SIZE 512

class HardwareSerial : public PrintSerialReceiver {
public:
    HardwareSerial();

    /* Set up/tear down */
    void begin(uint32_t baud);
    void end(void);

    /* I/O */
    uint8_t peek(void);
    uint32_t available(void);
    uint32_t pending(void);
    uint8_t read(void);
    uint32_t read(uint8_t *buf, uint32_t len);
    void flush(void);
    virtual void write(unsigned char);
    virtual void write(const void *buf, uint32_t len);
    virtual ~HardwareSerial(){}

    bool PossiblyStartNextTransmitUnsafe();
    void ResetUart();

    uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
    int  rx_buffer_index;
    int		rx_buffer_bytes_available;
    int  rx_buffer_next_readable_byte_index;
    bool tx_in_progress;
    bool receive_request_pending;
    uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
    int	tx_buffer_index;
    int tx_buffer_bytes_pending;
};

extern UART_HandleTypeDef huart6;

extern HardwareSerial Serial6;

#endif // _HARDWARESERIAL_H_
