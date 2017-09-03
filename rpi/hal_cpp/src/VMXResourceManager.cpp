/*
 * VMXResourceManager.cpp
 *
 *  Created on: 6 Jul 2017
 *      Author: pi
 */

#include "VMXChannelManager.h"
#include "VMXResourceManager.h"
#include "Logging.h"
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

static const VMXResourceDescriptor resource_descriptors[] =
{
	/*  Type                            Provider   PRI FVC #VC/R Default Config     FRI #Res MinP MaxP GRP ResIndexDiv */
	{	VMXResourceType::DigitalIO, 	VMX_PI,    	0,	0,	1,	 &def_dio_cfg,		0,	12,	 1,   1,  NULL, 1	},
	{	VMXResourceType::DigitalIO,		RPI,     	0,	12, 1,	 &def_dio_cfg,		12,	10,	 1,   1,  NULL, 1	},
	{	VMXResourceType::DigitalIO,		RPI,     	10,	26, 1,	 &def_dio_cfg,		22,	 2,	 1,   1,  &rpi_uart_res_group, 2	},
	{	VMXResourceType::DigitalIO,		RPI,     	12,	28, 1,	 &def_dio_cfg,		24,	 4,	 1,   1,  &rpi_spi_res_group, 4 },
	{	VMXResourceType::Interrupt,		VMX_PI,		0,	0,	1,	 &def_int_cfg,		0,	12,	 1,   1,  NULL, 1	},
	{	VMXResourceType::Interrupt,		RPI,		0,	12,	1,	 &def_int_cfg,		12,	10,	 1,   1,  NULL, 1	},
	{	VMXResourceType::Interrupt,		VMX_PI,		12,	22,	1,	 &def_int_cfg,		22,	 4,	 1,   1,  NULL, 1	},
	{	VMXResourceType::Interrupt,		RPI,		10,	26,	1,	 &def_int_cfg,		26,	 6,	 1,   1,  NULL, 1	},
	{	VMXResourceType::PWMGenerator,	VMX_PI,		0,	0,	2,	 &def_pwmgen_cfg,	0,	5,	 1,   2,  &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::PWMGenerator,	VMX_PI,		5,	10,	2,	 &def_pwmgen_cfg,	5,	1,	 1,   2,  &stm32_basic_timer_res_group, 1 },
	{	VMXResourceType::PWMGenerator,	RPI,		0,	12,	1,	 &def_pwmgen_cfg,	6,	16,	 1,   1,  NULL, 1	},
	{	VMXResourceType::PWMCapture,	VMX_PI,		0,	0,	2,	 &def_pwmcap_cfg,	0,	5,	 1,   1,  &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::PWMCapture,	VMX_PI,		5,	10,	2,	 &def_pwmcap_cfg,	5,	1,	 1,   1,  &stm32_basic_timer_res_group, 1 },
	{	VMXResourceType::Encoder,		VMX_PI,		0,	0,	2,	 &def_enc_cfg,		0,	4/*5*/,	  2,  2, &stm32_adv_timer_res_group, 1 },
	{	VMXResourceType::Accumulator,	VMX_PI,		0,	22,	1,	 &def_accum_cfg,	0,	4,	 1,   1,  NULL, 1	},
	{	VMXResourceType::AnalogTrigger,	VMX_PI,		0,	22,	1,	 &def_antrig_cfg, 	0,	4,	 1,   1,  NULL, 1	},
	{	VMXResourceType::UART,			RPI,		0,	26,	2,	 &def_uart_cfg,		0,	1,	 2,   2,  &rpi_uart_res_group, 1	},
	{	VMXResourceType::SPI,			RPI,		0,	28,	4,	 &def_spi_cfg,		0,	1,	 4,   4,  &rpi_spi_res_group, 1	},
	{	VMXResourceType::I2C,			RPI,		0,	32,	2,	 &def_i2c_cfg,		0,	1,	 2,   2,  NULL, 1	},
};

typedef struct {
	VMXChannelCapability channel_capability;
	VMXResourceType resource_type;
	VMXResourcePortIndex resource_port_index;
} VMXChannelCapabilityToResourceTypeMap;

static const VMXChannelCapabilityToResourceTypeMap channel_cap_to_res_type_map[] = {
	{ VMXChannelCapability::DigitalInput,		VMXResourceType::DigitalIO,		0 },
	{ VMXChannelCapability::DigitalOutput,		VMXResourceType::DigitalIO,		0 },
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

typedef struct {
	VMXChannelType channel_type;
	VMXResourceProviderType provider_type;
} VMXChannelTypeToVMXResourceProviderTypeMap;

static const VMXChannelTypeToVMXResourceProviderTypeMap chan_type_to_res_prov_type_map[] =
{
	{ FlexDIO, VMX_PI },
	{ AnalogIn, VMX_PI },
	{ HiCurrDIO, RPI },
	{ CommDIO, RPI },
	{ CommI2C, RPI },
};

VMXResourceManager::VMXResourceManager()
{
	for ( size_t i = 0; i < (sizeof(resource_descriptors) / sizeof(resource_descriptors[0])); i++ ) {
		VMXChannelIndex first_channel_index = resource_descriptors[i].first_channel_index;
		for ( int j = 0; j < resource_descriptors[i].resource_count; j++) {
			VMXResourceIndex new_res_index = VMXResourceIndex(resource_descriptors[i].resource_index_first + j);
			VMXResourceHandle new_res_handle =
					CREATE_VMX_RESOURCE_HANDLE(resource_descriptors[i].type, new_res_index);
			VMXResource *p_resource = new VMXResource(resource_descriptors[i], new_res_index, VMXChannelIndex(first_channel_index));
			resources.push_back(p_resource);
			resource_handle_to_resource_map[new_res_handle] = p_resource;
			first_channel_index += resource_descriptors[i].num_channels;
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
			channel_capability_bits = VMXChannelCapabilityOr(channel_capability_bits, channel_cap_to_res_type_map[i].channel_capability);
		}
	}
	return (channel_capability_bits != VMXChannelCapability::None);
}

bool VMXResourceManager::GetPortIndexForResourceTypeAndChannelCapability(VMXChannelCapability channel_capability_bit, VMXResourceType resource_type, VMXResourcePortIndex& resource_port) {
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
	return GetPortIndexForResourceTypeAndChannelCapability(channel_capability_bit, res_type, resource_port);
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

VMXResource *VMXResourceManager::GetVMXResourceAndVerifyType(VMXResourceHandle vmx_res_handle, VMXResourceType vmx_res_type) {
	VMXResource *p_vmx_res = GetVMXResource(vmx_res_handle);
	if ((!p_vmx_res) || (p_vmx_res->GetResourceType() != vmx_res_type)) {
		return 0;
	}
	return p_vmx_res;
}


VMXChannelCapability VMXResourceManager::GetCompatibleChannelCapabilityBits(VMXChannelIndex channel_index, VMXResource *p_resource)
{
	VMXChannelCapability res_channel_caps = VMXChannelCapability::None;
	if (GetChannelChapabilitiesByResourceType(p_resource->GetResourceType(), res_channel_caps)) {
		VMXChannelCapability channel_caps = VMXChannelManager::GetChannelCapabilityBits(channel_index);
		res_channel_caps = VMXChannelCapabilityAnd(res_channel_caps, channel_caps);
	}
	return res_channel_caps;
}

bool VMXResourceManager::GetResourceHandle(VMXResourceType resource_type, VMXResourceIndex res_index, VMXResourceHandle& resource_handle, VMXErrorCode *errcode)
{
	if (resource_type == VMXResourceType::Undefined) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_TYPE);
		return false;
	}
	if (int(resource_type) >= int(VMXResourceType::MaxVMXResourceType)) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_TYPE);
		return false;
	}

	if (res_index >= VMXResourceManager::GetMaxNumResources(resource_type)) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_INDEX);
	}

	resource_handle = CREATE_VMX_RESOURCE_HANDLE(resource_type, res_index);
	return true;
}

bool VMXResourceManager::GetResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability channel_capability, std::list<VMXResourceHandle>& compatible_res_handles, VMXErrorCode *errcode)
{
	bool success = false;
	VMXChannelType channel_type;
	VMXResourceProviderType res_provider_type;
	uint8_t provider_specific_channel_index;
	if (!VMXChannelManager::GetChannelTypeAndProviderSpecificIndex(channel_index, channel_type, provider_specific_channel_index)) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_CHANNEL_TYPE);
		return false;
	}
	if (!GetResourceProviderTypeFromChannelType(channel_type, res_provider_type)) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_PROVIDER_TYPE);
		return false;
	}

	std::list<VMXResourceType> compatible_resource_types;
	for (size_t i = 0; i < (sizeof(channel_cap_to_res_type_map)/sizeof(channel_cap_to_res_type_map[0])); i++ ) {
		if(channel_cap_to_res_type_map[i].channel_capability & channel_capability) {
			compatible_resource_types.push_back(channel_cap_to_res_type_map[i].resource_type);
		}
	}
	if (compatible_resource_types.size() < 1) {
		log_debug("No compatible resource types found for Channel capability %d\n", channel_capability);
	}

	for (std::list<VMXResourceType>::iterator it = compatible_resource_types.begin(); it != compatible_resource_types.end(); ++it) {
		for (size_t i = 0; i < (sizeof(resource_descriptors)/sizeof(resource_descriptors[0])); i++ ) {
			if (resource_descriptors[i].type == (*it)) {
				if ( resource_descriptors[i].provider_type == res_provider_type) {
					VMXChannelIndex resource_first_channel_index = resource_descriptors[i].first_channel_index;
					VMXChannelIndex resource_last_channel_index = resource_descriptors[i].first_channel_index +
							(resource_descriptors[i].resource_count * resource_descriptors[i].num_channels) - 1;
					if ((channel_index >= resource_first_channel_index) &&
						(channel_index <= resource_last_channel_index)) {
						/* Match */
						VMXResourceIndex res_index = resource_descriptors[i].resource_index_first +
								((channel_index - resource_first_channel_index) / resource_descriptors[i].max_num_ports);
						VMXResourceHandle compatible_res_handle = CREATE_VMX_RESOURCE_HANDLE(resource_descriptors[i].type, res_index);
						compatible_res_handles.push_back(compatible_res_handle);
						success = true;
					}
				}
			}
		}
	}
	if (!success) {
		SET_VMXERROR(errcode, VMXERR_IO_CHANNEL_RESOURCE_INCOMPATIBILITY);
	}
	return success;
}

bool VMXResourceManager::GetResourcesCompatibleWithChannelsAndCapability(uint8_t num_channels, VMXChannelIndex* p_channel_indexes, VMXChannelCapability *p_channel_capabilities, std::list<VMXResourceHandle>& compatible_res_handles, VMXErrorCode *errcode)
{
	std::list<VMXResourceHandle> compatible_res_handle_array[num_channels];
	std::map<VMXResourceHandle, uint8_t> resource_handle_to_count_map;
	std::map<VMXResourceHandle, uint8_t>::iterator map_it;
	for (uint8_t i = 0; i < num_channels; i++) {
		if (!GetResourcesCompatibleWithChannelAndCapability(p_channel_indexes[i], p_channel_capabilities[i], compatible_res_handle_array[i], errcode)) {
			return false;
		}
		std::list<VMXResourceHandle>::iterator list_it;
		for (list_it = compatible_res_handle_array[i].begin();
				list_it != compatible_res_handle_array[i].end();
				list_it++) {
			VMXResourceHandle res_handle = *list_it;
			if (i==0) {
				resource_handle_to_count_map[res_handle] = 1;
			} else {
				map_it = resource_handle_to_count_map.find(res_handle);
				if (map_it != resource_handle_to_count_map.end()) {
					map_it->second = map_it->second + 1;
				}
			}
		}
	}
	for (map_it=resource_handle_to_count_map.begin(); map_it != resource_handle_to_count_map.end(); map_it++ ) {
		if (map_it->second == num_channels) {
			compatible_res_handles.push_back(map_it->first);
		}
	}
	bool success = (compatible_res_handles.size() > 0);
	if (!success) {
		SET_VMXERROR(errcode, VMXERR_IO_NO_AVAILABLE_RESOURCE_PORTS);
	}
	return success;
}

bool VMXResourceManager::GetChannelsCompatibleWithResource(VMXResourceHandle resource_handle, VMXChannelIndex& first_channel_index, uint8_t& num_channels)
{
	VMXResource *p_vmx_resource = GetVMXResource(resource_handle);
	if (!p_vmx_resource) return false;
	return p_vmx_resource->GetRoutableChannelIndexes(first_channel_index, num_channels);
}


bool VMXResourceManager::GetResourceProviderTypeFromChannelType(VMXChannelType chan_type, VMXResourceProviderType& res_provider_type) {
	for (size_t i = 0; i < (sizeof(chan_type_to_res_prov_type_map)/sizeof(chan_type_to_res_prov_type_map[0])); i++ ) {
		if (chan_type_to_res_prov_type_map[i].channel_type == chan_type) {
			res_provider_type = chan_type_to_res_prov_type_map[i].provider_type;
			return true;
		}
	}
	return false;
}
