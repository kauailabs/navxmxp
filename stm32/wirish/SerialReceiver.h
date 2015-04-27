#ifndef _SERIAL_RECEIVER_H_
#define _SERIAL_RECEIVER_H_

#include <stdint.h>
#include "Print.h"

class SerialReceiver {
public:
	virtual uint32_t	available(void) = 0;
    virtual uint32_t 	read(uint8_t *buf, uint32_t len) = 0;
    virtual uint8_t 	read(void) = 0;
	virtual uint8_t 	peek(void) = 0;
	virtual uint32_t 	pending(void) = 0;
	virtual ~SerialReceiver(){}
};

class PrintSerialReceiver : public Print, public SerialReceiver {
public:
	virtual ~PrintSerialReceiver(){}
};

#endif
