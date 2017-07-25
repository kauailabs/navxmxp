/*
 * VMXResourceManager.cpp
 *
 *  Created on: 6 Jul 2017
 *      Author: pi
 */

#include "VMXResourceManager.h"
#include "VMXChannelManager.h"
#include "VMXResource.h"
#include "VMXResourceConfig.h"
#include "VMXHWOption.h"
#include <list>

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

static VMXResourceType stm32_basic_timer_shared_resources[] = { VMXResourceType::PWMCapture, VMXResourceType::PWMGenerator };
static VMXResourceType stm32_adv_timer_shared_resources[] = { VMXResourceType::PWMCapture, VMXResourceType::PWMGenerator, VMXResourceType::Encoder };
static VMXResourceType rpi_uart_shared_resources[] = { VMXResourceType::DigitalIO, VMXResourceType::UART };
static VMXResourceType rpi_spi_shared_resources[] = { VMXResourceType::DigitalIO, VMXResourceType::SPI };

static VMXSharedResourceGroup stm32_basic_timer_res_group =
{
	VMX_PI,
	stm32_basic_timer_shared_resources,
	ARRAY_COUNT(stm32_basic_timer_shared_resources)
};
static VMXSharedResourceGroup stm32_adv_timer_res_group =
{
	VMX_PI,
	stm32_adv_timer_shared_resources,
	ARRAY_COUNT(stm32_adv_timer_shared_resources)
};
static VMXSharedResourceGroup rpi_uart_res_group =
{
	RPI,
	rpi_uart_shared_resources,
	ARRAY_COUNT(rpi_uart_shared_resources)
};
static VMXSharedResourceGroup rpi_spi_res_group =
{
	RPI,
	rpi_spi_shared_resources,
	ARRAY_COUNT(rpi_spi_shared_resources)
};

static const DIOConfig 				def_dio_cfg;
static const InterruptConfig 		def_int_cfg;
static const PWMGeneratorConfig 	def_pwmgen_cfg;
static const PWMCaptureConfig 		def_pwmcap_cfg;
static const EncoderConfig      	def_enc_cfg;
static const AccumulatorConfig  	def_accum_cfg;
static const AnalogTriggerConfig	def_antrig_cfg;
static const UARTConfig				def_uart_cfg;
static const SPIConfig				def_spi_cfg;
static const I2CConfig				def_i2c_cfg;

static const DIOHWOption			dio_hwopt;
static const AccumulatorHWOption	acc_hwopt;

static const VMXResourceDescriptor resource_descriptors[] =
{
	{	VMXResourceType::DigitalIO, 	VMX_PI,    	0,	&def_dio_cfg,		&dio_hwopt,	0,	12,	1, 1, NULL, 1	},
	{	VMXResourceType::DigitalIO,		RPI,     	12,	&def_dio_cfg,		NULL,		12,	10,	1, 1, NULL, 1	},
	{	VMXResourceType::DigitalIO,		RPI,     	12,	&def_dio_cfg,		NULL,		22,	 2,	1, 1, &rpi_uart_res_group, 2	},
	{	VMXResourceType::DigitalIO,		RPI,     	12,	&def_dio_cfg,		NULL,		24,	 4,	1, 1, &rpi_spi_res_group, 4 },
	{	VMXResourceType::Interrupt,		VMX_PI,		0,	&def_int_cfg,		NULL,		0,	16,	1, 1, NULL, 1	},
	{	VMXResourceType::Interrupt,		RPI,		16,	&def_int_cfg,		NULL,		16,	16,	1, 1, NULL, 1	},
	{	VMXResourceType::PWMGenerator,	VMX_PI,		0,	&def_pwmgen_cfg,	NULL,		0,	5,	1, 2, &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::PWMGenerator,	VMX_PI,		0,	&def_pwmgen_cfg,	NULL,		5,	1,	1, 2, &stm32_basic_timer_res_group, 1 },
	{	VMXResourceType::PWMGenerator,	RPI,		12,	&def_pwmgen_cfg,	NULL,		12,	16,	1, 1, NULL, 1	},
	{	VMXResourceType::PWMCapture,	VMX_PI,		0,	&def_pwmcap_cfg,	NULL,		0,	5,	1, 1, &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::PWMCapture,	VMX_PI,		0,	&def_pwmcap_cfg,	NULL,		5,	1,	1, 1, &stm32_basic_timer_res_group, 1 },
	{	VMXResourceType::Encoder,		VMX_PI,		0,	&def_enc_cfg,		NULL,		0,	5,	2, 2, &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::Accumulator,	VMX_PI,		0,	&def_accum_cfg,		&acc_hwopt,	0,	4,	1, 1, NULL, 1	},
	{	VMXResourceType::AnalogTrigger,	VMX_PI,		0,	&def_antrig_cfg, 	NULL,		0,	4,	1, 1, NULL, 1	},
	{	VMXResourceType::UART,			RPI,		0,	&def_uart_cfg,		NULL,		0,	1,	2, 2, &rpi_uart_res_group, 1	},
	{	VMXResourceType::SPI,			RPI,		0,	&def_spi_cfg,		NULL,		0,	1,	4, 4, &rpi_spi_res_group, 1	},
	{	VMXResourceType::I2C,			RPI,		0,	&def_i2c_cfg,		NULL,		0,	1,	2, 2, NULL, 1	},
};

typedef struct {
	VMXChannelCapability channel_capability;
	VMXResourceType resource_type;
	VMXResourcePortIndex resource_port_index;
} VMXChannelCapabilityToResourceTypeMap;

static const VMXChannelCapabilityToResourceTypeMap channel_cap_to_res_type_map[] = {
	{ VMXChannelCapability::DIO,	 			VMXResourceType::DigitalIO,		0 },
	{ VMXChannelCapability::PWMGeneratorOutput, VMXResourceType::PWMGenerator, 	0 },
	{ VMXChannelCapability::PWMGeneratorOutput2,VMXResourceType::PWMGenerator, 	1 },
	{ VMXChannelCapability::PWMCaptureInput, 	VMXResourceType::PWMCapture, 	0 },
	{ VMXChannelCapability::EncoderAInput, 		VMXResourceType::Encoder, 		0 },
	{ VMXChannelCapability::EncoderBInput, 		VMXResourceType::Encoder, 		1 },
	{ VMXChannelCapability::AccumulatorInput, 	VMXResourceType::Accumulator, 	0 },
	{ VMXChannelCapability::AnalogTriggerInput, VMXResourceType::AnalogTrigger, 0 },
	{ VMXChannelCapability::InterruptInput, 	VMXResourceType::Interrupt, 	0 },
	{ VMXChannelCapability::UART_TX, 			VMXResourceType::UART, 			0 },
	{ VMXChannelCapability::UART_RX, 			VMXResourceType::UART, 			1 },
	{ VMXChannelCapability::SPI_CLK, 			VMXResourceType::SPI, 			0 },
	{ VMXChannelCapability::SPI_MISO, 			VMXResourceType::SPI, 			1 },
	{ VMXChannelCapability::SPI_MOSI, 			VMXResourceType::SPI, 			2 },
	{ VMXChannelCapability::SPI_CS, 			VMXResourceType::SPI, 			3 },
	{ VMXChannelCapability::I2C_SDA, 			VMXResourceType::I2C, 			0 },
	{ VMXChannelCapability::I2C_SCL, 			VMXResourceType::I2C, 			1 },
};

VMXResourceManager::VMXResourceManager()
{
	for ( size_t i = 0; i < (sizeof(resource_descriptors) / sizeof(resource_descriptors[0])); i++ ) {
		for ( int j = 0; j < resource_descriptors[i].resource_count; j++) {
			VMXResourceHandle new_res_handle =
					CREATE_VMX_RESOURCE_HANDLE(resource_descriptors[i].type, resource_descriptors[i].resource_num_first + j);
			VMXResource *p_resource = new VMXResource(resource_descriptors[i], VMXResourceIndex(j));
			resources.push_back(p_resource);
			resource_handle_to_resource_map[new_res_handle] = p_resource;
		}
	}
}

VMXResourceManager::~VMXResourceManager()
{
	for (std::map<VMXResourceHandle, VMXResource *>::iterator it=resource_handle_to_resource_map.begin(); it!=resource_handle_to_resource_map.end(); ++it) {
		delete it->second;
	}
}

bool VMXResourceManager::GetChannelChapabilitiesByResourceType(VMXResourceType res_type, VMXChannelCapability& channel_capability_bits)
{
	channel_capability_bits = VMXChannelCapability::None;
	for (size_t i = 0; i < (sizeof(channel_cap_to_res_type_map)/sizeof(channel_cap_to_res_type_map[0])); i++ ) {
		if(channel_cap_to_res_type_map[i].resource_type == res_type) {
			channel_capability_bits = VMXCapabilityOr(channel_capability_bits, channel_cap_to_res_type_map[i].channel_capability);
			break;
		}
	}
	return (channel_capability_bits != VMXChannelCapability::None);
}

bool VMXResourceManager::GetResourceTypeAndPortIndexForChannelCapability(VMXChannelCapability channel_capability_bit, VMXResourceType resource_type, VMXResourcePortIndex& resource_port) {
	for (size_t i = 0; i < (sizeof(channel_cap_to_res_type_map)/sizeof(channel_cap_to_res_type_map[0])); i++ ) {
		if(channel_cap_to_res_type_map[i].channel_capability & channel_capability_bit) {
			if(channel_cap_to_res_type_map[i].resource_type == resource_type) {
				resource_port = channel_cap_to_res_type_map[i].resource_port_index;
				return true;
			}
		}
	}
	return false;
}

bool VMXResourceManager::GetResourcePortIndexByChannelCapability(VMXResource *p_resource, VMXChannelCapability channel_capability_bit, VMXResourcePortIndex& resource_port ) {
	VMXResourceType res_type = p_resource->GetResourceType();
	return GetResourceTypeAndPortIndexForChannelCapability(channel_capability_bit, res_type, resource_port);
}

/* If this resource's ResourceDescriptor.max_num_routable_channel_slots != 1 the shared resource ResourceDescriptor.max_num_routable_channel_slots */
/*   then there is not a 1:1 mapping between resource slot indexes. */


bool VMXResourceManager::GetOtherResourcesInSharedResourceGroup(VMXResource *p_resource, std::list<VMXResource *>& other_shared_resources) {
	VMXSharedResourceGroup *p_shared_resource_group = p_resource->GetSharedResourceGroup();
	if (!p_shared_resource_group) return false;
	VMXResourceType this_resource_type = p_resource->GetResourceType();
	VMXResourceIndex this_res_index = p_resource->GetResourceIndex();

	for ( uint8_t i = 0; i < p_shared_resource_group->type_count; i++) {
		if (p_shared_resource_group->p_types[i] != this_resource_type) {
			VMXResource *p_shared_resource = GetVMXResource(CREATE_VMX_RESOURCE_HANDLE(p_shared_resource_group->p_types[i], this_res_index));
			if (p_shared_resource) {
				other_shared_resources.push_back(p_shared_resource);
			}
		}
	}
	return !other_shared_resources.empty();
}

VMXResource *VMXResourceManager::GetResourceFromRoutedChannel(VMXChannelIndex channel) {
	for (std::map<VMXResourceHandle, VMXResource *>::iterator it=resource_handle_to_resource_map.begin(); it!=resource_handle_to_resource_map.end(); ++it) {
		if(it->second->IsChannelRoutedToResource(channel)) {
			return it->second;
		}
	}
	return NULL;
}

uint8_t VMXResourceManager::GetMaxNumResources(VMXResourceType res_type)
{
	uint8_t res_count_by_type = 0;
	for ( size_t i = 0; i < (sizeof(resource_descriptors) / sizeof(resource_descriptors[0])); i++ ) {
		if(resource_descriptors[i].type == res_type) {
			res_count_by_type += resource_descriptors[i].resource_count;
		}
	}
	return res_count_by_type;
}

VMXResource *VMXResourceManager::GetVMXResource(VMXResourceHandle vmx_res_handle) {
	std::map<VMXResourceHandle, VMXResource *>::iterator it =
			resource_handle_to_resource_map.find(vmx_res_handle);
	if (it != resource_handle_to_resource_map.end()) {
		return it->second;
	}
	return NULL;
}

VMXChannelCapability VMXResourceManager::GetCompatibleChannelCapabilityBit(VMXChannelIndex channel_index, VMXResource *p_resource)
{
	VMXChannelCapability res_channel_caps = VMXChannelCapability::None;
	if (GetChannelChapabilitiesByResourceType(p_resource->GetResourceType(), res_channel_caps)) {
		VMXChannelCapability channel_caps = VMXChannelManager::GetChannelCapabilityBits(channel_index);
		res_channel_caps = VMXCapabilityAnd(res_channel_caps, channel_caps);
	}
	return res_channel_caps;
}

