/*
 * VMXChannelManager.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXCHANNELMANAGER_H_
#define VMXCHANNELMANAGER_H_

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

class VMXChannelManager
{
public:
	static void Init();

	VMXChannelManager();
	~VMXChannelManager();
	uint8_t GetNumChannelsByCapability(VMXChannelCapability capability);
	bool GetChannelTypeAndProviderSpecificIndex(VMXChannelIndex channel_index, VMXChannelType& type, uint8_t& type_specific_index);
	bool ChannelSupportsCapability(VMXChannelIndex channel_index, VMXChannelCapability capability);
	static VMXChannelCapability GetChannelCapabilityBits(VMXChannelIndex channel_index);
};

#endif /* VMXCHANNELMANAGER_H_ */
