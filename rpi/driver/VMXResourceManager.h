/*
 * VMXResourceManager.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXRESOURCEMANAGER_H_
#define VMXRESOURCEMANAGER_H_

#include "VMXResource.h"
#include <list>
#include <map>

class VMXResourceManager {
protected:
	std::list<VMXResource *> resources;
	std::map<VMXResourceHandle, VMXResource *> resource_handle_to_resource_map;

public:

	VMXResourceManager();
	~VMXResourceManager();

	uint8_t GetMaxNumResources(VMXResourceType vmx_res_type);
	bool GetChannelChapabilitiesByResourceType(VMXResourceType res_type, VMXChannelCapability& channel_capability_bits);
	bool GetResourceTypeAndPortIndexForChannelCapability(VMXChannelCapability channel_capability_bit, VMXResourceType resource_type, VMXResourcePortIndex& resource_port);
	bool GetResourcePortIndexByChannelCapability(VMXResource *p_resource, VMXChannelCapability channel_capability_bit, VMXResourcePortIndex& resource_port );
	bool GetOtherResourcesInSharedResourceGroup(VMXResource *p_resource, std::list<VMXResource *>& other_shared_resources);
	VMXResource *GetResourceFromRoutedChannel(VMXChannelIndex channel); /* NOTE:  Not currently optimized */
	VMXChannelCapability GetCompatibleChannelCapabilityBit(VMXChannelIndex channel_index, VMXResource *p_resource);

	VMXResource *GetVMXResource(VMXResourceHandle vmx_res_handle);
};

#endif /* VMXRESOURCEMANAGER_H_ */
