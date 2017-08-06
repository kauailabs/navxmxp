/*
 * VMXResourceImpl.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXRESOURCEIMPL_H_
#define VMXRESOURCEIMPL_H_

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <list>

#include "VMXChannel.h"
#include "VMXErrors.h"
#include "VMXResource.h"
#include "VMXResourceConfig.h"

typedef enum {
	VMX_PI,
	RPI
} VMXResourceProviderType;

typedef struct {
	VMXResourceProviderType provider_type;
	VMXResourceType *		p_types;
	uint8_t					type_count;
} VMXSharedResourceGroup;

typedef struct {
	VMXResourceType 			type;								 /* Type of resource. */
	VMXResourceProviderType 	provider_type;						 /* Type of resource provider */
	uint8_t						provider_resource_index_offset;		 /* Offset of this set of resources within the resource provider. */
	VMXChannelIndex				first_channel_index;				 /* First VMXChannelIndex (within the set of resources) that can be routed to this Resource. */
	uint8_t						num_channels;						 /* Num Channels that can be routed to this Resource. */
	const VMXResourceConfig *	p_res_cfg;   						 /* Default Configuration for this Resource. Must NOT be NULL */
	VMXResourceIndex			resource_index_first;				 /* First Resource Index */
	uint8_t						resource_count;						 /* Number of resource instances */
	uint8_t						min_num_ports;						 /* Min allowable ports on this resource. */
	uint8_t						max_num_ports;						 /* Max ports available on this resource. */
	VMXSharedResourceGroup *	p_shared_resource_group;			 /* May be NULL */
	uint8_t						shared_resource_group_index_divisor; /* Divide this resource's index to yield the common resource index within the group. */
} VMXResourceDescriptor;

using namespace std;

const int INVALID_PROVIDER_RESOURCE_HANDLE = -1;

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
	int provider_resource_handle;
	bool active;
	VMXChannelIndex first_routable_channel_index;

public:
	VMXResource(const VMXResourceDescriptor& resource_descriptor, VMXResourceIndex res_index, VMXChannelIndex first_channel_index) :
		descriptor(resource_descriptor),
		resource_index(res_index),
		allocated_primary(false),
		allocated_shared(false),
		port_routings(resource_descriptor.max_num_ports)
	{
		p_res_config = descriptor.p_res_cfg->GetCopy();
		provider_resource_handle = INVALID_PROVIDER_RESOURCE_HANDLE;
		for ( size_t i = 0; i < port_routings.size(); i++) {
			port_routings[i].channel_index = INVALID_VMX_CHANNEL_INDEX;
		}
		active = false;
		first_routable_channel_index = first_channel_index;
#ifdef DEBUG_RESOURCE_MANAGEMENT
		printf("Created Resource Type %d, Index %d, Num Ports %d, First Channel Index %d, Last Channel Index %d\n",
				int(descriptor.type),
				res_index,
				descriptor.max_num_ports,
				first_channel_index,
				first_channel_index + descriptor.num_channels - 1);
#endif
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

	bool GetRoutableChannelIndexes(VMXChannelIndex& first_routable_channel_index, uint8_t& num_routable_channels) {
		first_routable_channel_index = this->first_routable_channel_index;
		num_routable_channels = descriptor.num_channels;
		return true;
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

	bool RouteChannel(VMXChannelIndex channel_index, VMXResourcePortIndex port_index, VMXErrorCode *errcode = 0) {
		if (!allocated_primary) {
			SET_VMXERROR(errcode, VMXERR_IO_SHARED_RESOURCE_UNROUTABLE);
			return false;
		}
		if (port_index >= descriptor.max_num_ports) {
			SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_PORT_INDEX);
			return false;
		}
		if (channel_index > LAST_VALID_VMX_CHANNEL_INDEX) {
			SET_VMXERROR(errcode, VMXERR_IO_INVALID_CHANNEL_INDEX);
			return false;
		}

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

	bool GetRoutedChannels(std::list<VMXChannelIndex>& routed_channel_list) const {
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

	const VMXResourceConfig *GetDefaultConfig() const {
		return this->p_res_config;
	}
};

#endif /* VMXRESOURCEIMPL_H_ */
