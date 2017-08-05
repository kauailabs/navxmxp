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
	IOCX_D = 1,
	IOCX_A = 2,
	PIGPIO = 3
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

inline VMXChannelCapability VMXChannelCapabilityAnd(VMXChannelCapability cap1, VMXChannelCapability cap2) {
	return (VMXChannelCapability)(int(cap1) & int(cap2));
}

inline VMXChannelCapability VMXChannelCapabilityOr(VMXChannelCapability cap1, VMXChannelCapability cap2) {
	return (VMXChannelCapability)(int(cap1) | int(cap2));
}

inline bool VMXChannelCapabilityCheck(VMXChannelCapability cap_bits, VMXChannelCapability cap) {
	 return (VMXChannelCapabilityAnd(cap_bits, cap) != 0);
}

inline VMXChannelCapability VMXChannelCapabilityClear(VMXChannelCapability cap_bits, VMXChannelCapability caps_to_clear) {
	int caps_inverse = ~int(caps_to_clear);
	return (VMXChannelCapability)(int(cap_bits) & caps_inverse);
}

inline bool IsVMXChannelCapabilityUnitary(VMXChannelCapability capability) {
	int cap_count = 0;
	uint32_t caps = uint32_t(capability);
	for ( int i = 0; i < 32; i++) {
		if (caps & 0x00000001) {
			cap_count++;
		}
		if (cap_count > 1) {
			return false;
		}
		caps >>= 1;
	}
	return (cap_count == 1);
}

typedef uint64_t VMXChannelDescriptor;

typedef uint8_t  VMXChannelIndex;

const VMXChannelIndex LAST_VALID_VMX_CHANNEL_INDEX = 33;
const VMXChannelIndex INVALID_VMX_CHANNEL_INDEX = 255;
const VMXChannelIndex SHARED_VMX_CHANNEL_INDEX = 254; /* A VMXChannel already allocated to a VMXResource in a shared resource group. */

#endif /* VMXCHANNEL_H_ */
