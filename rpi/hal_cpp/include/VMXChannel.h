/*
 * VMXChannel.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXCHANNEL_H_
#define VMXCHANNEL_H_

#include <stdint.h>

/* Each VMXChannel has zero or more of the following Capabilities */
/* Some of these Capabilities are dynamic, and may be changed via
 * jumper (e.g., VMX PWM/DigInput jumper).  However, their state at the
 * beginning of the application will not change during the lifetime of the
 * application.
 *
 * When VMXChannel Capabilities refer to Shared Resources, there is no
 * guarantee that at any instant a VMXChannel can be configured with this,
 * as VMXChannels must be routed to resources w/sufficient availability
 * in order for the VMXChannel capability to be active.
 */

typedef enum {
	INVALID = 0,
	FlexDIO  = 1,
	AnalogIn = 2,
	HiCurrDIO = 3,
	CommDIO = 4,
	CommI2C = 5
} VMXChannelType;

typedef enum {
	None				= 0x00000000,
	DigitalInput		= 0x00000001,
	DigitalOutput		= 0x00000002,
	PWMGeneratorOutput	= 0x00000004,
	PWMGeneratorOutput2 = 0x00000008,
	PWMCaptureInput		= 0x00000010,
	EncoderAInput		= 0x00000020,
	EncoderBInput		= 0x00000040,
	AccumulatorInput	= 0x00000080,
	AnalogTriggerInput 	= 0x00000100,
	InterruptInput		= 0x00000200,
	UART_TX				= 0x00000400,
	UART_RX				= 0x00000800,
	SPI_CLK				= 0x00001000,
	SPI_MISO			= 0x00002000,
	SPI_MOSI			= 0x00004000,
	SPI_CS				= 0x00008000,
	I2C_SDA				= 0x00010000,
	I2C_SCL				= 0x00020000,
} VMXChannelCapability;

typedef uint8_t  VMXChannelIndex;

const VMXChannelIndex INVALID_VMX_CHANNEL_INDEX = 255;

#endif /* VMXCHANNEL_H_ */
