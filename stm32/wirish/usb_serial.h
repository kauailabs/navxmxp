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
 * @brief Wirish virtual serial port
 */

#ifndef _USB_SERIAL_H_
#define _USB_SERIAL_H_

#include "SerialReceiver.h"

/**
 * @brief Virtual serial terminal.
 */
class USBSerial : public PrintSerialReceiver {
public:
    USBSerial(void);

    void begin(void);
    void begin(int);
    void end(void);

    uint32_t available(void);

    uint32_t read(uint8_t *buf, uint32_t len);
    uint8_t  read(void);
    uint8_t  peek(void);

    void write(uint8_t);
    void write(const char *str);
    void write(const void*, uint32_t);

    uint8_t getRTS();
    uint8_t getDTR();
    uint8_t isConnected();
    uint32_t pending();

    void enableBlockingTx(void);
    void disableBlockingTx(void);
    ~USBSerial(){}
};

extern USBSerial SerialUSB;

#endif

