/*
 * VMXResource.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXRESOURCE_H_
#define VMXRESOURCE_H_

#include <stdint.h>
#include <vector>
#include <list>

#include "VMXResourceProvider.h"
#include "VMXChannel.h"

typedef enum {
	Undefined,
	DigitalIO,
	PWMGenerator,
	PWMCapture,
	Encoder,
	Accumulator,
	AnalogTrigger,
	Interrupt,
	UART,
	SPI,
	I2C
} VMXResourceType;

typedef struct {
	VMXResourceProviderType provider_type;
	VMXResourceType *		p_types;
	uint8_t					type_count;
} VMXSharedResourceGroup;

/* Represents resource-type specific configuration data that must be
 * set to a valid default before activating the resource, and if any
 * of this data changes, the resources must be first deactivated
 * before the configuration can change.
 */
struct VMXResourceConfig {
	VMXResourceType res_type;
	VMXResourceConfig(VMXResourceType res_type) {
		this->res_type = res_type;
	}
	VMXResourceType GetResourceType() const { return res_type; }
	virtual VMXResourceConfig *GetCopy() const = 0;
	virtual bool Copy(const VMXResourceConfig *p_config) = 0;
	virtual ~VMXResourceConfig() {}
};

struct VMXHWOption {
	VMXResourceType res_type;
	VMXHWOption(VMXResourceType res_type) {
		this->res_type = res_type;
	}
	VMXResourceType GetResourceType() { return res_type; }

};

typedef struct {
	VMXResourceType 			type;
	VMXResourceProviderType 	provider_type;
	uint8_t						provider_resource_index_offset;
	const VMXResourceConfig *	p_res_cfg;   /* Must NOT be NULL */
	const VMXHWOption *			p_hw_option; /* May be NULL */
	uint8_t						resource_num_first;
	uint8_t						resource_count;
	uint8_t						min_num_ports;
	uint8_t						max_num_ports;
	VMXSharedResourceGroup *	p_shared_resource_group;
	uint8_t						shared_resource_group_index_divisor; /* Divide this resource's index to yield the common index within the group. */
} VMXResourceDescriptor;

typedef uint8_t  VMXResourceIndex;
typedef uint16_t VMXResourceHandle;
typedef uint8_t  VMXResourcePortIndex;

const VMXResourceIndex INVALID_VMX_RESOURCE_INDEX = 255;

#define INVALID_VMX_RESOURCE_HANDLE(vmx_res_handle)     (((uint8_t)vmx_res_handle)==INVALID_VMX_RESOURCE_INDEX)
#define CREATE_VMX_RESOURCE_HANDLE(res_type,res_index) 	((((uint16_t)res_type)<<8) || (uint8_t)res_index)
#define EXTRACT_VMX_RESOURCE_TYPE(res_handle)			(VMXResourceType)(res_handle >> 8)
#define EXTRACT_VMX_RESOURCE_INDEX(res_handle)			(uint8_t)(res_handle & 0x00FF)

const int INVALID_PROVIDER_RESOURCE_HANDLE = -1;

using namespace std;

class VMXResource {
	typedef struct {
		VMXChannelIndex channel_index;
	} VMXChannelRouting;

	const VMXResourceDescriptor& descriptor;
	VMXResourceIndex resource_index;
	bool allocated_primary;
	bool allocated_shared;
	std::vector<VMXChannelRouting> port_routings;
	VMXResourceConfig *p_res_config;
	VMXHWOption *p_hw_option;
	int provider_resource_handle;
	bool active;

public:
	VMXResource(const VMXResourceDescriptor& resource_descriptor, VMXResourceIndex res_index) :
		descriptor(resource_descriptor),
		resource_index(res_index),
		allocated_primary(false),
		allocated_shared(false),
		port_routings(resource_descriptor.max_num_ports)
	{
		p_res_config = descriptor.p_res_cfg->GetCopy();
		p_hw_option = 0;
		provider_resource_handle = INVALID_PROVIDER_RESOURCE_HANDLE;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			port_routings[i].channel_index = INVALID_VMX_CHANNEL_INDEX;
		}
		active = false;
	}

	~VMXResource() {
	}

	bool SetActive(bool active) {
		if (!active) {
			this->active = active;
			return true;
		}
		return false;
	}

	bool GetActive() const {
		return active;
	}

	VMXResourceHandle GetResourceHandle() const {
		return CREATE_VMX_RESOURCE_HANDLE(GetResourceType(), GetResourceIndex());
	}

	VMXResourceType GetResourceType() const {
		return descriptor.type;
	}

	VMXResourceIndex GetResourceIndex() const {
		return resource_index;
	}

	VMXResourceProviderType GetResourceProviderType() const {
		return descriptor.provider_type;
	}

	uint8_t GetMinNumPorts() const {
		return descriptor.min_num_ports;
	}

	uint8_t GetMaxNumPorts() const {
		return descriptor.max_num_ports;
	}

	uint8_t GetProviderResourceIndex() const {
		return resource_index - descriptor.provider_resource_index_offset;
	}

	VMXSharedResourceGroup *GetSharedResourceGroup() const {
		return descriptor.p_shared_resource_group;
	}

	int GetProviderResourceHandle() const {
		return provider_resource_handle;
	}

	void SetProviderResourceHandle(int provider_resource_handle) {
		this->provider_resource_handle = provider_resource_handle;
	}

	bool IsAllocated() const {
		return (allocated_primary || allocated_shared);
	}

	bool IsAllocatedPrimary() const {
		return allocated_primary;
	}

	bool IsAllocatedShared() const {
		return allocated_shared;
	}

	bool RouteChannel(VMXChannelIndex channel_index, VMXResourcePortIndex port_index) {
		if (!allocated_primary) return false;
		if (port_index >= descriptor.max_num_ports) return false; /* Invalid resource slot index */
		if (channel_index > LAST_VALID_VMX_CHANNEL_INDEX) return false;

		port_routings[port_index].channel_index = channel_index;
		return true;
	}

	bool AllocatePrimary() {
		if (allocated_primary) return false;
		allocated_primary = true;
		return allocated_primary;
	}

	bool DeallocatePrimary() {
		if (!allocated_primary) return false;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			port_routings[i].channel_index = INVALID_VMX_CHANNEL_INDEX;
		}
		allocated_primary = false;
		return allocated_primary;
	}

	bool AllocateShared() {
		if (!descriptor.p_shared_resource_group) return false;
		if (allocated_shared) return false;
		allocated_shared = true;
		return allocated_shared;
	}

	bool DeallocateShared() {
		if (!descriptor.p_shared_resource_group) return false;
		if (!allocated_shared) return false;
		allocated_shared = false;
		return allocated_shared;
	}

	uint8_t GetNumRoutedChannels() const {
		if (!IsAllocated()) return 0;
		uint8_t allocated_channel_count = 0;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			VMXChannelIndex routed_channel_index = port_routings[i].channel_index;
			if(routed_channel_index != INVALID_VMX_CHANNEL_INDEX) {
				allocated_channel_count++;
			}
		}
		return allocated_channel_count;
	}

	bool GetRoutedResourcePortIndex(std::list<VMXResourcePortIndex>& routed_port_index_list) const {
		if (!IsAllocated()) return false;
		if (!allocated_primary) return false;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			VMXChannelIndex routed_channel_index = port_routings[i].channel_index;
			if(routed_channel_index != INVALID_VMX_CHANNEL_INDEX) {
				routed_port_index_list.push_back(i);
			}
		}
		return true;
	}

	bool GetRoutedChannels(std::vector<VMXChannelIndex>& routed_channel_list) const {
		if (!IsAllocated()) return false;
		if (!allocated_primary) return false;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			VMXChannelIndex routed_channel_index = port_routings[i].channel_index;
			if(routed_channel_index != INVALID_VMX_CHANNEL_INDEX) {
				routed_channel_list.push_back(routed_channel_index);
			}
		}
		return true;
	}

	bool IsChannelRoutedToResource(VMXChannelIndex channel_index) const {
		for ( size_t i = 0; i < port_routings.size(); i++) {
			if(port_routings[i].channel_index == channel_index) {
				return true;
			}
		}
		return false;
	}

	bool UnrouteChannel(VMXChannelIndex channel_index) {
		for ( size_t i = 0; i < port_routings.size(); i++) {
			if(port_routings[i].channel_index == channel_index) {
				port_routings[i].channel_index = INVALID_VMX_CHANNEL_INDEX;
				return true;
			}
		}
		return false;
	}

	bool SetResourceConfig(const VMXResourceConfig *p_config) {
		if (!p_config) return false;

		if (p_config->GetResourceType() != this->GetResourceType()) return false;

		return this->p_res_config->Copy(p_config);
	}

	bool GetResourceConfig(VMXResourceConfig *p_config) const {
		if (!p_config) return false;

		if (p_config->GetResourceType() != this->GetResourceType()) return false;

		return p_config->Copy(this->p_res_config);
	}

	bool ChannelRoutingComplete() const {
		return (GetNumRoutedChannels() >= descriptor.min_num_ports);
	}
};

#endif /* VMXRESOURCE_H_ */
