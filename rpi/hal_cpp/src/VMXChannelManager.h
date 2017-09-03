/*
 * VMXChannelManager.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXCHANNELMANAGER_H_
#define VMXCHANNELMANAGER_H_

#include <limits.h>
#include "VMXChannel.h"

/* Channel Map:
 *
 * Index   Description
 * -----   -----------
 *  0- 3:  VMX-pi 4-pin DIO Header (1-4)
 *  4- 5:  VMX-pi 2-pin QE1 Connector (A-B)
 *  6- 7:  VMX-pi 2-pin QE2 Connector (A-B)
 *  8- 9:  VMX-pi 2-pin QE3 Connector (A-B)
 * 10-11:  VMX-pi 2-pin QE4 Connector (A-B)
 * 12-21:  VMX-pi PWM Header (1-10) [RPI GPIOs]
 * 22-23:  VMX-pi UART Connector (TX/RX) [RPI GPIOs]
 * 24-27:  VMX-pi SPI Connector (CLK/MOSI/MISO/CS) [RPI GPIOs]
 * 28-31:  VMX-pi Analog In Header (1-4)
 * 32-33:  VMX-pi I2C Connector (SDA/SCL) RPI GPIOs)
 * 34-63:  Unused Channels
 */

typedef enum {
	NoHwOptions			= 0x00,
	IODirectionSelect	= 0x01
} VMXChannelHwOpt;

inline VMXChannelHwOpt VMXChannelHwOptAnd(VMXChannelHwOpt info1, VMXChannelHwOpt info2) {
	return (VMXChannelHwOpt)(int(info1) & int(info2));
}

inline bool VMXChannelHwOptCheck(VMXChannelHwOpt info1, VMXChannelHwOpt info2) {
	 return (VMXChannelHwOptAnd(info1, info2) != 0);
}

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
	for (uint8_t i = 0; i < (sizeof(caps)*CHAR_BIT); i++) {
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

class VMXChannelManager
{
public:
	static void Init();

	VMXChannelManager();
	~VMXChannelManager();
	uint8_t GetMaxNumChannels();
	uint8_t GetNumChannelsByCapability(VMXChannelCapability capability);
	static bool GetChannelTypeAndProviderSpecificIndex(VMXChannelIndex channel_index, VMXChannelType& type, uint8_t& type_specific_index);
	bool ChannelSupportsCapability(VMXChannelIndex channel_index, VMXChannelCapability capability);
	static VMXChannelCapability GetChannelCapabilityBits(VMXChannelIndex channel_index);
	static VMXChannelHwOpt GetChannelHwOpts(VMXChannelIndex channel_index);
	static uint8_t GetNumChannelsByType(VMXChannelType channel_type, VMXChannelIndex& first_channel_index);
	static uint8_t GetNumChannelsByTypeAndCapability(VMXChannelType channel_type, VMXChannelCapability, VMXChannelIndex& first_channel_index);
};

#endif /* VMXCHANNELMANAGER_H_ */
