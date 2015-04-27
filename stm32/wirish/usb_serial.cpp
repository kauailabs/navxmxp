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
 * @brief USB virtual serial terminal
 */

#include <string.h>

#include "usb_serial.h"
extern "C" {
#include "usbd_cdc_if.h"
}

#define USB_TIMEOUT 50

USBSerial::USBSerial(void) {
}

void USBSerial::begin(void) {
    //setupUSB();
}

void USBSerial::begin(int) {
    //setupUSB();
}

void USBSerial::end(void) {
    //disableUSB();
}

void USBSerial::write(uint8_t ch) {
    const uint8_t buf[] = {ch};
    this->write(buf, 1);
}

void USBSerial::write(const char *str) {
    this->write(str, strlen(str));
}

void USBSerial::write(const void *buf, uint32_t len) {
	/*
    if (!(usbIsConnected() && usbIsConfigured()) || !buf) {
        return;
    }

    uint32 txed = 0;
    uint32 old_txed = 0;
    uint32 start = millis();

    while (txed < len && (millis() - start < USB_TIMEOUT)) {
        txed += usbSendBytes((const uint8*)buf + txed, len - txed);
        if (old_txed != txed) {
            start = millis();
        }
        old_txed = txed;
    }
    */
	CDC_Transmit_FS((uint8_t*)buf,len);
}

uint32_t USBSerial::available(void) {
	return CDC_Receive_Available();
	return 0;
}

uint32_t USBSerial::read(uint8_t *buf, uint32_t len) {
    if (!buf) {
        return 0;
    }
    /*
    uint32_t rxed = 0;
    while (rxed < len) {
        rxed += usbReceiveBytes((uint8*)buf + rxed, len - rxed);
    }

    return rxed;
    */
    return 0;
}

/* Blocks forever until 1 byte is received */
uint8_t USBSerial::read(void) {
    return CDC_Receive_Read();
}

/* Returns the oldest received character, without removing it.*/
uint8_t USBSerial::peek(void) {
    return CDC_Receive_Peek();
}

uint32_t USBSerial::pending(void) {
    //return usbGetPending();
	return 0;
}

uint8_t USBSerial::isConnected(void) {
	return 1;
    //return usbIsConnected() && usbIsConfigured();
}

uint8_t USBSerial::getDTR(void) {
    //return usbGetDTR();
	return 0;
}

uint8_t USBSerial::getRTS(void) {
    //return usbGetRTS();
	return 0;
}

void USBSerial::enableBlockingTx(void) {
	//usbEnableBlockingTx();
}

void USBSerial::disableBlockingTx(void) {
	//usbEnableBlockingTx();
}

USBSerial SerialUSB;
