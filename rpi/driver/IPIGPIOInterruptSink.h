/*
 * IPIGPIOInterruptSink.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef IPIGPIOINTERRUPTSINK_H_
#define IPIGPIOINTERRUPTSINK_H_

#include <pigpio.h>

#define PIGPIO_INT_RISING_EDGE		RISING_EDGE
#define PIGPIO_INT_FALLING_EDGE 	FALLING_EDGE
#define PIGPIO_INT_EITHER_EDGE   	EITHER_EDGE

class IPIGPIOInterruptSink
{
public:
	virtual void iocx_interrupt(int level, uint32_t tick_us) = 0;
	virtual void can_interrupt(int level, uint32_t tick_us) = 0;
	virtual void ahrs_interrupt(int level, uint32_t tick_us) = 0;
	virtual void pigpio_interrupt(int gpio_num, int level, uint32_t tick_us) = 0;
	virtual ~IPIGPIOInterruptSink() {}
};

#endif /* IPIGPIOINTERRUPTSINK_H_ */
