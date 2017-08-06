/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h>  /* printf() */
#include <string.h> /* memcpy() */
#include <inttypes.h>

#include "VMXPi.h"

void DisplayVMXError(VMXErrorCode vmxerr) {
	const char *p_err_description = GetVMXErrorString(vmxerr);
	printf("VMXError %d:  %s\n", vmxerr, p_err_description);
}

static void VMXIOInterruptHandler(uint32_t io_interrupt_num,
		InterruptEdgeType edge,
		void* param,
		uint64_t timestamp_us)
{
	const char *edge_type = "unknown";
	switch(edge)
	{
	case InterruptEdgeType::RISING_EDGE_INTERRUPT:
		edge_type = "Rising";
		break;
	case InterruptEdgeType::FALLING_EDGE_INTERRUPT:
		edge_type = "Falling";
		break;
	}
	printf("IO Interrupt Received.  Number:  %d, Edge:  %s, Param:  %x, Timestamp:  %" PRIu64 "\n",
			io_interrupt_num,
			edge_type,
			(unsigned int)param,
			timestamp_us);
}

static void CANNewRxDataNotifyHandler(void *param, uint64_t timestamp_us)
{
	printf("CAN New Rx Data Notiffy Handler.  Param:  %x, Timestamp:  %" PRIu64 "\n",
			(unsigned int)param,
			timestamp_us);
}

class AHRSCallback : public ITimestampedDataSubscriber
{
public:
	AHRSCallback() {}
	virtual ~AHRSCallback() {}
    virtual void timestampedDataReceived( long system_timestamp, long sensor_timestamp, AHRSProtocol::AHRSUpdateBase& sensor_data, void * context )
    {
    	printf("AHRS Callback Data Received.  SysTimestamp:  %ld, SensorTimestamp:  %ld\n",
    			system_timestamp,
    			sensor_timestamp);
    }
};

int main(int argc, char *argv[])
{
	bool realtime = true;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	AHRSCallback ahrs_callback;
	try {
		if(vmx.IsOpen()) {
			vmx.ahrs.RegisterCallback(&ahrs_callback, NULL);
			printf("VMX is open.\n");

			float full_scale_voltage;
			if(vmx.io.Accumulator_GetFullScaleVoltage(full_scale_voltage)) {
				printf("Analog Input Voltage:  %0.1f\n", full_scale_voltage);
			} else {
				printf("ERROR acquring Analog Input Voltage.\n");
			}

			/* Wait awhile for AHRS data (acquired in background thread) to accumulate */
			vmx.time.DelayMilliseconds(50);

			/* AHRS test */

			printf("AHRS Connected:  %s\n", (vmx.ahrs.IsConnected() ? "Yes" : "No"));
			for ( int i = 0; i < 10; i++) {
				printf("Yaw, Pitch, Roll:  %f, %f, %f\n", vmx.ahrs.GetYaw(), vmx.ahrs.GetPitch(), vmx.ahrs.GetRoll());
				vmx.time.DelayMilliseconds(20);
			}

			vmx.ahrs.Stop(); /* Stop background AHRS data acquisition thread (during debugging, this can be useful... */

			/* IO test */

			VMXChannelIndex first_pigpio_channel;
			uint8_t num_rpi_gpios = vmx.io.GetNumChannelsByType(VMXChannelType::PIGPIO, first_pigpio_channel);
#ifdef DEBUG_RESOURCE_MANAGEMENT
			printf("PIGPIO Channel Indexes:  %d - %d\n", first_pigpio_channel, first_pigpio_channel + num_rpi_gpios - 1);
			VMXChannelIndex first_iocx_digital_channel;
			uint8_t num_iocx_digital_channels = vmx.io.GetNumChannelsByType(VMXChannelType::IOCX_D, first_iocx_digital_channel);
			printf("IOCX Digital Channel Indexes:  %d - %d\n", first_iocx_digital_channel, first_iocx_digital_channel + num_iocx_digital_channels - 1);
			VMXChannelIndex first_iocx_analog_channel;
			uint8_t num_iocx_analog_channels = vmx.io.GetNumChannelsByType(VMXChannelType::IOCX_A, first_iocx_analog_channel);
			printf("IOCX Analog Channel Indexes:  %d - %d\n", first_iocx_analog_channel, first_iocx_analog_channel + num_iocx_analog_channels - 1);
#endif

			VMXChannelType channel_type;
			VMXChannelCapability capability_bits;
			if(vmx.io.GetChannelCapabilities(first_pigpio_channel, channel_type, capability_bits)) {
				bool output = VMXChannelCapabilityCheck(capability_bits, VMXChannelCapability::DigitalOutput);
				printf("IOCX RPI PWM Header Direction:  %s\n", output ? "Output" : "Input");
			} else {
				printf("ERROR acquiring Channel Capability Flags for VMX-pi channel %d.\n", first_pigpio_channel);
			}

			const uint8_t num_encoder_resources = vmx.io.GetNumResourcesByType(VMXResourceType::Encoder);

			VMXErrorCode vmxerr;

			/* Configure Quad Encoder Resources */
			/* Two VMXChannels must be routed to each Quad Encoder */
			const VMXResourceIndex first_encoder_index = 0;
			for ( VMXResourceIndex encoder_index = first_encoder_index;
					encoder_index < (first_encoder_index + num_encoder_resources);
					encoder_index++) {

				VMXResourceHandle encoder_res_handle;
				if (!vmx.io.GetResourceHandle(VMXResourceType::Encoder, encoder_index, encoder_res_handle, &vmxerr)) {
					DisplayVMXError(vmxerr);
					continue;
				}

#ifdef DEBUG_RESOURCE_MANAGEMENT
				printf("Encoder ResourceHandle Type:  %d, Index %d\n",
						EXTRACT_VMX_RESOURCE_TYPE(encoder_res_handle),
						EXTRACT_VMX_RESOURCE_INDEX(encoder_res_handle));
#endif

				VMXChannelIndex first;
				uint8_t num_enc_channels;

				if (!vmx.io.GetChannelsCompatibleWithResource(encoder_res_handle, first, num_enc_channels)) {
					printf("Failed to retrieve VMXChannels compatible with Encoder Resource %d.\n", encoder_index);
					continue;
				}

				if (!vmx.io.AllocateResource(encoder_res_handle, &vmxerr)) {
					printf("Failed to allocate Encoder Resource %d.\n", encoder_index);
					DisplayVMXError(vmxerr);
					continue;
				}

				int successful_route_count = 0;
				VMXChannelIndex enc_channels[2];
				for ( uint8_t enc_channel = first; enc_channel < (first + num_enc_channels); enc_channel++) {
					if (!vmx.io.RouteChannelToResource(enc_channel, encoder_res_handle, &vmxerr)) {
						printf("Failed to route VMXChannel %d to Encoder Resource %d.\n", enc_channel, encoder_index);
						DisplayVMXError(vmxerr);
					} else {
						enc_channels[enc_channel - first] = enc_channel;
						successful_route_count++;
					}
				}
				if (successful_route_count < 2) {
					vmx.io.DeallocateResource(encoder_res_handle);
					continue;
				}

				EncoderConfig encoder_cfg;
				encoder_cfg.SetEncoderEdge(EncoderConfig::EncoderEdge::x4);
				if (!vmx.io.SetResourceConfig(encoder_res_handle, &encoder_cfg)) {
					printf("Failed to Set Encoder Config for Encoder Resource %d.\n", encoder_index);
					vmx.io.DeallocateResource(encoder_res_handle);
					continue;
				}
				if (!vmx.io.ActivateResource(encoder_res_handle, &vmxerr)) {
					printf("Failed to Activate Encoder Resource %d.\n", encoder_index);
					DisplayVMXError(vmxerr);
					vmx.io.DeallocateResource(encoder_res_handle);
					continue;
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Successfully Activated Encoder Resource %d with VMXChannels %d and %d\n", encoder_index, enc_channels[0], enc_channels[1]);
#endif
				}
			}

			/* Configure the 4 STM32 GPIOs as PWM Outputs */
			const VMXResourceIndex first_pwmgen_resource_index = 4;
			const uint8_t num_pwmgen_resources = 2;

			for (int pwmgen_index = first_pwmgen_resource_index;
					pwmgen_index < (first_pwmgen_resource_index + num_pwmgen_resources);
					pwmgen_index++ ) {

				VMXResourceHandle pwmgen_res_handle;
				if (!vmx.io.GetResourceHandle(VMXResourceType::PWMGenerator, pwmgen_index, pwmgen_res_handle, &vmxerr)) {
					DisplayVMXError(vmxerr);
					continue;
				}

#ifdef DEBUG_RESOURCE_MANAGEMENT
				printf("PWM Generator ResourceHandle Type:  %d, Index %d\n",
						EXTRACT_VMX_RESOURCE_TYPE(pwmgen_res_handle),
						EXTRACT_VMX_RESOURCE_INDEX(pwmgen_res_handle));
#endif

				VMXChannelIndex first;
				uint8_t num_pwm_channels;

				if (!vmx.io.GetChannelsCompatibleWithResource(pwmgen_res_handle, first, num_pwm_channels)) {
					printf("Failed to retrieve VMXChannels compatible with PWM Generator Resource %d.\n", pwmgen_index);
					continue;
				}

				if (!vmx.io.AllocateResource(pwmgen_res_handle, &vmxerr)) {
					printf("Failed to allocate PWM Generator Resource %d.\n", pwmgen_index);
					DisplayVMXError(vmxerr);
					continue;
				}

				int successful_route_count = 0;
				VMXChannelIndex pwm_channels[2];
				for ( uint8_t pwm_channel = first; pwm_channel < (first + num_pwm_channels); pwm_channel++) {
					if (!vmx.io.RouteChannelToResource(pwm_channel, pwmgen_res_handle, &vmxerr)) {
						printf("Failed to route VMXChannel %d to PWMGenerator Resource %d.\n", pwm_channel, pwmgen_index);
						DisplayVMXError(vmxerr);
					} else {
						pwm_channels[pwm_channel - first] = pwm_channel;
						successful_route_count++;
					}
				}
				if (successful_route_count < 2) {
					vmx.io.DeallocateResource(pwmgen_index);
					continue;
				}

				PWMGeneratorConfig pwmgen_cfg;
				pwmgen_cfg.SetFrequencyHz(200);

				if (!vmx.io.SetResourceConfig(pwmgen_res_handle, &pwmgen_cfg)) {
					printf("Failed to Set PWMGenerator Config for PWMGenerator Resource %d.\n", pwmgen_index);
					vmx.io.DeallocateResource(pwmgen_res_handle);
					continue;
				}
				if (!vmx.io.ActivateResource(pwmgen_res_handle, &vmxerr)) {
					printf("Failed to Activate PWMGenerator Resource %d.\n", pwmgen_index);
					DisplayVMXError(vmxerr);
					vmx.io.DeallocateResource(pwmgen_res_handle);
					continue;
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Successfully Activated PWMGenerator Resource %d with VMXChannels %d and %d\n", pwmgen_index, pwm_channels[0], pwm_channels[1]);
#endif
					for (uint8_t port_index = 0; port_index < 2; port_index++) {
						if (!vmx.io.PWMGenerator_SetDutyCycle(pwmgen_res_handle, port_index, 128, &vmxerr)) {
							printf("Failed to set DutyCycle for PWMGenerator Resource %d, Port %d.\n", pwmgen_index, port_index);
							DisplayVMXError(vmxerr);
						}
					}
				}
			}

			VMXChannelIndex first_anin_channel;
			uint8_t num_analog_inputs = vmx.io.GetNumChannelsByType(VMXChannelType::IOCX_A, first_anin_channel);
			VMXResourceHandle accumulator_res_handles[4];

			/* 2) Configure all DIOs as outputs. */
			for ( uint8_t analog_in_chan_index = first_anin_channel; analog_in_chan_index < first_anin_channel + num_analog_inputs; analog_in_chan_index++) {
				VMXResourceIndex accum_res_index = analog_in_chan_index - first_anin_channel;
				AccumulatorConfig accum_config;
				if (!vmx.io.ActivateSinglechannelResource(analog_in_chan_index, VMXChannelCapability::AccumulatorInput,
						accumulator_res_handles[accum_res_index], &accum_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Accumulator for Channel index %d.\n", analog_in_chan_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Analog Input Channel %d activated on Resource type %d, index %d\n", analog_in_chan_index,
							EXTRACT_VMX_RESOURCE_TYPE(accumulator_res_handles[accum_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(accumulator_res_handles[accum_res_index]));
#endif
				}
			}


			for ( int i = 0; i < 10; i++) {
				/* Display System (Battery) Voltage */
				float system_voltage = -1.0f;
				if(vmx.power.GetSystemVoltage(system_voltage)){
					printf("System (Battery) Voltage:  %f\n", system_voltage);
				}
				/* Display Analog Input Values */
				for (int j = 0; j < num_analog_inputs; j++){
					float an_in_voltage;
					if(vmx.io.Accumulator_GetAverageVoltage(accumulator_res_handles[j], an_in_voltage, &vmxerr)){
						printf("Analog In Channel %d Voltage:  %0.3f\n", j, an_in_voltage);
					} else {
						printf("Error getting Average Voltage of analog accumulator %d\n", j);
						DisplayVMXError(vmxerr);
					}
				}
				/* Display Quad Encoder Input Counts */
				for ( int encoder_index = first_encoder_index; encoder_index < first_encoder_index + num_encoder_resources; encoder_index++) {
					int32_t counter = 65535;
					VMXResourceHandle encoder_res_handle;
					if (!vmx.io.GetResourceHandle(VMXResourceType::Encoder, encoder_index, encoder_res_handle, &vmxerr)) {
						DisplayVMXError(vmxerr);
						continue;
					}

					if (vmx.io.Encoder_GetCount(encoder_res_handle, counter, &vmxerr)) {
						printf("Encoder %d count    :  %d.\n", encoder_index, counter);
						VMXIO::EncoderDirection encoder_direction;
						if(vmx.io.Encoder_GetDirection(encoder_res_handle, encoder_direction, &vmxerr)) {
							printf("Encoder %d direction:  %s.\n", encoder_index, (encoder_direction == VMXIO::EncoderForward) ? "Forward" : "Reverse");
						} else {
							printf("Error retrieving Encoder %d direction.\n", encoder_index);
							DisplayVMXError(vmxerr);
						}
					} else {
						printf("Error retrieving Encoder %d count.\n", encoder_index);
						DisplayVMXError(vmxerr);
					}
				}
				vmx.time.DelayMilliseconds(10);
			}

			/* Deallocate all previously-allocated resources */
			if (!vmx.io.DeallocateAllResources(&vmxerr)) {
				printf("Error Deallocating all resources.\n");
				DisplayVMXError(vmxerr);
			}

			const VMXChannelIndex first_stm32_gpio = 0;
			const uint8_t num_stm32_gpios = 12;
			VMXResourceHandle gpio_res_handles[num_stm32_gpios];

			/* 2) Configure all DIOs as outputs. */
			for ( int dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				DIOConfig dio_config;
				dio_config.SetInput(false);
				dio_config.SetOutputMode(DIOConfig::PUSHPULL);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalOutput,
						gpio_res_handles[dio_channel_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Output Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(gpio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(gpio_res_handles[dio_channel_index]));
#endif
				}
			}

			/* 3) Set all GPIOs high. */
			for ( int dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				if (!vmx.io.DIO_Set(gpio_res_handles[dio_channel_index], true, &vmxerr)) {
					printf("Error Setting DO HIGH for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				}
			}

			/* 4) Set all GPIOs low. */
			for ( int dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				if (!vmx.io.DIO_Set(gpio_res_handles[dio_channel_index], false, &vmxerr)) {
					printf("Error Setting DO LOW for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				}
			}

			/* 5) Reconfigure all GPIOs as inputs. */
			for ( int dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				if (!vmx.io.DeallocateResource(gpio_res_handles[dio_channel_index])) {
					printf("Error Deallocating Digitial Output Channel %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
					continue;
				}
				//vmx.iocx.set_gpio_config(i, GPIO_TYPE_OUTPUT_PUSHPULL, GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
				DIOConfig dio_config;
				dio_config.SetInput(true);
				dio_config.SetInputMode(DIOConfig::PULLUP);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalInput,
						gpio_res_handles[dio_channel_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(gpio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(gpio_res_handles[dio_channel_index]));
#endif
				}
			}

			VMXResourceHandle interrupt_res_handles[num_stm32_gpios];

			/* Configure all DIOs for rising-edge interrupt-handling */
			for ( uint8_t dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				VMXResourceIndex int_res_index = dio_channel_index - first_stm32_gpio;
				InterruptConfig int_config;
				int_config.SetEdge(InterruptConfig::RISING);
				int_config.SetHandler(VMXIOInterruptHandler);
				int_config.SetParam((void *)int(dio_channel_index));

				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::InterruptInput,
						interrupt_res_handles[int_res_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Interrupt Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(interrupt_res_handles[int_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(interrupt_res_handles[int_res_index]));
#endif
				}
			}
			/* 6) Display current input values. */
			for ( int dio_channel_index = first_stm32_gpio; dio_channel_index < first_stm32_gpio + num_stm32_gpios; dio_channel_index++) {
				bool high;
				if (!vmx.io.DIO_Get(gpio_res_handles[dio_channel_index], high, &vmxerr)) {
					printf("Error Getting Digital Input Value for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("GPIO Input Channel %d value:  %s.\n", dio_channel_index, high ? "High": "Low");
				}
			}

			bool hw_rx_overflow_detected = false;

			/* It is recommended, but not strictly required to enter CAN_MODE_CONFIG */
			/* whenever modifying acceptance filters or masks.                       */
			vmx.can.Reset(); /* Note:  may block for 3-4ms */
			/* Wait 10ms after resetting CAN transceiver/controller */
			vmx.time.DelayMilliseconds(10);

			vmx.can.SetMode(VMXCAN::VMXCAN_CONFIG);
			VMXCANReceiveStreamHandle rxh1, rxh2;
			if (!vmx.can.OpenReceiveStream(rxh1, 0x08041400, 0x0FFFFFF0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 1.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 1, handle:  %d\n", rxh1);
			}
			if (!vmx.can.OpenReceiveStream(rxh2, 0x08041480, 0x0FFFFFF0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 2.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 2, handle:  %d\n", rxh2);
			}

			if (!vmx.can.FlushRxFIFO(&vmxerr)) {
				printf("Error Flushing CAN RX FIFO.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Flushed CAN RX FIFO\n");
			}

			if (!vmx.can.FlushTxFIFO(&vmxerr)) {
				printf("Error Flushing CAN TX FIFO.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Flushed CAN TX FIFO\n");
			}

			if (!vmx.can.SetMode(VMXCAN::VMXCAN_NORMAL)) {
				printf("Error setting CAN Mode to NORMAL\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Set CAN Mode to Normal.\n");
			}

			//vmx.can.RegisterNewRxDataNotifyHandler(CANNewRxDataNotifyHandler, NULL);

#if 0
			if (!vmx.can.SetMode(VMXCAN::VMXCAN_LOOPBACK)) {
				printf("Error setting CAN Mode to LOOPBACK\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Set CAN Mode to Normal.\n");
			}

			for ( int i = 0; i < 10; i++) {
				VMXCANMessage msg;
				msg.messageID = 0x8041403; /* This is an extended ID */
				msg.dataSize = 8;
				memcpy(msg.data,"Hello!!",msg.dataSize);
				if (!vmx.can.SendMessage(msg, 0, &vmxerr)) {
					printf("Error sending CAN message %d\n", i);
					DisplayVMXError(vmxerr);
				}
			}
#endif

			/* Allow time for some CAN messages to be received. */
			vmx.time.DelayMilliseconds(50);

			VMXCAN::VMXCANMode can_mode;
			if(vmx.can.GetMode(can_mode)) {
				printf("Current CAN Mode:  ");
				switch(can_mode) {
				case VMXCAN::VMXCAN_LISTEN:
					printf("LISTEN");
					break;
				case VMXCAN::VMXCAN_LOOPBACK:
					printf("LOOPBACK");
					break;
				case VMXCAN::VMXCAN_NORMAL:
					printf("NORMAL");
					break;
				case VMXCAN::VMXCAN_CONFIG:
					printf("CONFIG");
					break;
				case VMXCAN::VMXCAN_OFF:
					printf("OFF (SLEEP)");
					break;
				}
				printf("\n");
			} else {
				printf("Error retrieving current CAN Mode.\n");
			}

			VMXCANBusStatus can_bus_status;
			if (!vmx.can.GetCANBUSStatus(can_bus_status, &vmxerr)) {
				printf("Error getting CAN BUS Status.\n");
				DisplayVMXError(vmxerr);
			} else {
				if(can_bus_status.busWarning) {
					printf("CAN Bus Warning.\n");
				}
				if(can_bus_status.busPassiveError) {
					printf("CAN Bus in Passive mode due to errors.\n");
				}
				if(can_bus_status.busOffError) {
					printf("CAN Bus Transmitter Off due to errors.\n");
				}
				if(can_bus_status.transmitErrorCount > 0) {
					printf("CAN Bus Tx Error Count:  %d\n", can_bus_status.transmitErrorCount);
				}
				if(can_bus_status.receiveErrorCount > 0) {
					printf("CAN Bus Rx Error Count:  %d\n", can_bus_status.receiveErrorCount);
				}
				if(can_bus_status.hwRxOverflow) {
					hw_rx_overflow_detected = true;
					printf("CAN HW Receive Overflow detected.\n");
				}
				if(can_bus_status.swRxOverflow) {
					printf("CAN SW Receive Overflow detected.\n");
				}
				if(can_bus_status.busError) {
					printf("CAN Bus Error detected.\n");
				}
				if(can_bus_status.wake) {
					printf("CAN Bus Wake occured.\n");
				}
				if(can_bus_status.messageError) {
					printf("CAN Message Error detected.\n");
				}
			}

			if (!vmx.can.RetrieveAllCANData(&vmxerr)) {
				printf("Error retrieving all CAN Data.");
				DisplayVMXError(vmxerr);
			} else {
				bool done = false;
				while (!done) {
					VMXCANTimestampedMessage msg;
					uint32_t num_msgs_read;
					if (!vmx.can.ReadReceiveStream(rxh1, &msg, 1, num_msgs_read, &vmxerr)) {
						printf("Error invoking CAN ReadReceiveStream for stream %d.\n", rxh1);
						DisplayVMXError(vmxerr);
						done = true;
					}
					if (num_msgs_read == 0) {
						done = true;
					} else {
						printf("Received CAN Msg from ID 0x%08x [%d bytes]\n", msg.messageID, msg.dataSize);
					}
				}
				done = false;
				while (!done) {
					VMXCANTimestampedMessage msg;
					uint32_t num_msgs_read;
					if (!vmx.can.ReadReceiveStream(rxh2, &msg, 1, num_msgs_read, &vmxerr)) {
						printf("Error invoking CAN ReadReceiveStream for stream %d.\n", rxh2);
						DisplayVMXError(vmxerr);
						done = true;
					}
					if (num_msgs_read == 0) {
						done = true;
					} else {
						printf("Received CAN Msg from ID 0x%08x [%d bytes]\n", msg.messageID, msg.dataSize);
					}
				}
			}

			if(hw_rx_overflow_detected) {
				vmx.can.ClearErrors();
				printf("Clearing CAN HW Receive Overflow.\n");
			}
			//test_navx_pi_spi();
			//vmx.io.TestPWMOutputs();
			//vmx.io.TestExtI2C();
			//vmx.io.TestGPIOInputs(100);
			uint8_t hour, minute, second;
			uint32_t subsecond;
			if (vmx.time.GetRTCTime(hour,minute,second,subsecond)) {
				printf("RTC Time:  %02d:%02d:%02d:%04d\n", hour, minute, second, subsecond);
			} else {
				printf("Error retrieving RTC time.\n");
			}
			uint8_t weekday, date, month, year;
			if (vmx.time.GetRTCDate(weekday, date, month, year)) {
				printf("RTC Date:  %02d/%02d/20%02d (Weekday:  %d)\n", month, date, year, weekday);
			} else {
				printf("Error retrieving RTC date.\n");
			}

		    time_t     now = time(0);
		    struct tm  tstruct;
		    tstruct = *localtime(&now);

		    printf("Current Time:  %02d:%02d:%02d\n", tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec);
		    if (vmx.time.SetRTCTime(tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec)) {
		    	printf("Set VMX RTC Time to current time.\n");
		    }
		    if (vmx.time.SetRTCDate(tstruct.tm_wday, tstruct.tm_mday, tstruct.tm_mon, tstruct.tm_year-100)) {
		    	printf("Set VMX RTC Date to current date.\n");
		    }

			uint64_t curr_sys_time = vmx.time.GetCurrentOSTimeInMicroseconds();
			uint64_t curr_sys_time_alt = vmx.time.GetCurrentOSTimeInMicrosecondsAlt();
			uint32_t curr_sys_ticks = vmx.time.GetCurrentMicroseconds();
			uint64_t curr_sys_ticks_total = vmx.time.GetCurrentTotalMicroseconds();
			printf("CurrSysTime:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_time, curr_sys_time / 1000000);
			printf("CurrSysTimeAlt:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_time_alt, curr_sys_time_alt / 1000000);
			printf("CurrMicroseconds:  %u\n", curr_sys_ticks);
			printf("CurrTotalMicroseconds:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_ticks_total, curr_sys_ticks_total / 1000000);
			uint64_t divider = 1;
			divider <<= 32;
			printf("TotalMicrosecondsRemainder:  %" PRIu64 " (High Portion:  %" PRIu64 ")\n", (curr_sys_ticks_total % divider), (curr_sys_ticks_total / divider));
		} else {
			printf("Error:  Unable to open VMX Client.\n");
			printf("\n");
			printf("        - Is pigpio functional/running?\n");
			printf("        - Does this application have root privileges?\n");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}


