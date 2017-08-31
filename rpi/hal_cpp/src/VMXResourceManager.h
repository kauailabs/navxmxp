/*
 * VMXResourceManager.h
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#ifndef VMXRESOURCEMANAGER_H_
#define VMXRESOURCEMANAGER_H_

#include "VMXResourceImpl.h"
#include "VMXErrors.h"
#include <list>
#include <map>

class VMXResourceManager {
protected:
	std::list<VMXResource *> resources;
	std::map<VMXResourceHandle, VMXResource *> resource_handle_to_resource_map;

public:

	VMXResourceManager();
	~VMXResourceManager();

	bool GetChannelChapabilitiesByResourceType(VMXResourceType res_type, VMXChannelCapability& channel_capability_bits);
	bool GetPortIndexForResourceTypeAndChannelCapability(VMXChannelCapability channel_capability_bit, VMXResourceType resource_type, VMXResourcePortIndex& resource_port);
	bool GetResourcePortIndexByChannelCapability(VMXResource *p_resource, VMXChannelCapability channel_capability_bit, VMXResourcePortIndex& resource_port );
	bool GetOtherResourcesInSharedResourceGroup(VMXResource *p_resource, std::list<VMXResource *>& other_shared_resources);
	VMXResource *GetResourceFromRoutedChannel(VMXChannelIndex channel); /* NOTE:  Not currently optimized */
	VMXChannelCapability GetCompatibleChannelCapabilityBits(VMXChannelIndex channel_index, VMXResource *p_resource);
	bool GetResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability channel_capability, std::list<VMXResourceHandle>& compatible_res_handles, VMXErrorCode *errcode = 0);
	bool GetChannelsCompatibleWithResource(VMXResourceHandle resource_handle, VMXChannelIndex& first_channel_index, uint8_t& num_channels);

	VMXResource *GetVMXResource(VMXResourceHandle vmx_res_handle);
	VMXResource *GetVMXResourceAndVerifyType(VMXResourceHandle vmx_res_handle, VMXResourceType vmx_res_type);

	static uint8_t GetMaxNumResources(VMXResourceType vmx_res_type);
	static bool GetResourceHandle(VMXResourceType resource_type, VMXResourceIndex res_index, VMXResourceHandle& resource_handle, VMXErrorCode *errcode = 0);
	static bool GetResourceProviderTypeFromChannelType(VMXChannelType chan_type, VMXResourceProviderType& res_provider_type);
	std::list<VMXResource *>& GetAllResources() { return resources; }
};

#endif /* VMXRESOURCEMANAGER_H_ */
