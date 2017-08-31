/*
 * IPIGPIOInterruptSinks.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef IPIGPIOINTERRUPTSINKS_H_
#define IPIGPIOINTERRUPTSINKS_H_

#include <pigpio.h>

typedef enum {
	PIGPIO_INT_RISING_EDGE = RISING_EDGE,
	PIGPIO_INT_FALLING_EDGE = FALLING_EDGE,
	PIGPIO_INT_EITHER_EDGE = EITHER_EDGE
} PIGPIOInterruptEdge;

class IIOInterruptSink
{
public:
	IIOInterruptSink() {}
	virtual void IOCXInterrupt(PIGPIOInterruptEdge edge, uint64_t tick_us) = 0;
	virtual void PIGPIOInterrupt(int gpio_num, PIGPIOInterruptEdge edge, uint64_t tick_us) = 0;
	virtual ~IIOInterruptSink() {}
};

class ICANInterruptSink
{
public:
	ICANInterruptSink() {}
	virtual void CANInterrupt(uint64_t tick_us) = 0;
	virtual ~ICANInterruptSink() {}
};

class IAHRSInterruptSink
{
public:
	IAHRSInterruptSink() {}
	virtual void AHRSInterrupt(uint64_t tick_us) = 0;
	virtual ~IAHRSInterruptSink() {}
};

#endif /* IPIGPIOINTERRUPTSINKS_H_ */
