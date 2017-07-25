/*
 * VMXIO.h
 *
 *  Created on: 6 Jul 2017
 *      Author: pi
 */

#ifndef VMXIO_H_
#define VMXIO_H_

#include "PIGPIOClient.h"
#include "IOCXClient.h"
#include "MISCClient.h"
#include "VMXResourceManager.h"
#include "VMXChannelManager.h"
#include "VMXResourceConfig.h"
#include <unordered_set>

class VMXIO {
private:
	PIGPIOClient& pigpio;
	IOCXClient&   iocx;
	MISCClient&   misc;
	VMXChannelManager& chan_mgr;
	VMXResourceManager& res_mgr;

	bool DisconnectAnalogInputInterrupt(uint8_t analog_trigger_num);
public:

	VMXIO(PIGPIOClient& pigpio, IOCXClient& iocx, MISCClient& misc, VMXChannelManager& chan_mgr, VMXResourceManager& res_mgr);
	virtual ~VMXIO();

	uint8_t GetNumAvailableResources(VMXResourceType resource_type);

	/*** RESOURCE ALLOCATION ***/

	bool AllocateResource(VMXResourceHandle resource);
	bool DeallocateResource(VMXResourceHandle resource);

	/*** RESOURCE-CHANNEL ROUTING ***/

	bool RouteChannelToResource(VMXChannelIndex channel, VMXResourceHandle resource);
	bool RouteResourceToResource(VMXResourceHandle from, VMXResourceHandle to);
	bool UnrouteChannelFromResource(VMXChannelIndex channel, VMXResourceHandle resource);
	bool UnrouteAllChannelsFromResource(VMXResourceHandle resource);
	bool UnrouteResourceFromResource(VMXResourceHandle from, VMXResourceHandle to);
	bool GetRoutedChannels(VMXResourceHandle resource, std::unordered_set<VMXChannelIndex>& routed_channels);
	bool GetRoutedResources(VMXResourceHandle resource, std::unordered_set<VMXResourceHandle>& routed_resources);

	/*** RESOURCE CONFIGURATION (see VMXResourceConfig.h for various configuration classes) ***/

	bool SetResourceConfig(VMXResourceHandle resource, const VMXResourceConfig* p_config);
	bool GetResourceConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config);

	/*** RESOURCE ACTIVATION ***/

	bool ActivateResource(VMXResourceHandle resource);
	bool DeactivateResource(VMXResourceHandle resource);

	/*** RESOURCE ACTIONS ***/

	/* DIO Resources */
	bool DIO_Get(VMXResourceHandle res_handle, bool& high);
	bool DIO_Set(VMXResourceHandle res_handle, bool high);
	/* PWMGenerator */
	bool PWMGenerator_SetDutyCycle(VMXResourceHandle res_handle, uint8_t /* 0-255 */);
	/* PWMCapture */
	bool PWMCapture_GetCount(VMXResourceHandle res_handle, int32_t& count);
	/* Encoder */
	bool Encoder_GetCount(VMXResourceHandle res_handle, int32_t& count);
	/* Accumulator */
	bool Accumulator_GetOversampleBits(VMXResourceHandle res_handle, uint32_t& oversample_bits);
	bool Accumulator_GetAverageBits(VMXResourceHandle res_handle, uint32_t average_bits);
	/* Analog Trigger */
	bool AnalogTrigger_GetState(VMXResourceHandle res_handle, uint8_t& state /* 0 = below threshold; 1 = above threshold; 2 = in window*/);
	/* Interrupt */
	/* UART */
	bool UART_Write(VMXResourceHandle res_handle, uint8_t *p_data, uint16_t size);
	bool UART_Read(VMXResourceHandle res_handle, uint8_t *p_data, uint16_t size);
	bool UART_GetBytesAvailable(VMXResourceHandle res_handle, uint16_t& size);
	/* SPI */
	bool SPI_Write(VMXResourceHandle res_handle, uint8_t *p_send_data, uint16_t size);
	bool SPI_Read(VMXResourceHandle res_handle, uint8_t *p_rcv_data, uint16_t size);
	bool SPI_Transaction(VMXResourceHandle res_handle, uint8_t *p_send_data, uint8_t *p_rcv_data, uint16_t size);
	/* I2C */
	bool I2C_Write(VMXResourceHandle res_handle, uint8_t deviceAddress, uint8_t* dataToSend, int32_t sendSize);
	bool I2C_Read(VMXResourceHandle res_handle, uint8_t deviceAddress, uint8_t* buffer, int32_t count);
	bool I2C_Transaction(VMXResourceHandle res_handle, uint8_t deviceAddress,
	                    uint8_t* dataToSend, uint16_t sendSize,
	                    uint8_t* dataReceived, uint16_t receiveSize);
};

#endif /* VMXIO_H_ */
