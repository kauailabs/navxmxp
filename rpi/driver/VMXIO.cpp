/*
 * VMXIO.cpp
 *
 *  Created on: 6 Jul 2017
 *      Author: pi
 */

#include "VMXIO.h"

VMXIO::VMXIO(PIGPIOClient& pigpio_ref, IOCXClient& iocx_ref, MISCClient& misc_ref, VMXChannelManager& chan_mgr, VMXResourceManager& res_mgr) :
	pigpio(pigpio_ref),
	iocx(iocx_ref),
	misc(misc_ref),
	chan_mgr(chan_mgr),
	res_mgr(res_mgr)
{
	// TODO Auto-generated constructor stub

}

VMXIO::~VMXIO() {
	// TODO Auto-generated destructor stub
}

bool VMXIO::AllocateResource(VMXResourceHandle resource)
{
	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	if (p_vmx_resource->IsAllocated()) return false;

	/* Ensure that any other resources (if this is a shared resource) are not already allocated as primary. */
	std::list<VMXResource *> other_shared_resources;
	res_mgr.GetOtherResourcesInSharedResourceGroup(p_vmx_resource, other_shared_resources);
	for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
		if ((*it)->IsAllocated()) return false;
	}

	if (!p_vmx_resource->AllocatePrimary()) return false;

	for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
		(*it)->AllocateShared();
	}
	return true;
}


bool VMXIO::DeallocateResource(VMXResourceHandle resource)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	if (!p_vmx_resource->IsAllocatedPrimary()) return false;

	if (p_vmx_resource->GetActive()) {
		if (!DeactivateResource(resource)) return false;
	}

	UnrouteAllChannelsFromResource(resource);

	p_vmx_resource->DeallocatePrimary();

	std::list<VMXResource *> other_shared_resources;
	if(res_mgr.GetOtherResourcesInSharedResourceGroup(p_vmx_resource, other_shared_resources)) {
		for (std::list<VMXResource *>::iterator it=other_shared_resources.begin(); it != other_shared_resources.end(); ++it) {
			if ((*it)->IsAllocatedShared()) {
				(*it)->DeallocateShared();
			}
		}
	}
	return true;
}

/* Assumes that Resource has already been allocated */
/* Note:  If the resource requires multiple channels to be allocated to the resource, */
/* all channels are routed when the first is routed.  If any resources support optionally */
/* multiple, special case code will be added herein, and commented clearly. */
bool VMXIO::RouteChannelToResource(VMXChannelIndex channel_index, VMXResourceHandle resource)
{
	/* Retrieve resource the channel may already be routed to. */
	VMXResource *p_existing_routed_resource = res_mgr.GetResourceFromRoutedChannel(channel_index);
	if (p_existing_routed_resource) return false; /* Fail:  Channel already routed */

	VMXResource *p_vmx_res = res_mgr.GetVMXResource(resource);
	if (p_vmx_res) {

		VMXChannelCapability compatible_capability = res_mgr.GetCompatibleChannelCapabilityBit(channel_index, p_vmx_res);
		if (compatible_capability == VMXChannelCapability::None) return false;

		if (p_vmx_res->GetNumRoutedChannels() >= p_vmx_res->GetMaxNumPorts()) return false; /* Max Num Channels are already routed */

		VMXChannelType vmx_chan_type;
		uint8_t provider_specific_channel_index;
		VMXResourcePortIndex resource_port_index;
		if (chan_mgr.GetChannelTypeAndProviderSpecificIndex(channel_index, vmx_chan_type, provider_specific_channel_index) &&
			(vmx_chan_type != VMXChannelType::INVALID) &&
			res_mgr.GetResourcePortIndexByChannelCapability(p_vmx_res, compatible_capability, resource_port_index)){
			bool physical_resource_routed = true;
			VMXResourceType res_type = p_vmx_res->GetResourceType();
			VMXResourceProviderType res_prov_type = p_vmx_res->GetResourceProviderType();
			if (res_prov_type == VMXResourceProviderType::VMX_PI) {
				IOCX_GPIO_TYPE curr_type;
				IOCX_GPIO_INPUT curr_input;
				IOCX_GPIO_INTERRUPT curr_interrupt;
				if (iocx.get_gpio_config(provider_specific_channel_index, curr_type,
						curr_input, curr_interrupt)) {
					switch(res_type) {

					case VMXResourceType::DigitalIO:
						/* Note:  During resource configuration phase, the input vs. output configuration
						 * will occur; for now, configure this as a floating input.
						 */
						physical_resource_routed =
							iocx.set_gpio_config(
								provider_specific_channel_index,
								GPIO_TYPE_INPUT,
								GPIO_INPUT_FLOAT,
								curr_interrupt);
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
								curr_interrupt);
						if (res_type == VMXResourceType::PWMGenerator) {
							/* Set the duty cycle for this timer channel to the default (0) [off] */
							iocx.set_timer_chx_ccr(p_vmx_res->GetProviderResourceIndex(), resource_port_index, 0);
						}
						break;
					case VMXResourceType::Interrupt:
						switch(vmx_chan_type) {
						case VMXChannelType::IOCX_D:
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
						case VMXChannelType::IOCX_A:
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
				return p_vmx_res->RouteChannel(resource_port_index, channel_index);
			}
		} else {
			return false; /* Invalid ChannelIndex */
		}
	} else {
		return false; /* Invalid ResourceHandle */
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

bool VMXIO::UnrouteChannelFromResource(VMXChannelIndex channel_index, VMXResourceHandle resource)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	if (!p_vmx_resource->IsChannelRoutedToResource(channel_index)) return false;

	/* Todo???:  If the resource is active, only allow unrouting down to the minimum, otherwise fail??? */

	VMXChannelType vmx_chan_type;
	uint8_t provider_specific_channel_index;
	if (chan_mgr.GetChannelTypeAndProviderSpecificIndex(channel_index, vmx_chan_type, provider_specific_channel_index) &&
		(vmx_chan_type != VMXChannelType::INVALID)) {
		bool physical_channel_unrouted = true;
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
						case VMXChannelType::IOCX_D:
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
						case VMXChannelType::IOCX_A:
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

		if(physical_channel_unrouted) {
			if (p_vmx_resource->UnrouteChannel(channel_index)) {
				return true;
			}
		}
	}

	return false;
}

bool VMXIO::UnrouteAllChannelsFromResource(VMXResourceHandle resource) {
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	/* If any channels are routed to the resource, unroute them. */
	std::vector<VMXChannelIndex> routed_channel_list(p_vmx_resource->GetNumRoutedChannels());
	if (p_vmx_resource->GetRoutedChannels(routed_channel_list)) {
		for (size_t i = 0; i < routed_channel_list.size(); i++) {
			VMXChannelIndex routed_channel_index = routed_channel_list[i];
			if (!UnrouteChannelFromResource(routed_channel_index, resource)) return false;
		}
	}
	return true;
}

bool VMXIO::SetResourceConfig(VMXResourceHandle resource, const VMXResourceConfig* p_config)
{
	if (!p_config) return false;

	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	return p_vmx_resource->SetResourceConfig(p_config);
}

bool VMXIO::GetResourceConfig(VMXResourceHandle resource, VMXResourceConfig*& p_config)
{
	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	return p_vmx_resource->GetResourceConfig(p_config);
}

static const uint32_t us_per_timer_tick = 5;
static const uint32_t us_per_second = 1000000;
static const uint32_t vmx_timer_clock_frequency_mhz = 48000000;
static const uint32_t vmx_timer_clocks_per_tick = us_per_timer_tick * (vmx_timer_clock_frequency_mhz / us_per_second);
static const uint32_t vmx_timer_tick_hz = us_per_second / us_per_timer_tick;

bool VMXIO::ActivateResource(VMXResourceHandle resource)
{
	/* Retrieve resource */
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	if (p_vmx_resource->IsAllocated()) return false;

	if (!p_vmx_resource->ChannelRoutingComplete()) return false;

	VMXResourceType res_type = p_vmx_resource->GetResourceType();
	VMXResourceProviderType res_prov_type = p_vmx_resource->GetResourceProviderType();

	bool configuration_success = false;

	switch(res_type) {

	case VMXResourceType::DigitalIO:
		{
			DIOConfig dio_config;
			p_vmx_resource->GetResourceConfig(&dio_config);
			switch(res_prov_type) {
			case VMX_PI:
				{
					IOCX_GPIO_TYPE curr_type;
					IOCX_GPIO_INPUT curr_input;
					IOCX_GPIO_INTERRUPT curr_interrupt;
					if (iocx.get_gpio_config(p_vmx_resource->GetProviderResourceIndex(), curr_type, curr_input, curr_interrupt)) {
						if (iocx.set_gpio_config(
							p_vmx_resource->GetProviderResourceIndex(),
							(dio_config.GetInput() ?
								GPIO_TYPE_INPUT :
								((dio_config.GetOutputMode() == DIOConfig::OutputMode::PUSHPULL) ?
									GPIO_TYPE_OUTPUT_PUSHPULL :
									GPIO_TYPE_OUTPUT_OPENDRAIN)),
							((dio_config.GetInputMode() == DIOConfig::InputMode::PULLUP) ?
									GPIO_INPUT_PULLUP :
									GPIO_INPUT_PULLDOWN),
							curr_interrupt)) {
							configuration_success = true;
						}
					}
				}
				break;

			case RPI:
				if (dio_config.GetInput()) {
					configuration_success = pigpio.ConfigureGPIOAsInput(p_vmx_resource->GetProviderResourceIndex());
				} else {
					configuration_success = pigpio.ConfigureGPIOAsOutput(p_vmx_resource->GetProviderResourceIndex());
				}
				break;

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
			configuration_success &= iocx.set_timer_config(vmx_timer_index, timer_config);
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
			configuration_success &= iocx.set_timer_config(vmx_timer_index, timer_config);
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
				/* Todo:  Configure the interrupt callback function */
				configuration_success =
						pigpio.EnableGPIOInterrupt(p_vmx_resource->GetProviderResourceIndex());
				break;
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
		return false;
	}

	return false;
}

bool VMXIO::DeactivateResource(VMXResourceHandle resource)
{
	VMXResource *p_vmx_resource = res_mgr.GetVMXResource(resource);
	if (!p_vmx_resource) return false; /* Fail:  Invalid Resource Handle */

	if (!p_vmx_resource->IsAllocated()) return false;

	if (!p_vmx_resource->ChannelRoutingComplete()) return false;

	if (!p_vmx_resource->GetActive()) return false;

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
		switch(res_prov_type) {
		case VMX_PI:
		{
			IOCX_GPIO_TYPE curr_type;
			IOCX_GPIO_INPUT curr_input;
			IOCX_GPIO_INTERRUPT curr_interrupt;
			if (iocx.get_gpio_config(p_vmx_resource->GetProviderResourceIndex(), curr_type, curr_input, curr_interrupt)) {
				configuration_success =
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

	if (configuration_success) {
		p_vmx_resource->SetActive(false);
	}

	return configuration_success;
}


