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
#include "IInterruptRegistrar.h"
#include "VMXErrors.h"
#include <unordered_set>
#include <list>

class VMXIO {
private:
	PIGPIOClient& pigpio;
	IOCXClient&   iocx;
	MISCClient&   misc;
	VMXChannelManager& chan_mgr;
	VMXResourceManager& res_mgr;
	IInterruptRegistrar& int_reg;

	bool DisconnectAnalogInputInterrupt(uint8_t analog_trigger_num); /* ????? */

public:

	VMXIO(PIGPIOClient& pigpio, IOCXClient& iocx, MISCClient& misc, VMXChannelManager& chan_mgr, VMXResourceManager& res_mgr, IInterruptRegistrar& int_registrar);
	virtual ~VMXIO();

	/*** RESOURCE AND CHANNEL ENUMERATION */
	uint8_t GetNumResourcesByType(VMXResourceType resource_type);
	uint8_t GetNumChannelsByCapability(VMXChannelCapability channel_capability);
	uint8_t GetNumChannelsByType(VMXChannelType channel_type, VMXChannelIndex& first_channel_index);
	bool GetChannelCapabilities(VMXChannelIndex channel_index, VMXChannelType& channel_type, VMXChannelCapability& capability_bits);

	/*** RESOURCE HANDLE ACQUISITION */
	bool GetResourceHandle(VMXResourceType resource_type, VMXResourceIndex res_index, VMXResourceHandle& resource_handle, VMXErrorCode *errcode = 0);
	bool GetResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability capability, std::list<VMXResourceHandle>& compatible_res_handles);
	bool GetUnallocatedResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability capability, std::list<VMXResourceHandle>& unallocated_compatible_res_handles);
	bool GetChannelsCompatibleWithResource(VMXResourceHandle resource_handle, VMXChannelIndex& first_channel_index, uint8_t& num_channels);

	/*** RESOURCE ALLOCATION ***/

	bool IsResourceAllocated(VMXResourceHandle resource, bool& allocated, bool& is_shared, VMXErrorCode *errcode = 0);
	bool AllocateResource(VMXResourceHandle resource, VMXErrorCode* errcode = 0);
	bool DeallocateResource(VMXResourceHandle resource, VMXErrorCode *errcode = 0);
	bool DeallocateAllResources(VMXErrorCode *last_errorcode = 0);

	/*** RESOURCE-CHANNEL ROUTING ***/

	bool RouteChannelToResource(VMXChannelIndex channel, VMXResourceHandle resource, VMXErrorCode* errcode = 0);
	bool RouteResourceToResource(VMXResourceHandle from, VMXResourceHandle to);
	bool UnrouteChannelFromResource(VMXChannelIndex channel, VMXResourceHandle resource, VMXErrorCode *errcode = 0);
	bool UnrouteAllChannelsFromResource(VMXResourceHandle resource, VMXErrorCode *errcode = 0);
	bool UnrouteResourceFromResource(VMXResourceHandle from, VMXResourceHandle to);
	bool GetRoutedChannels(VMXResourceHandle resource, std::unordered_set<VMXChannelIndex>& routed_channels);
	bool GetRoutedResources(VMXResourceHandle resource, std::unordered_set<VMXResourceHandle>& routed_resources);

	/*** RESOURCE CONFIGURATION (see VMXResourceConfig.h for various configuration classes) ***/

	bool SetResourceConfig(VMXResourceHandle resource, const VMXResourceConfig* p_config, VMXErrorCode *errcode = 0);
	bool GetResourceConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config, VMXErrorCode *errcode = 0);
	bool GetResourceDefaultConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config, VMXErrorCode *errcode = 0);

	/*** RESOURCE ACTIVATION ***/

	bool IsResourceActive(VMXResourceHandle, bool &active, VMXErrorCode* errcode = 0);
	bool ActivateResource(VMXResourceHandle resource, VMXErrorCode* errcode = 0);
	bool DeactivateResource(VMXResourceHandle resource, VMXErrorCode* errcode = 0);

	/*** ACTIVATION HELPERS ***/
	bool ActivateSinglechannelResource(VMXChannelIndex channel_index, VMXChannelCapability channel_capability,
			VMXResourceHandle& res_handle, const VMXResourceConfig *res_cfg = 0, VMXErrorCode *errcode = 0);

	/*** RESOURCE ACTIONS ***/

	/* DIO Resources */
	bool DIO_Get(VMXResourceHandle dio_res_handle, bool& high, VMXErrorCode *errcode = 0);
	bool DIO_Set(VMXResourceHandle dio_res_handle, bool high, VMXErrorCode *errcode = 0);

	/* PWMGenerator */
	const uint8_t MIN_PWM_GENERATOR_DUTY_CYCLE = 0;
	const uint8_t MAX_PWM_GENERATOR_DUTY_CYCLE = 255;
	bool PWMGenerator_SetDutyCycle(VMXResourceHandle pwmgen_res_handle, VMXResourcePortIndex port_index, uint8_t duty_cycle, VMXErrorCode *errcode = 0);

	/* PWMCapture */
	/* Returns current PWM Capture count of 1us ticks in the current duty cycle. */
	bool PWMCapture_GetCount(VMXResourceHandle pwmcap_res_handle, int32_t& count, VMXErrorCode *errcode = 0);

	/* Encoder */
	/* Returns current integrated count of encoder ticks (at the current resolution) */
	bool Encoder_GetCount(VMXResourceHandle encoder_res_handle, int32_t& count, VMXErrorCode *errcode = 0);
	typedef enum { EncoderForward, EncoderReverse } EncoderDirection;
	bool Encoder_GetDirection(VMXResourceHandle encoder_res_handle, EncoderDirection& direction, VMXErrorCode *errcode = 0);
	bool Encoder_Reset(VMXResourceHandle encoder_res_handle, VMXErrorCode *errcode = 0);

	/* Accumulator */
	/* NOTE:  The resolution of Accumulator values is dependent upon the current number of bits */
	/* 0 bits:  12-bit resolution, 1 bit:  13-bit resolution, etc. */
	/* See the AccumulatorConfig for more information on modifying these bits */
	bool Accumulator_GetOversampleValue(VMXResourceHandle accum_res_handle, uint32_t oversample_value, VMXErrorCode *errcode = 0);
	bool Accumulator_GetAverageValue(VMXResourceHandle accum_res_handle, uint32_t average_value, VMXErrorCode *errcode = 0);
	bool Accumulator_GetInstantaneousValue(VMXResourceHandle accum_res_handle, uint32_t average_value, VMXErrorCode *errcode = 0);
	bool Accumulator_GetFullScaleVoltage(float& full_scale_voltage, VMXErrorCode *errcode = 0);

	/* Analog Trigger */
	typedef enum { BelowThreshold, AboveThreshold, InWindow } AnalogTriggerState;
	bool AnalogTrigger_GetState(VMXResourceHandle antrig_res_handle, AnalogTriggerState& state, VMXErrorCode *errcode = 0);

	/* Interrupt */

	/* UART */
	bool UART_Write(VMXResourceHandle uart_res_handle, uint8_t *p_data, uint16_t size, VMXErrorCode *errcode = 0);
	bool UART_Read(VMXResourceHandle uart_res_handle, uint8_t *p_data, uint16_t size, VMXErrorCode *errcode = 0);
	bool UART_GetBytesAvailable(VMXResourceHandle uart_es_handle, uint16_t& size, VMXErrorCode *errcode = 0);

	/* SPI */
	bool SPI_Write(VMXResourceHandle spi_res_handle, uint8_t *p_send_data, uint16_t size, VMXErrorCode *errcode = 0);
	bool SPI_Read(VMXResourceHandle spi_res_handle, uint8_t *p_rcv_data, uint16_t size, VMXErrorCode *errcode = 0);
	bool SPI_Transaction(VMXResourceHandle spi_res_handle, uint8_t *p_send_data, uint8_t *p_rcv_data, uint16_t size, VMXErrorCode *errcode = 0);

	/* I2C */
	bool I2C_Write(VMXResourceHandle i2c_res_handle, uint8_t deviceAddress, uint8_t* dataToSend, int32_t sendSize, VMXErrorCode *errcode = 0);
	bool I2C_Read(VMXResourceHandle i2c_res_handle, uint8_t deviceAddress, uint8_t* buffer, int32_t count, VMXErrorCode *errcode = 0);
	bool I2C_Transaction(VMXResourceHandle i2c_res_handle, uint8_t deviceAddress,
	                    uint8_t* dataToSend, uint16_t sendSize,
	                    uint8_t* dataReceived, uint16_t receiveSize, VMXErrorCode *errcode = 0);

	/* TESTS */
	void TestPWMOutputs() { pigpio.test_pwm_outputs(); }
	void TestExtI2C() { pigpio.test_ext_i2c(); }
	void TestGPIInputs(int iteration_count) { pigpio.test_gpio_inputs(iteration_count); }
};

#endif /* VMXIO_H_ */
