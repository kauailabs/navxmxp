/*
 * VMXIO.cpp
 *
 *  Created on: 6 Jul 2017
 *      Author: pi
 */

#include "VMXIO.h"
#include "PIGPIOClient.h"
#include "IOCXClient.h"
#include "MISCClient.h"
#include "VMXResourceManager.h"
#include "VMXChannelManager.h"
#include "Logging.h"
#include <list>
#include <mutex>

static std::mutex handler_mutex;

#include "IPIGPIOInterruptSinks.h"
#define MAX_NUM_INTERRUPT_NOTIFY_HANDLERS 32

class VMXIO_PIGPIOInterruptSink : public IIOInterruptSink
{
	PIGPIOClient& pigpio;
	IOCXClient& iocx;

	typedef struct {
		VMXIO_InterruptHandler interrupt_func;
		void *interrupt_params;
		void Init() {
			interrupt_func = 0;
			interrupt_params = 0;
		}
	} InterruptNotificationInfo;

	InterruptNotificationInfo interrupt_notifications[MAX_NUM_INTERRUPT_NOTIFY_HANDLERS];

public:

	VMXIO_PIGPIOInterruptSink(PIGPIOClient& pigpio_ref, IOCXClient& iocx_ref) :
		pigpio(pigpio_ref),
		iocx(iocx_ref)
	{
		for (size_t i = 0; i < (sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])); i++ ) {
			interrupt_notifications[i].Init();
		}
		pigpio.SetIOInterruptSink(this);
		pigpio.EnableIOCXInterrupt();
		/* Clear any pending interrupts */
		uint16_t iocx_int_status;
		uint16_t iocx_last_interrupt_edges;
		iocx.get_gpio_interrupt_status(iocx_int_status, iocx_last_interrupt_edges);
		iocx.set_gpio_interrupt_status(iocx_int_status);
		/* Mask all IOCX Interrupts */
		iocx.set_interrupt_config(0);
	}

	virtual void IOCXInterrupt(PIGPIOInterruptEdge edge, uint64_t curr_timestamp) {

		/* Request IOCX Interrupt Status, indicating which VMX GPIO was the source
		 * Note:  The act of reading the interrupt status register will automatically
		 * clear all pending interrupts.
		 * It's possible that multiple interrupts will be indicated in the
		 * iocx_int_status bits.  In this case, interrupt funcs can be triggered for each
		 * interrupt received. */

		printf("IOCX Interrupt Received.\n");
		uint16_t iocx_int_status;
		uint16_t iocx_last_interrupt_edges;
		if(iocx.get_gpio_interrupt_status(iocx_int_status, iocx_last_interrupt_edges)) {
			/* Clear interrupt */
			iocx.set_gpio_interrupt_status(iocx_int_status);
			printf("IOCX Interrupt Status:  0x%2x - Last Edges:  0x%2x\n", iocx_int_status, iocx_last_interrupt_edges);
			/* Notify Client of Interrupt(s) */
			for ( int i = 0; i < 16; i++) {
				if(iocx_int_status & 0x0001) {
					if (interrupt_notifications[i].interrupt_func) {
						std::unique_lock<std::mutex> sync(handler_mutex);
						if (interrupt_notifications[i].interrupt_func) {
							InterruptEdgeType edge = ((iocx_last_interrupt_edges & 0x0001) ?
									RISING_EDGE_INTERRUPT : FALLING_EDGE_INTERRUPT);
							interrupt_notifications[i].interrupt_func(i, edge, interrupt_notifications[i].interrupt_params, curr_timestamp);
						}
					}
				}
				iocx_int_status >>= 1;
				if (iocx_int_status == 0) {
					break;
				}
				iocx_last_interrupt_edges >>= 1;
			}
		} else {
			printf("Error retrieving VMX IOCX Interrupt Status in gpio_isr().\n");
		}
	}

	virtual void PIGPIOInterrupt(int gpio_num, PIGPIOInterruptEdge level, uint64_t curr_timestamp) {
		uint8_t vmx_interrupt_number = gpio_num;
		if ( vmx_interrupt_number < MAX_NUM_INTERRUPT_NOTIFY_HANDLERS) {
			if (interrupt_notifications[vmx_interrupt_number].interrupt_func) {
				InterruptEdgeType edge;
				if (level == FALLING_EDGE) {
					edge = FALLING_EDGE_INTERRUPT;
				} else {
					edge = RISING_EDGE_INTERRUPT;
				}
				std::unique_lock<std::mutex> sync(handler_mutex);
				if (interrupt_notifications[vmx_interrupt_number].interrupt_func) {
					interrupt_notifications[vmx_interrupt_number].interrupt_func(vmx_interrupt_number, edge, interrupt_notifications[vmx_interrupt_number].interrupt_params, curr_timestamp);
				}
			}
		}
	}


	bool RegisterIOInterruptHandler(uint8_t index, VMXIO_InterruptHandler handler, void *param)
	{
		if (index >= uint8_t((sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])))) return false;
		std::unique_lock<std::mutex> sync(handler_mutex);
		if (interrupt_notifications[index].interrupt_func) return false;
		if (!handler) return false;
		interrupt_notifications[index].interrupt_func = handler;
		interrupt_notifications[index].interrupt_params = param;
		return true;
	}

	bool DeregisterIOInterruptHandler(uint8_t index)
	{
		if (index >= uint8_t((sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])))) return false;
		std::unique_lock<std::mutex> sync(handler_mutex);
		interrupt_notifications[index].Init();
		return true;
	}

	virtual ~VMXIO_PIGPIOInterruptSink()
	{
		/* Deregister all handlers */
		pigpio.DisableIOCXInterrupt();
		pigpio.SetIOInterruptSink(0);
		for (size_t i = 0; i < (sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])); i++ ) {
			if (interrupt_notifications[i].interrupt_func) {
				DeregisterIOInterruptHandler(i);
				interrupt_notifications[i].interrupt_func = 0;
			}
		}
	}

};

VMXIO::VMXIO(PIGPIOClient& pigpio_ref,
		IOCXClient& iocx_ref,
		MISCClient& misc_ref,
		VMXChannelManager& chan_mgr,
		VMXResourceManager& res_mgr,
		VMXTime& time_ref) :
	pigpio(pigpio_ref),
	iocx(iocx_ref),
	misc(misc_ref),
	chan_mgr(chan_mgr),
	res_mgr(res_mgr),
	time(time_ref)
{
	p_int_sink = new VMXIO_PIGPIOInterruptSink(pigpio_ref, iocx_ref);
}

void VMXIO::ReleaseResources()
{
	if (p_int_sink) {
		delete p_int_sink;
		p_int_sink = 0;
	}
}

VMXIO::~VMXIO() {
	ReleaseResources();
}

uint8_t VMXIO::GetNumResourcesByType(VMXResourceType resource_type)
{
	return res_mgr.GetMaxNumResources(resource_type);
}

uint8_t VMXIO::GetNumChannelsByCapability(VMXChannelCapability channel_capability)
{
	return VMXIO::chan_mgr.GetNumChannelsByCapability(channel_capability);
}

uint8_t VMXIO::GetNumChannelsByType(VMXChannelType channel_type, VMXChannelIndex& first_channel_index)
{
	return VMXChannelManager::GetNumChannelsByType(channel_type, first_channel_index);
}

bool VMXIO::GetResourceHandle(VMXResourceType resource_type, VMXResourceIndex res_index, VMXResourceHandle& resource_handle, VMXErrorCode *errcode)
{
	return VMXResourceManager::GetResourceHandle(resource_type, res_index, resource_handle, errcode);
}

bool VMXIO::GetResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability capability, std::list<VMXResourceHandle>& compatible_res_handles)
{
	return res_mgr.GetResourcesCompatibleWithChannelAndCapability(channel_index, capability, compatible_res_handles);
}

bool VMXIO::GetChannelsCompatibleWithResource(VMXResourceHandle resource_handle, VMXChannelIndex& first_channel_index, uint8_t& num_channels)
{
	return res_mgr.GetChannelsCompatibleWithResource(resource_handle, first_channel_index, num_channels);
}

bool VMXIO::GetUnallocatedResourcesCompatibleWithChannelAndCapability(VMXChannelIndex channel_index, VMXChannelCapability capability, std::list<VMXResourceHandle>& unallocated_compatible_res_handles)
{
	bool success = false;
	std::list<VMXResourceHandle> compatible_resources;
	if (GetResourcesCompatibleWithChannelAndCapability(channel_index, capability, compatible_resources)) {
		for (std::list<VMXResourceHandle>::iterator it = compatible_resources.begin(); it != compatible_resources.end(); ++it) {
			VMXResource *p_vmx_resource = res_mgr.GetVMXResource((*it));
			if (!p_vmx_resource->IsAllocated()) {
				unallocated_compatible_res_handles.push_back((*it));
				success = true;
			}
		}
	}
	return success;
}


bool VMXIO::GetChannelCapabilities(VMXChannelIndex channel_index, VMXChannelType& channel_type, VMXChannelCapability& capability_bits)
{
	capability_bits = chan_mgr.GetChannelCapabilityBits(channel_index);
	if (capability_bits == VMXChannelCapability::None) return false;
	uint8_t type_specific_index;
	if (chan_mgr.GetChannelTypeAndProviderSpecificIndex(channel_index, channel_type, type_specific_index)) {
		if (channel_type == HiCurrDIO) {
			VMXChannelHwOpt vmx_hw_opt = chan_mgr.GetChannelHwOpts(channel_index);
			if (VMXChannelHwOptCheck(vmx_hw_opt, VMXChannelHwOpt::IODirectionSelect)) {
				IOCX_CAPABILITY_FLAGS iocx_caps;
				if (iocx.get_capability_flags(iocx_caps)) {
					if (iocx_caps.rpi_gpio_out) {
						// clear Digital Input and Interrupt Flags
						capability_bits = VMXChannelCapabilityClear(capability_bits, VMXChannelCapability::DigitalInput);
						capability_bits = VMXChannelCapabilityClear(capability_bits, VMXChannelCapability::InterruptInput);
					} else {
						// clear Digital Output & PWM Output Flags
						capability_bits = VMXChannelCapabilityClear(capability_bits, VMXChannelCapability::DigitalOutput);
						capability_bits = VMXChannelCapabilityClear(capability_bits, VMXChannelCapability::PWMGeneratorOutput);
						capability_bits = VMXChannelCapabilityClear(capability_bits, VMXChannelCapability::PWMGeneratorOutput2);
					}
				} else {
					log_warning("Error retrieving IOCX::get_capability_flags() for channel %d.\n", channel_index);
				}
			}
		}
		return true;
	}
	return false;
}

bool VMXIO::ChannelSupportsCapability(VMXChannelIndex channel_index, VMXChannelCapability capability)
{
	VMXChannelType channel_type;
	VMXChannelCapability capability_bits;
	if(GetChannelCapabilities(channel_index, channel_type, capability_bits)) {
		return VMXChannelCapabilityCheck(capability_bits, VMXChannelCapability::DigitalOutput);
	}
	return false;
}

bool VMXIO::IsResourceAllocated(VMXResourceHandle resource, bool& allocated, bool& is_shared, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	allocated = p_vmx_resource->IsAllocated();
	is_shared = p_vmx_resource->IsAllocatedShared();
	return true;
}

bool VMXIO::IsResourceActive(VMXResourceHandle resource, bool &active, VMXErrorCode* errcode)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false; /* Fail:  Invalid Resource Handle */
	}

	active = p_vmx_resource->GetActive();
	return true;
}

bool VMXIO::AllocateResource(VMXResourceHandle resource, VMXErrorCode* errcode)
{
	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (p_vmx_resource->IsAllocated()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_ALREADY_ALLOCATED);
		return false;
	}

	/* Ensure that any other resources (if this is a shared resource) are not already allocated as primary. */
	std::list<VMXResource *> other_shared_resources;
	res_mgr.GetOtherResourcesInSharedResourceGroup(p_vmx_resource, other_shared_resources);
	for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
		if ((*it)->IsAllocated()) {
			SET_VMXERROR(errcode, VMXERR_IO_SHARED_RESOURCE_ALREADY_ALLOCATED);
			return false;
		}
	}

	if (!p_vmx_resource->AllocatePrimary()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_ALLOCATION_ERROR);
		return false;
	}

	for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
		(*it)->AllocateShared();
	}
	return true;
}


bool VMXIO::DeallocateResource(VMXResourceHandle resource, VMXErrorCode *errcode)
{
	bool complete_success = false;
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (p_vmx_resource->IsAllocatedPrimary())
	{
		complete_success = true;
		if (p_vmx_resource->GetActive()) {
			if (!DeactivateResource(resource, errcode)) {
				log_warning("Error deactivating Resource type %d, index %d\n", p_vmx_resource->GetResourceType(), p_vmx_resource->GetResourceIndex());
				if (errcode) {
					log_warning("Error Code %d (%s)\n", *errcode, GetVMXErrorString(*errcode));
				}
				complete_success = false;
			}
		}
		if (!UnrouteAllChannelsFromResource(resource, errcode)) {
			log_warning("Error unrouting all Channels from Resource type %d, index %d\n", p_vmx_resource->GetResourceType(), p_vmx_resource->GetResourceIndex());
			if (errcode) {
				log_warning("Error Code %d (%s)\n", *errcode, GetVMXErrorString(*errcode));
			}
			complete_success = false;
		}
		p_vmx_resource->DeallocatePrimary();

		std::list<VMXResource *> other_shared_resources;
		if(res_mgr.GetOtherResourcesInSharedResourceGroup(p_vmx_resource, other_shared_resources)) {
			for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
				if ((*it)->IsAllocatedShared()) {
					(*it)->DeallocateShared();
				}
			}
		}
	}

	return complete_success;
}

bool VMXIO::DeallocateAllResources(VMXErrorCode *last_errorcode) {
	bool all_resources_deallocated = true;
	std::list<VMXResource *>& all_resources = res_mgr.GetAllResources();
	for (std::list<VMXResource *>::iterator it = all_resources.begin(); it != all_resources.end(); ++it) {
		if ((*it)->IsAllocatedPrimary()) {
			VMXResourceHandle res_handle = (*it)->GetResourceHandle();
			if (!DeallocateResource(res_handle, last_errorcode)) {
				all_resources_deallocated = false;
				log_warning("Error deallocating Resource type %d, index %d\n", (*it)->GetResourceType(), (*it)->GetResourceIndex());
			}
		}
	}
	return all_resources_deallocated;
}

/* Assumes that Resource has already been allocated */
/* Note:  If the resource requires multiple channels to be allocated to the resource, */
/* all channels are routed when the first is routed.  If any resources support optionally */
/* multiple, special case code will be added herein, and commented clearly. */
bool VMXIO::RouteChannelToResource(VMXChannelIndex channel_index, VMXResourceHandle resource, VMXErrorCode* errcode)
{
	/* Retrieve resource the channel may already be routed to. */
	VMXResource *p_existing_routed_resource = res_mgr.GetResourceFromRoutedChannel(channel_index);
	VMXResource *p_vmx_res = res_mgr.GetVMXResource(resource);
	if (p_existing_routed_resource) {
		bool dual_routing_allowed = false;
		if (p_vmx_res) {
			if ((p_existing_routed_resource->GetResourceType() == VMXResourceType::DigitalIO) &&
				(p_vmx_res->GetResourceType() == VMXResourceType::Interrupt)) {
				dual_routing_allowed = true;
			}
			if ((p_vmx_res->GetResourceType() == VMXResourceType::DigitalIO) &&
				(p_existing_routed_resource->GetResourceType() == VMXResourceType::Interrupt)) {
				dual_routing_allowed = true;
			}
		}
		if (!dual_routing_allowed) {
			SET_VMXERROR(errcode, VMXERR_IO_CHANNEL_ALREADY_ROUTED);
			return false; /* Fail:  Channel already routed */
		}
	}

	if (p_vmx_res) {

		VMXChannelCapability compatible_capability = res_mgr.GetCompatibleChannelCapabilityBits(channel_index, p_vmx_res);
		if (compatible_capability == VMXChannelCapability::None) {
			SET_VMXERROR(errcode, VMXERR_IO_CHANNEL_RESOURCE_INCOMPATIBILITY);
			return false;
		}

		if (p_vmx_res->GetNumRoutedChannels() >= p_vmx_res->GetMaxNumPorts()) {
			SET_VMXERROR(errcode, VMXERR_IO_NO_AVAILABLE_RESOURCE_PORTS);
			return false; /* Max Num Channels are already routed */
		}

		VMXChannelType vmx_chan_type;
		uint8_t provider_specific_channel_index;
		VMXResourcePortIndex resource_port_index;
		if (chan_mgr.GetChannelTypeAndProviderSpecificIndex(channel_index, vmx_chan_type, provider_specific_channel_index) &&
			(vmx_chan_type != VMXChannelType::INVALID) &&
			res_mgr.GetResourcePortIndexByChannelCapability(p_vmx_res, compatible_capability, resource_port_index)){
			bool physical_resource_routed = true;
			VMXResourceType res_type = p_vmx_res->GetResourceType();
			VMXResourceProviderType res_prov_type = p_vmx_res->GetResourceProviderType();
			VMXResourceIndex res_index = p_vmx_res->GetResourceIndex();
			if (res_prov_type == VMXResourceProviderType::VMX_PI) {
				IOCX_GPIO_TYPE curr_type;
				IOCX_GPIO_INPUT curr_input;
				IOCX_GPIO_INTERRUPT curr_interrupt;
				if (iocx.get_gpio_config(provider_specific_channel_index, curr_type,
						curr_input, curr_interrupt)) {
					switch(res_type) {

					case VMXResourceType::DigitalIO:
						/* Note:  During resource configuration phase, the input vs. output configuration
						 * will occur; for now, as long as the channel supports the requested io direction,
						 * route the resource as a floating input.
						 */
						physical_resource_routed =
							iocx.set_gpio_config(
								provider_specific_channel_index,
								GPIO_TYPE_INPUT,
								GPIO_INPUT_FLOAT,
								curr_interrupt);
						if (physical_resource_routed) {
							printf("Set VMX_PI Gpio %d to type Input, interrupt %d.\n",
									p_vmx_res->GetProviderResourceIndex(),
									curr_interrupt);
						}
						break;
					case VMXResourceType::Encoder:
					case VMXResourceType::PWMCapture:
					case VMXResourceType::PWMGenerator:
						/* Route this GPIO to the alternate function */
						physical_resource_routed =
							iocx.set_gpio_config(
								provider_specific_channel_index,
								GPIO_TYPE_AF,
								GPIO_INPUT_FLOAT,
								GPIO_INTERRUPT_DISABLED);
						if (res_type == VMXResourceType::PWMGenerator) {
							/* Set the duty cycle for this timer channel to the default (0) [off] */
							iocx.set_timer_chx_ccr(p_vmx_res->GetProviderResourceIndex(), resource_port_index, 0);
						}
						break;
					case VMXResourceType::Interrupt:
						switch(vmx_chan_type) {
						case VMXChannelType::FlexDIO:
							/* Note:  During resource configuration phase, the chosen interrupt
							 * edge will be configured.  For now, assume it is rising-edge only.
							 */
							physical_resource_routed =
								iocx.set_gpio_config(
									provider_specific_channel_index,
									curr_type,
									curr_input,
									GPIO_INTERRUPT_RISING_EDGE);
							break;
						case VMXChannelType::AnalogIn:
							/* Analog Input Channels routing to Interrupt Resources is necessary,
							 * but not sufficient to receive interrupts.  What is also required
							 * is to route the Analog Input Channel to an Accumulator as well.  */
							break;
						default:
							break;
						}
						break;
					default:
						break;
					}
				}
			}
			if (physical_resource_routed) {
				VMXErrorCode routing_errcode;
				if (!p_vmx_res->RouteChannel(channel_index, resource_port_index, &routing_errcode)) {
					log_debug("Error routing channel %d to Port Index %d for Resource Type %d, Resource Index %d\n",
							channel_index,
							resource_port_index,
							res_type,
							res_index);
					SET_VMXERROR(errcode, routing_errcode);
					return false;
				}
				return true;
			} else {
				SET_VMXERROR(errcode, VMXERR_IO_PHYSICAL_ROUTING_ERROR);
				return false;
			}
		} else {
			SET_VMXERROR(errcode, VMXERR_IO_INVALID_CHANNEL_INDEX);
			return false;
		}
	} else {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}
	return false;
}

bool VMXIO::DisconnectAnalogInputInterrupt(uint8_t analog_trigger_num)
{
	uint16_t curr_interrupt_cfg;
	if (iocx.get_interrupt_config(curr_interrupt_cfg)) {
		uint16_t analog_trigger_int_bit = ANALOG_TRIGGER_NUMBER_TO_INT_BIT(analog_trigger_num);
		if (curr_interrupt_cfg & analog_trigger_int_bit) {
			curr_interrupt_cfg &= ~analog_trigger_int_bit;
			return iocx.set_interrupt_config(curr_interrupt_cfg);
		}
	}
	return true;
}

bool VMXIO::UnrouteChannelFromResource(VMXChannelIndex channel_index, VMXResourceHandle resource, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!p_vmx_resource->IsChannelRoutedToResource(channel_index)) {
		SET_VMXERROR(errcode, VMXERR_IO_CHANNEL_NOT_ROUTED_TO_RESOURCE);
		return false;
	}

	/* Todo???:  If the resource is active, only allow unrouting down to the minimum, otherwise fail??? */

	VMXChannelType vmx_chan_type;
	uint8_t provider_specific_channel_index;
	bool physical_channel_unrouted = true;
	if (chan_mgr.GetChannelTypeAndProviderSpecificIndex(channel_index, vmx_chan_type, provider_specific_channel_index) &&
		(vmx_chan_type != VMXChannelType::INVALID)) {
		VMXResourceType res_type = p_vmx_resource->GetResourceType();
		VMXResourceProviderType res_prov_type = p_vmx_resource->GetResourceProviderType();
		if (res_prov_type == VMXResourceProviderType::VMX_PI) {
			switch(res_type) {

				case VMXResourceType::DigitalIO:
				case VMXResourceType::Encoder:
				case VMXResourceType::PWMCapture:
				case VMXResourceType::PWMGenerator:
					/* Configure this port as a floating input. */
					physical_channel_unrouted =
						iocx.set_gpio_config(
							provider_specific_channel_index,
							GPIO_TYPE_INPUT,
							GPIO_INPUT_FLOAT,
							GPIO_INTERRUPT_DISABLED);
					break;
				case VMXResourceType::Interrupt:
					{
						IOCX_GPIO_TYPE curr_type;
						IOCX_GPIO_INPUT curr_input;
						IOCX_GPIO_INTERRUPT curr_interrupt;
						// Leave existing gpio config, but disable interrupt generation */
						switch(vmx_chan_type) {
						case VMXChannelType::FlexDIO:
							if (iocx.get_gpio_config(provider_specific_channel_index, curr_type,
									curr_input, curr_interrupt)) {
								physical_channel_unrouted =
									iocx.set_gpio_config(
										provider_specific_channel_index,
										curr_type,
										curr_input,
										GPIO_INTERRUPT_DISABLED);
							}
							break;
						case VMXChannelType::AnalogIn:
							/* Disconnect the Analog Trigger Interrupt Routing, if any. */
							physical_channel_unrouted = DisconnectAnalogInputInterrupt(provider_specific_channel_index);
							break;
						default:
							break;
						}
					}
					break;

				case VMXResourceType::AnalogTrigger:
					/* Disconnect the Analog Trigger Interrupt Routing */
					physical_channel_unrouted = DisconnectAnalogInputInterrupt(provider_specific_channel_index);
					break;
				default:
					break;
			}
		}
	} else {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_CHANNEL_INDEX);
		return false;
	}

	if(!physical_channel_unrouted) {
		SET_VMXERROR(errcode, VMXERR_IO_PHYSICAL_ROUTING_ERROR);
	}

	if (!p_vmx_resource->UnrouteChannel(channel_index)) {
		SET_VMXERROR(errcode, VMXERR_IO_PHYSICAL_ROUTING_ERROR);
		return false;
	}

	return physical_channel_unrouted;
}

bool VMXIO::UnrouteAllChannelsFromResource(VMXResourceHandle resource, VMXErrorCode *errcode) {
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	/* If any channels are routed to the resource, unroute them. */
	bool all_channels_unrouted = true;
	std::list<VMXChannelIndex> routed_channel_list;
	if (p_vmx_resource->GetRoutedChannels(routed_channel_list)) {
		for (std::list<VMXChannelIndex>::iterator it=routed_channel_list.begin(); it!=routed_channel_list.end(); ++it) {
			VMXChannelIndex routed_channel_index = *it;
			if (!UnrouteChannelFromResource(routed_channel_index, resource, errcode)) {
				log_warning("Error unrouting Channel %d from Resource Type %d Index %d\n",
						routed_channel_index, p_vmx_resource->GetResourceType(), p_vmx_resource->GetResourceIndex());
				all_channels_unrouted = false;
			}
		}
	}
	return all_channels_unrouted;
}

bool VMXIO::SetResourceConfig(VMXResourceHandle resource, const VMXResourceConfig* p_config, VMXErrorCode *errcode)
{
	if (!p_config) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_NULL_PARAMETER);
		return false;
	}

	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	return p_vmx_resource->SetResourceConfig(p_config);
}

bool VMXIO::GetResourceConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config, VMXErrorCode *errcode)
{
	if (!p_config) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_NULL_PARAMETER);
		return false;
	}

	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	return p_vmx_resource->GetResourceConfig(p_config);
}

bool VMXIO::GetResourceDefaultConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config, VMXErrorCode *errcode)
{
	if (!p_config) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_NULL_PARAMETER);
		return false;
	}

	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	return p_vmx_resource->GetResourceConfig(p_config);
}

static const uint32_t us_per_timer_tick = 5;
static const uint32_t us_per_second = 1000000;
static const uint32_t vmx_timer_clock_frequency_mhz = 48000000;
static const uint32_t vmx_timer_clocks_per_tick = us_per_timer_tick * (vmx_timer_clock_frequency_mhz / us_per_second);
static const uint32_t vmx_timer_tick_hz = us_per_second / us_per_timer_tick;

bool VMXIO::ActivateResource(VMXResourceHandle resource, VMXErrorCode* errcode)
{
	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!p_vmx_resource->IsAllocated()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_NOT_ALLOCATED);
		return false;
	}

	if (!p_vmx_resource->ChannelRoutingComplete()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_ROUTING_INCOMPLETE);
		return false;
	}

	VMXResourceType res_type = p_vmx_resource->GetResourceType();
	VMXResourceProviderType res_prov_type = p_vmx_resource->GetResourceProviderType();

	bool configuration_success = false;

	std::list<VMXChannelIndex> routed_channels;
	if (!p_vmx_resource->GetRoutedChannels(routed_channels)) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_ROUTING_INCOMPLETE);
		return false;
	}

	switch(res_type) {

	case VMXResourceType::DigitalIO:
		{
			DIOConfig dio_config;
			p_vmx_resource->GetResourceConfig(&dio_config);
			if (routed_channels.size() > 0) {
				VMXChannelIndex routed_channel = *(routed_channels.begin());
				VMXChannelCapability chan_caps = chan_mgr.GetChannelCapabilityBits(routed_channel);
				switch(res_prov_type) {
				case VMX_PI:
					{
						if (dio_config.GetInput()) {
							if (iocx.set_gpio_config(
								p_vmx_resource->GetProviderResourceIndex(),
								GPIO_TYPE_INPUT,
								((dio_config.GetInputMode() == DIOConfig::InputMode::PULLUP) ? GPIO_INPUT_PULLUP : GPIO_INPUT_PULLDOWN),
								GPIO_INTERRUPT_DISABLED)) {
								printf("Set VMX_PI Gpio %d to type Input, interrupt %d.\n",
										p_vmx_resource->GetProviderResourceIndex(),
										GPIO_INTERRUPT_DISABLED);
								configuration_success = true;
							}
						}
						else {
							if (iocx.set_gpio_config(
								p_vmx_resource->GetProviderResourceIndex(),
								((dio_config.GetOutputMode() == DIOConfig::OutputMode::PUSHPULL) ? GPIO_TYPE_OUTPUT_PUSHPULL : GPIO_TYPE_OUTPUT_OPENDRAIN),
								GPIO_INPUT_FLOAT,
								GPIO_INTERRUPT_DISABLED)) {
								printf("Set VMX_PI Gpio %d to type Output.\n",
										p_vmx_resource->GetProviderResourceIndex());
								configuration_success = true;
							}
						}
					}
					break;

				case RPI:
					if (VMXChannelCapabilityCheck(chan_caps, VMXChannelCapability::DigitalInput) &&
					    dio_config.GetInput()) {
						configuration_success = pigpio.ConfigureGPIOAsInput(p_vmx_resource->GetProviderResourceIndex());
					} else if (VMXChannelCapabilityCheck(chan_caps, VMXChannelCapability::DigitalOutput) &&
						!dio_config.GetInput()) {
						configuration_success = pigpio.ConfigureGPIOAsOutput(p_vmx_resource->GetProviderResourceIndex());
					}
					break;

				}
			}
		}
		break;

	case VMXResourceType::PWMGenerator:
		{
			PWMGeneratorConfig pwm_cfg;
			p_vmx_resource->GetResourceConfig(&pwm_cfg);
			switch(res_prov_type) {
			case VMX_PI:
				{
					int vmx_timer_index = int(p_vmx_resource->GetProviderResourceIndex());
					/* To keep things consistent between VMXPI & RPI, VMXPI PWM Frequency is also set to 5us. */
					/* Disable Timer during configuration */
					uint8_t timer_config;
					iocx_encode_timer_mode(&timer_config, TIMER_MODE_DISABLED);
					configuration_success = iocx.set_timer_config(vmx_timer_index, timer_config);
					configuration_success &= iocx.set_timer_prescaler(vmx_timer_index, vmx_timer_clocks_per_tick);
					uint16_t ticks_per_frame = vmx_timer_tick_hz / pwm_cfg.GetFrequencyHz();
					configuration_success &= iocx.set_timer_aar(vmx_timer_index, ticks_per_frame);
					iocx_encode_timer_mode(&timer_config, TIMER_MODE_PWM_OUT);
					configuration_success &= iocx.set_timer_config(vmx_timer_index, timer_config);
				}
				break;

			case RPI:
				/* RPI only has a set number of possible PWM frequencies, based on sample rate. */
				/* The default sample rate (5us) is used. */
				/* The resulting available frequences are:  8000  4000  2000 1600 1000  800  500  400  320
				250   200   160  100   80   50   40   20   10 */
				configuration_success = pigpio.ConfigurePWMFrequency(p_vmx_resource->GetProviderResourceIndex(), pwm_cfg.GetFrequencyHz());
				break;
			}
		}
		break;

	case VMXResourceType::PWMCapture:
		{
			PWMCaptureConfig pwmcap_cfg;
			p_vmx_resource->GetResourceConfig(&pwmcap_cfg);
			int vmx_timer_index = int(p_vmx_resource->GetProviderResourceIndex());
			/* To keep things consistent between VMXPI & RPI, VMXPI PWM Frequency is also set to 5us. */
			/* Disable Timer during configuration */
			uint8_t timer_config;
			iocx_encode_timer_mode(&timer_config, TIMER_MODE_INPUT_CAPTURE);
			std::list<VMXResourcePortIndex> routed_port_index_list;
			if (p_vmx_resource->GetRoutedResourcePortIndex(routed_port_index_list)) {
				if (routed_port_index_list.size() > 0) {
					VMXResourcePortIndex res_port_index = *(routed_port_index_list.begin());
					if (res_port_index == 0) {
						iocx_encode_input_capture_channel(&timer_config, INPUT_CAPTURE_FROM_CH1);
					} else {
						iocx_encode_input_capture_channel(&timer_config, INPUT_CAPTURE_FROM_CH2);
					}
				}
			}
			if (pwmcap_cfg.GetCaptureEdge() == PWMCaptureConfig::CaptureEdge::RISING) {
				iocx_encode_input_capture_polarity(&timer_config, INPUT_CAPTURE_POLARITY_RISING);
			} else {
				iocx_encode_input_capture_polarity(&timer_config, INPUT_CAPTURE_POLARITY_FALLING);
			}
			configuration_success = iocx.set_timer_config(vmx_timer_index, timer_config);
		}
		break;

	case VMXResourceType::Encoder:
		{
			EncoderConfig encoder_cfg;
			p_vmx_resource->GetResourceConfig(&encoder_cfg);
			int vmx_timer_index = int(p_vmx_resource->GetProviderResourceIndex());
			uint8_t timer_config;
			iocx_encode_timer_mode(&timer_config, TIMER_MODE_QUAD_ENCODER);
			switch(encoder_cfg.GetEncoderEdge()) {
			case EncoderConfig::EncoderEdge::x1:
				iocx_encode_quad_encoder_mode(&timer_config, QUAD_ENCODER_MODE_1x);
				break;
			case EncoderConfig::EncoderEdge::x2:
				iocx_encode_quad_encoder_mode(&timer_config, QUAD_ENCODER_MODE_2x);
				break;
			case EncoderConfig::EncoderEdge::x4:
				iocx_encode_quad_encoder_mode(&timer_config, QUAD_ENCODER_MODE_4x);
				break;
			}
			configuration_success = iocx.set_timer_config(vmx_timer_index, timer_config);
		}
		break;

	case VMXResourceType::Accumulator:
		{
			AccumulatorConfig accum_cfg;
			p_vmx_resource->GetResourceConfig(&accum_cfg);

			configuration_success = misc.set_accumulator_average_bits(p_vmx_resource->GetProviderResourceIndex(), accum_cfg.GetNumAverageBits());
			configuration_success &= misc.set_accumulator_oversample_bits(p_vmx_resource->GetProviderResourceIndex(), accum_cfg.GetNumOversampleBits());
		}
		break;

	case VMXResourceType::AnalogTrigger:
		{
			AnalogTriggerConfig antrig_cfg;
			p_vmx_resource->GetResourceConfig(&antrig_cfg);

			configuration_success = misc.set_analog_trigger_threshold_high(p_vmx_resource->GetProviderResourceIndex(), antrig_cfg.GetThresholdHigh());
			configuration_success &= misc.set_analog_trigger_threshold_low(p_vmx_resource->GetProviderResourceIndex(), antrig_cfg.GetThresholdLow());
			ANALOG_TRIGGER_MODE antrig_mode = ANALOG_TRIGGER_DISABLED;
			switch(antrig_cfg.GetMode()) {
			case AnalogTriggerConfig::AnalogTriggerMode::STATE:
				antrig_mode = ANALOG_TRIGGER_MODE_STATE;
				break;
			case AnalogTriggerConfig::AnalogTriggerMode::RISING_EDGE_PULSE:
				antrig_mode = ANALOG_TRIGGER_MODE_RISING_EDGE_PULSE;
				break;
			case AnalogTriggerConfig::AnalogTriggerMode::FALLING_EDGE_PULSE:
				antrig_mode = ANALOG_TRIGGER_MODE_FALLING_EDGE_PULSE;
				break;
			}
			configuration_success &= misc.set_analog_trigger_mode(p_vmx_resource->GetProviderResourceIndex(), antrig_mode);
		}
		break;

	case VMXResourceType::Interrupt:
		{
			InterruptConfig interrupt_cfg;
			p_vmx_resource->GetResourceConfig(&interrupt_cfg);

			p_int_sink->RegisterIOInterruptHandler(p_vmx_resource->GetResourceIndex(), interrupt_cfg.p_handler, interrupt_cfg.p_param);
			switch(res_prov_type) {
			case VMX_PI:
				{
					IOCX_GPIO_TYPE curr_type;
					IOCX_GPIO_INPUT curr_input;
					IOCX_GPIO_INTERRUPT curr_interrupt_cfg;
					IOCX_GPIO_INTERRUPT new_interrupt_cfg;
					switch(interrupt_cfg.GetEdge()) {
					case InterruptConfig::InterruptEdge::RISING:
						new_interrupt_cfg = GPIO_INTERRUPT_RISING_EDGE;
						break;
					case InterruptConfig::InterruptEdge::FALLING:
						new_interrupt_cfg = GPIO_INTERRUPT_FALLING_EDGE;
						break;
					case InterruptConfig::InterruptEdge::BOTH:
						new_interrupt_cfg = GPIO_INTERRUPT_BOTH_EDGES;
						break;
					}
					/* Route the GPIO as an interrupt. */
					if (iocx.get_gpio_config(p_vmx_resource->GetProviderResourceIndex(), curr_type, curr_input, curr_interrupt_cfg)) {
						configuration_success = iocx.set_gpio_config(
							p_vmx_resource->GetProviderResourceIndex(),
							curr_type,
							curr_input,
							new_interrupt_cfg);
						/* Mask in the interrupt */
						uint16_t curr_interrupt_mask;
						configuration_success &= iocx.get_interrupt_config(curr_interrupt_mask);
						if (configuration_success) {
							curr_interrupt_mask |= GPIO_NUMBER_TO_INT_BIT(p_vmx_resource->GetProviderResourceIndex());
							configuration_success = iocx.set_interrupt_config(curr_interrupt_mask);
						}
					}
				}
				break;
			/* Note:  This interrupt will be received at RPI via Internal VMX IOCX Interrupt Signal */

			case RPI:
				{
					PIGPIOClient::PIGPIOIntEdge new_interrupt_cfg;
					switch(interrupt_cfg.GetEdge()) {
					case InterruptConfig::InterruptEdge::RISING:
						new_interrupt_cfg = PIGPIOClient::INT_EDGE_RISING;
						break;
					case InterruptConfig::InterruptEdge::FALLING:
						new_interrupt_cfg = PIGPIOClient::INT_EDGE_FALLING;
						break;
					case InterruptConfig::InterruptEdge::BOTH:
						new_interrupt_cfg = PIGPIOClient::INT_EDGE_BOTH;
						break;
					}
					configuration_success =
							pigpio.EnableGPIOInterrupt(p_vmx_resource->GetProviderResourceIndex(), new_interrupt_cfg);
				}
				break;
			}
			if (!configuration_success) {
				p_int_sink->DeregisterIOInterruptHandler(p_vmx_resource->GetResourceIndex());
			}
		}
		break;

	case VMXResourceType::UART:
		{
			UARTConfig uart_cfg;
			p_vmx_resource->GetResourceConfig(&uart_cfg);

			int uart_handle;
			configuration_success = pigpio.UARTOpen(uart_cfg.baudrate_bps, uart_handle);
			if (configuration_success) {
				p_vmx_resource->SetProviderResourceHandle(uart_handle);
			}
		}
		break;

	case VMXResourceType::SPI:
		{
			SPIConfig spi_cfg;
			p_vmx_resource->GetResourceConfig(&spi_cfg);
			int spi_handle;
			configuration_success =
					pigpio.SPIMasterOpen(
							spi_cfg.GetBitrate(),
							spi_cfg.GetMode(),
							spi_cfg.GetCSActiveLow(),
							spi_cfg.GetMSBFirst(),
							spi_handle);
			if (configuration_success) {
				p_vmx_resource->SetProviderResourceHandle(spi_handle);
			}
		}
		break;

	case VMXResourceType::I2C:
		{
			I2CConfig i2c_cfg;
			p_vmx_resource->GetResourceConfig(&i2c_cfg);
			int i2c_handle;
			configuration_success =
					pigpio.I2CMasterOpen(
							false, /* use hw, not bitbang */
							100000, /* Note:  this is only use if bitbanging */
							i2c_handle);
			if (configuration_success) {
				p_vmx_resource->SetProviderResourceHandle(i2c_handle);
			}
		}
		break;

	default:
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_TYPE_INVALID);
		return false;
	}
	if (!configuration_success) {
		SET_VMXERROR(errcode, VMXERR_IO_ERROR_CONFIGURING_PHYSICAL_RESOURCE);
		return false;
	}
	return true;
}

bool VMXIO::ActivateSinglechannelResource(VMXChannelIndex channel_index, VMXChannelCapability channel_capability,
		VMXResourceHandle& res_handle, const VMXResourceConfig *res_cfg, VMXErrorCode *errcode)
{
	return ActivateMultichannelResource(1, &channel_index, &channel_capability, res_handle, res_cfg, errcode);
}

bool VMXIO::ActivateMultichannelResource(uint8_t num_channels, VMXChannelIndex *p_channel_indexes, VMXChannelCapability *p_channel_capabilities,
		VMXResourceHandle& res_handle, const VMXResourceConfig *res_cfg, VMXErrorCode *errcode)
{
	for (uint8_t i = 0; i < num_channels; i++) {
		if (!IsVMXChannelCapabilityUnitary(p_channel_capabilities[i])) {
			SET_VMXERROR(errcode, VMXERR_IO_UNITARY_CHANNEL_CAP_REQUIRED);
			return false;
		}
	}

	std::list<VMXResourceHandle> compatible_res_handles;
	if (!res_mgr.GetResourcesCompatibleWithChannelsAndCapability(num_channels, p_channel_indexes, p_channel_capabilities, compatible_res_handles, errcode)) {
		log_debug("No compatible resources found for the following %d channels:", num_channels);
		for (uint8_t i = 0; i < num_channels; i++) {
			log_debug("    - Channel %d, Capability %x", p_channel_indexes[i], p_channel_capabilities[i]);
		}
		return false;
	}

	VMXResource* p_compatible_unallocated_resource = 0;
	for (std::list<VMXResourceHandle>::iterator it = compatible_res_handles.begin();
			it != compatible_res_handles.end();
			++it) {
		VMXResourceHandle res_handle = (*it);
		VMXResource *p_vmx_res = res_mgr.GetVMXResource(res_handle);
		if (p_vmx_res) {
			if (!p_vmx_res->IsAllocated()) {
				p_compatible_unallocated_resource = p_vmx_res;
				break;
			} else {
				log_debug("Resource Type %d Index %d is already allocated; Channel(s) cannot be routed to it.",
						p_vmx_res->GetResourceType(),
						p_vmx_res->GetResourceIndex());
			}
		}
	}
	if (!p_compatible_unallocated_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_NO_UNALLOCATED_COMPATIBLE_RESOURCES);
		return false;
	}

	if (!AllocateResource(p_compatible_unallocated_resource->GetResourceHandle(), errcode)) {
		return false;
	}

	for (uint8_t i = 0; i < num_channels; i++) {
		if (!RouteChannelToResource(p_channel_indexes[i], p_compatible_unallocated_resource->GetResourceHandle(), errcode)) {
			for (uint8_t j = 0; j < i; j++) {
				UnrouteChannelFromResource(p_channel_indexes[j], p_compatible_unallocated_resource->GetResourceHandle());
			}
			DeallocateResource(p_compatible_unallocated_resource->GetResourceHandle());
			return false;
		}
	}

	if (!res_cfg) {
		res_cfg = p_compatible_unallocated_resource->GetDefaultConfig();
	}

	if (!SetResourceConfig(p_compatible_unallocated_resource->GetResourceHandle(), res_cfg, errcode)) {
		for (uint8_t i = 0; i < num_channels; i++) {
			UnrouteChannelFromResource(p_channel_indexes[i], p_compatible_unallocated_resource->GetResourceHandle());
		}
		DeallocateResource(p_compatible_unallocated_resource->GetResourceHandle());
		return false;
	}

	if (ActivateResource(p_compatible_unallocated_resource->GetResourceHandle(), errcode)) {
		res_handle = p_compatible_unallocated_resource->GetResourceHandle();
		return true;
	} else {
		for (uint8_t i = 0; i < num_channels; i++) {
			UnrouteChannelFromResource(p_channel_indexes[i], p_compatible_unallocated_resource->GetResourceHandle());
		}
		DeallocateResource(p_compatible_unallocated_resource->GetResourceHandle());
		return false;
	}
}


bool VMXIO::DeactivateResource(VMXResourceHandle resource, VMXErrorCode* errcode)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false; /* Fail:  Invalid Resource Handle */
	}

	if (!p_vmx_resource->IsAllocated()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_NOT_ALLOCATED);
		return false;
	}

	if (!p_vmx_resource->ChannelRoutingComplete()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_ROUTING_INCOMPLETE);
		return false;
	}

	if (!p_vmx_resource->GetActive()) {
		SET_VMXERROR(errcode, VMXERR_IO_RESOURCE_INACTIVE);
		return false;
	}

	VMXResourceType res_type = p_vmx_resource->GetResourceType();
	VMXResourceProviderType res_prov_type = p_vmx_resource->GetResourceProviderType();

	/* Disable any output activity or input electrical activity - but leave channels routed if any. */

	bool configuration_success = false;
	switch(res_type) {

	case VMXResourceType::DigitalIO:
		switch(res_prov_type) {
		case VMX_PI:
			/* Configure GPIO as Floating Input */
			configuration_success = iocx.set_gpio_config(
				p_vmx_resource->GetProviderResourceIndex(),
				GPIO_TYPE_DISABLED,
				GPIO_INPUT_FLOAT,
				GPIO_INTERRUPT_DISABLED);
			break;

		case RPI:
			/* Configure GPIO as Floating Input */
			configuration_success = pigpio.ConfigureGPIOAsInput(p_vmx_resource->GetProviderResourceIndex());
			configuration_success &= pigpio.ConfigureGPIOInputPullUpDown(p_vmx_resource->GetProviderResourceIndex(), false, false);
			break;
		}
		break;

	case VMXResourceType::PWMGenerator:
		switch(res_prov_type) {
		case VMX_PI:
			configuration_success = iocx.set_timer_config(p_vmx_resource->GetProviderResourceIndex(), uint8_t(TIMER_MODE_DISABLED));
			break;

		case RPI:
			/* Configure GPIO as Floating Input */
			configuration_success = pigpio.ConfigureGPIOAsInput(p_vmx_resource->GetProviderResourceIndex());
			configuration_success &= pigpio.ConfigureGPIOInputPullUpDown(p_vmx_resource->GetProviderResourceIndex(), false, false);
			break;
		}
		break;

	case VMXResourceType::PWMCapture:
		configuration_success = iocx.set_timer_config(p_vmx_resource->GetProviderResourceIndex(), uint8_t(TIMER_MODE_DISABLED));
		break;

	case VMXResourceType::Encoder:
		configuration_success = iocx.set_timer_config(p_vmx_resource->GetProviderResourceIndex(), uint8_t(TIMER_MODE_DISABLED));
		break;

	case VMXResourceType::Accumulator:
		/* Todo:  Disable oversampling and averaging? */
		break;

	case VMXResourceType::AnalogTrigger:
		configuration_success = misc.set_analog_trigger_mode(p_vmx_resource->GetProviderResourceIndex(), ANALOG_TRIGGER_DISABLED);
		break;

	case VMXResourceType::Interrupt:
		{
			InterruptConfig interrupt_cfg;
			p_vmx_resource->GetResourceConfig(&interrupt_cfg);

			p_int_sink->DeregisterIOInterruptHandler(p_vmx_resource->GetResourceIndex());
			switch(res_prov_type) {
			case VMX_PI:
			{
				/* Mask out the interrupt */
				uint16_t curr_interrupt_mask;
				configuration_success &= iocx.get_interrupt_config(curr_interrupt_mask);
				if (configuration_success) {
					curr_interrupt_mask &= ~(GPIO_NUMBER_TO_INT_BIT(p_vmx_resource->GetProviderResourceIndex()));
					configuration_success = iocx.set_interrupt_config(curr_interrupt_mask);
				}

				IOCX_GPIO_TYPE curr_type;
				IOCX_GPIO_INPUT curr_input;
				IOCX_GPIO_INTERRUPT curr_interrupt;
				if (iocx.get_gpio_config(p_vmx_resource->GetProviderResourceIndex(), curr_type, curr_input, curr_interrupt)) {
					configuration_success &=
							iocx.set_gpio_config(
									p_vmx_resource->GetProviderResourceIndex(),
									curr_type,
									curr_input,
									GPIO_INTERRUPT_DISABLED);
				}
			}
			break;

			case RPI:
				configuration_success = pigpio.DisableGPIOInterrupt(p_vmx_resource->GetProviderResourceIndex());
				break;
			}
			break;
		}
		break;

	case VMXResourceType::UART:
		configuration_success = pigpio.UARTClose(p_vmx_resource->GetProviderResourceHandle());
		if (configuration_success) {
			p_vmx_resource->SetProviderResourceHandle(INVALID_PROVIDER_RESOURCE_HANDLE);
		}
		break;

	case VMXResourceType::SPI:
		configuration_success = pigpio.SPIMasterClose(p_vmx_resource->GetProviderResourceHandle());
		if (configuration_success) {
			p_vmx_resource->SetProviderResourceHandle(INVALID_PROVIDER_RESOURCE_HANDLE);
		}
		break;

	case VMXResourceType::I2C:
		configuration_success = pigpio.I2CMasterClose(p_vmx_resource->GetProviderResourceHandle());
		if (configuration_success) {
			p_vmx_resource->SetProviderResourceHandle(INVALID_PROVIDER_RESOURCE_HANDLE);
		}
		break;

	default:
		break;
	}

	if (!configuration_success) {
		SET_VMXERROR(errcode, VMXERR_IO_ERROR_CONFIGURING_PHYSICAL_RESOURCE);
	}
	p_vmx_resource->SetActive(false);

	return configuration_success;
}

bool VMXIO::Encoder_GetCount(VMXResourceHandle encoder_res_handle, int32_t& count, VMXErrorCode *errcode) {
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(encoder_res_handle, VMXResourceType::Encoder);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}
	if(!iocx.get_timer_counter(p_vmx_res->GetProviderResourceIndex(), count)){
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXIO::Encoder_GetDirection(VMXResourceHandle encoder_res_handle, EncoderDirection& direction, VMXErrorCode *errcode) {
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(encoder_res_handle, VMXResourceType::Encoder);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}
	uint8_t timer_status;
	if(!iocx.get_timer_status(p_vmx_res->GetProviderResourceIndex(), timer_status)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	IOCX_TIMER_DIRECTION timer_direction = iocx_decode_timer_direction(&timer_status);
	if (timer_direction == UP) {
		direction = EncoderDirection::EncoderForward;
	} else {
		direction = EncoderDirection::EncoderForward;
	}
	return true;
}

bool VMXIO::Encoder_Reset(VMXResourceHandle encoder_res_handle, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(encoder_res_handle, VMXResourceType::Encoder);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}
	if (!iocx.set_timer_control(p_vmx_res->GetProviderResourceIndex(), RESET_REQUEST)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXIO::PWMGenerator_SetDutyCycle(VMXResourceHandle pwmgen_res_handle, VMXResourcePortIndex port_index, uint8_t duty_cycle, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(pwmgen_res_handle, VMXResourceType::PWMGenerator);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	PWMGeneratorConfig pwm_cfg;
	p_vmx_res->GetResourceConfig(&pwm_cfg);
	// The CHx_CCR register is the duty cycle in ticks
	// The duty_cycle is a ratio (from 0-255) within the frequency
	// The configured frequency is in ticks, and each tick is 5us
	uint16_t ticks_per_frame = vmx_timer_tick_hz / pwm_cfg.GetFrequencyHz();

	uint16_t duty_cycle_ticks = (uint16_t)((((uint32_t)ticks_per_frame) * duty_cycle) / (MAX_PWM_GENERATOR_DUTY_CYCLE + 1));

	if (!iocx.set_timer_chx_ccr(p_vmx_res->GetProviderResourceIndex(), port_index, duty_cycle_ticks)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;

}

bool VMXIO::DIO_Get(VMXResourceHandle dio_res_handle, bool& high, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(dio_res_handle, VMXResourceType::DigitalIO);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!iocx.get_gpio(p_vmx_res->GetProviderResourceIndex(), high)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXIO::DIO_Set(VMXResourceHandle dio_res_handle, bool high, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(dio_res_handle, VMXResourceType::DigitalIO);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!iocx.set_gpio(p_vmx_res->GetProviderResourceIndex(), high)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

/* Accumulator */
/* NOTE:  The resolution of Accumulator values is dependent upon the current number of bits */
/* 0 bits:  12-bit resolution, 1 bit:  13-bit resolution, etc. */
/* See the AccumulatorConfig for more information on modifying these bits */
bool VMXIO::Accumulator_GetOversampleValue(VMXResourceHandle accum_res_handle, uint32_t& oversample_value, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(accum_res_handle, VMXResourceType::Accumulator);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!misc.get_accumulator_oversample_value(p_vmx_res->GetProviderResourceIndex(), oversample_value)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXIO::Accumulator_GetAverageValue(VMXResourceHandle accum_res_handle, uint32_t& average_value, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(accum_res_handle, VMXResourceType::Accumulator);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	if (!misc.get_accumulator_input_average_value(p_vmx_res->GetProviderResourceIndex(), average_value)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	return true;
}

bool VMXIO::Accumulator_GetInstantaneousValue(VMXResourceHandle accum_res_handle, uint32_t& average_value, VMXErrorCode *errcode)
{
	return VMXIO::Accumulator_GetAverageValue(accum_res_handle, average_value);
}

bool VMXIO::Accumulator_GetFullScaleVoltage(float& full_scale_voltage, VMXErrorCode *errcode)
{
	return misc.GetAnalogInputFullScaleVoltage(full_scale_voltage);
}

bool VMXIO::Accumulator_GetAverageVoltage(VMXResourceHandle accum_res_handle, float& average_voltage, VMXErrorCode *errcode)
{
	uint32_t average_value;
	if (!Accumulator_GetAverageValue(accum_res_handle, average_value, errcode)) return false;
	float full_scale_voltage;
	if (!Accumulator_GetFullScaleVoltage(full_scale_voltage, errcode)) return false;
	average_voltage = (full_scale_voltage * average_value) / 4096;
	return true;
}

/* Analog Trigger */
bool VMXIO::AnalogTrigger_GetState(VMXResourceHandle antrig_res_handle, AnalogTriggerState& state, VMXErrorCode *errcode)
{
	VMXResource *p_vmx_res = res_mgr.GetVMXResourceAndVerifyType(antrig_res_handle, VMXResourceType::AnalogTrigger);
	if (!p_vmx_res) {
		SET_VMXERROR(errcode, VMXERR_IO_INVALID_RESOURCE_HANDLE);
		return false;
	}

	ANALOG_TRIGGER_STATE trigstate;
	if (!misc.get_analog_trigger_state(p_vmx_res->GetProviderResourceIndex(), trigstate)) {
		SET_VMXERROR(errcode, VMXERR_IO_BOARD_COMM_ERROR);
		return false;
	}
	switch(trigstate) {
	case ANALOG_TRIGGER_LOW:
		state = BelowThreshold;
		break;
	case ANALOG_TRIGGER_HIGH:
		state = AboveThreshold;
		break;
	case ANALOG_TRIGGER_IN_WINDOW:
		state = InWindow;
		break;
	}
	return true;
}

