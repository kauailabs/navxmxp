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

#define DEBUG_RESOURCE_MANAGEMENT
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

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

class AHRSCallback : public ITimestampedDataSubscriber
{
public:
	AHRSCallback() {}
	virtual ~AHRSCallback() {}
    virtual void timestampedDataReceived( long system_timestamp, long sensor_timestamp, AHRSProtocol::AHRSUpdateBase& sensor_data, void * context )
    {
#if 0
    	printf("AHRS Callback Data Received.  SysTimestamp:  %ld, SensorTimestamp:  %ld\n",
    			system_timestamp,
    			sensor_timestamp);
#endif
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
			printf("VMX-pi HAL is open.\n");

			float full_scale_voltage;
			if(vmx.io.Accumulator_GetFullScaleVoltage(full_scale_voltage)) {
				printf("Analog Input Voltage:  %0.1f\n", full_scale_voltage);
			} else {
				printf("ERROR acquiring Analog Input Voltage.\n");
			}

			/* Wait awhile for AHRS data (acquired in background thread) to accumulate */
			vmx.time.DelayMilliseconds(50);

			/* AHRS test */

			printf("AHRS Connected:  %s\n", (vmx.ahrs.IsConnected() ? "Yes" : "No"));
			for ( int i = 0; i < 10; i++) {
				printf("Yaw, Pitch, Roll:  %f, %f, %f\n", vmx.ahrs.GetYaw(), vmx.ahrs.GetPitch(), vmx.ahrs.GetRoll());
				vmx.time.DelayMilliseconds(20);
			}

			//vmx.ahrs.Stop(); /* Stop background AHRS data acquisition thread (during debugging, this can be useful... */

			/*
			 * Display IO Channel Numbers/Types
			 */

			VMXChannelIndex first_flexio_channel_index;
			uint8_t num_flexio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::FlexDIO, first_flexio_channel_index);
			printf("FlexDIO Channel Indexes:  %d - %d\n", first_flexio_channel_index, first_flexio_channel_index + num_flexio_channels - 1);
			VMXChannelIndex first_hicurrdio_channel_index;
			uint8_t num_hicurrdio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::HiCurrDIO, first_hicurrdio_channel_index);
			printf("HiCurrDIO Channel Indexes:  %d - %d\n", first_hicurrdio_channel_index, first_hicurrdio_channel_index + num_hicurrdio_channels - 1);
			VMXChannelIndex first_analogin_channel_index;
			uint8_t num_analogin_channels = vmx.io.GetNumChannelsByType(VMXChannelType::AnalogIn, first_analogin_channel_index);
			printf("Analog Input Channel Indexes:  %d - %d\n", first_analogin_channel_index, first_analogin_channel_index + num_analogin_channels - 1);
			VMXChannelIndex first_comm_dio_channel_index;
			uint8_t num_comm_dio_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommDIO, first_comm_dio_channel_index);
			printf("Comm/DIO Channel Indexes:  %d - %d\n", first_comm_dio_channel_index, first_comm_dio_channel_index + num_comm_dio_channels - 1);
			VMXChannelIndex first_i2c_channel_index;
			uint8_t num_i2c_channels = vmx.io.GetNumChannelsByType(VMXChannelType::CommI2C, first_i2c_channel_index);
			printf("I2C Channel Indexes:  %d - %d\n", first_i2c_channel_index, first_i2c_channel_index + num_i2c_channels - 1);

			/*
			 * Display High-Current IO Channel Direction (Jumper setting)
			 */

			bool supports_output = vmx.io.ChannelSupportsCapability(first_hicurrdio_channel_index, VMXChannelCapability::DigitalOutput);
			printf("IOCX RPI PWM Header Direction:  %s\n", supports_output ? "Output" : "Input");

			/*
			 * CONFIGURE ENCODER RESOURCES
			 */

			const uint8_t num_encoder_resources = vmx.io.GetNumResourcesByType(VMXResourceType::Encoder);

			VMXErrorCode vmxerr;

			/* Configure Quad Encoder Resources */
			/* Two VMXChannels must be routed to each Quad Encoder */
			const VMXResourceIndex first_encoder_index = 0;
			for ( VMXResourceIndex encoder_index = first_encoder_index;
					encoder_index < (first_encoder_index + num_encoder_resources);
					encoder_index++) {
				VMXChannelIndex enc_channels[2];
				enc_channels[0] = first_flexio_channel_index + (encoder_index * 2) + 0;
				enc_channels[1] = first_flexio_channel_index + (encoder_index * 2) + 1;
				VMXChannelCapability encoder_channel_capabilities[2];
				encoder_channel_capabilities[0] = VMXChannelCapability::EncoderAInput;
				encoder_channel_capabilities[1] = VMXChannelCapability::EncoderBInput;

				VMXResourceHandle encoder_res_handle;
				EncoderConfig encoder_cfg(EncoderConfig::EncoderEdge::x4);

				if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(enc_channels), enc_channels, encoder_channel_capabilities, encoder_res_handle, &encoder_cfg, &vmxerr)) {
					printf("Failed to Activate Encoder Resource %d.\n", encoder_index);
					DisplayVMXError(vmxerr);
					vmx.io.DeallocateResource(encoder_res_handle);
					continue;
				} else {
					printf("Successfully Activated Encoder Resource %d with VMXChannels %d and %d\n", encoder_index, enc_channels[0], enc_channels[1]);
				}
			}

			/*
			 * CONFIGURE PWM GENERATOR RESOURCES
			 */

			/* Configure the 4 STM32 GPIOs as PWM Outputs */
			const uint8_t num_pwmgen_resources = 2;

			for (int pwmgen_index = 0;
					pwmgen_index < num_pwmgen_resources;
					pwmgen_index++ ) {

				VMXResourceHandle pwmgen_res_handle;
				VMXChannelIndex pwm_channels[2];
				pwm_channels[0] = (num_encoder_resources * 2) + (pwmgen_index * 2) + 0;
				pwm_channels[1] = (num_encoder_resources * 2) + (pwmgen_index * 2) + 1;
				VMXChannelCapability pwm_channel_capabilities[2];
				pwm_channel_capabilities[0] = VMXChannelCapability::PWMGeneratorOutput;
				pwm_channel_capabilities[1] = VMXChannelCapability::PWMGeneratorOutput2;

				PWMGeneratorConfig pwmgen_cfg(200 /* Frequency in Hz */);

				if (!vmx.io.ActivateMultichannelResource(ARRAY_SIZE(pwm_channels), pwm_channels, pwm_channel_capabilities, pwmgen_res_handle, &pwmgen_cfg, &vmxerr)) {
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

			/*
			 * CONFIGURE ANALOG ACCUMULATOR RESOURCES
			 */

			VMXChannelIndex first_anin_channel;
			uint8_t num_analog_inputs = vmx.io.GetNumChannelsByType(VMXChannelType::AnalogIn, first_anin_channel);
			VMXResourceHandle accumulator_res_handles[4];

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

			/*
			 * DISPLAY CURRENT VALUES FROM System Battery, Encoders, Analog Inputs
			 */

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

			/* Deallocate all previously-allocated resources. */
			/* All previously routed channels are unrounted, and all previously-initialized resource handles are now invalid */
			if (!vmx.io.DeallocateAllResources(&vmxerr)) {
				printf("Error Deallocating all resources.\n");
				DisplayVMXError(vmxerr);
			}

			/*
			 * CONFIGURE Digital IOs
			 */

			VMXResourceHandle digitalio_res_handles[num_flexio_channels];

			/* Configure all DIOs as OUTPUTS. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				DIOConfig dio_config(DIOConfig::OutputMode::PUSHPULL);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalOutput,
						digitalio_res_handles[dio_channel_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Output Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(digitalio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(digitalio_res_handles[dio_channel_index]));
#endif
				}
			}

			/* 3) Set all GPIOs high. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				if (!vmx.io.DIO_Set(digitalio_res_handles[dio_channel_index], true, &vmxerr)) {
					printf("Error Setting DO HIGH for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				}
			}

			/* 4) Set all GPIOs low. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				if (!vmx.io.DIO_Set(digitalio_res_handles[dio_channel_index], false, &vmxerr)) {
					printf("Error Setting DO LOW for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				}
			}

			/* 5) Reconfigure all GPIOs as inputs. */
			/* NOTE: to transition a GPIO from input to output mode, the resource must be first deallocated, then reallocated. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				if (!vmx.io.DeallocateResource(digitalio_res_handles[dio_channel_index])) {
					printf("Error Deallocating Digitial Output Channel %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
					continue;
				}

				DIOConfig dio_config(DIOConfig::InputMode::PULLUP);
				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::DigitalInput,
						digitalio_res_handles[dio_channel_index], &dio_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource DIO for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(digitalio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(digitalio_res_handles[dio_channel_index]));
#endif
				}
			}

			/* 6) Display current input values. */
			for ( int dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				bool high;
				if (!vmx.io.DIO_Get(digitalio_res_handles[dio_channel_index], high, &vmxerr)) {
					printf("Error Getting Digital Input Value for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
					printf("GPIO Input Channel %d value:  %s.\n", dio_channel_index, high ? "High": "Low");
				}
			}

			VMXResourceHandle flexio_interrupt_res_handles[num_flexio_channels];
			/* Configure all DIOs for rising-edge interrupt-handling */
			for ( uint8_t dio_channel_index = first_flexio_channel_index; dio_channel_index < first_flexio_channel_index + num_flexio_channels; dio_channel_index++) {
				VMXResourceIndex int_res_index = dio_channel_index - first_flexio_channel_index;
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(dio_channel_index));

				if (!vmx.io.ActivateSinglechannelResource(dio_channel_index, VMXChannelCapability::InterruptInput,
						flexio_interrupt_res_handles[int_res_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Interrupt Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(flexio_interrupt_res_handles[int_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(flexio_interrupt_res_handles[int_res_index]));
#endif
				}
			}

			VMXResourceHandle highcurrdio_interrupt_res_handles[num_hicurrdio_channels];
			/* Configure all DIOs for rising-edge interrupt-handling */
			VMXResourceIndex int_res_index = 0;
			for ( uint8_t highcurrdio_channel_index = first_hicurrdio_channel_index; highcurrdio_channel_index < first_hicurrdio_channel_index + num_hicurrdio_channels; highcurrdio_channel_index++) {
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(highcurrdio_channel_index));

				if (!vmx.io.ActivateSinglechannelResource(highcurrdio_channel_index, VMXChannelCapability::InterruptInput,
						highcurrdio_interrupt_res_handles[int_res_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Channel index %d.\n", highcurrdio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Interrupt Resource type %d, index %d\n", highcurrdio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(highcurrdio_interrupt_res_handles[int_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(highcurrdio_interrupt_res_handles[int_res_index]));
#endif
				}
				int_res_index++;
			}

			/*
			 * CONFIGURE Interrupts
			 */

			VMXResourceHandle commdio_interrupt_res_handles[num_comm_dio_channels];
			/* Configure all Comm DIOs for rising-edge interrupt-handling */
			VMXResourceIndex commdio_int_res_index = 0;
			for ( uint8_t comm_dio_channel_index = first_comm_dio_channel_index; comm_dio_channel_index < first_comm_dio_channel_index + num_comm_dio_channels; comm_dio_channel_index++) {
				InterruptConfig int_config(InterruptConfig::RISING, VMXIOInterruptHandler, (void *)int(comm_dio_channel_index));

				if (!vmx.io.ActivateSinglechannelResource(comm_dio_channel_index, VMXChannelCapability::InterruptInput,
						commdio_interrupt_res_handles[commdio_int_res_index], &int_config, &vmxerr)) {
					printf("Error Activating Singlechannel Resource Interrupt for Comm DIO Channel index %d.\n", comm_dio_channel_index);
					DisplayVMXError(vmxerr);
				} else {
#ifdef DEBUG_RESOURCE_MANAGEMENT
					printf("Digital Input Channel %d activated on Interrupt Resource type %d, index %d\n", comm_dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(commdio_interrupt_res_handles[commdio_int_res_index]),
							EXTRACT_VMX_RESOURCE_INDEX(commdio_interrupt_res_handles[commdio_int_res_index]));
#endif
				}
				int_res_index++;
			}

			/*
			 * CONFIGURE CAN; receive and display data
			 */


			VMXCANReceiveStreamHandle canrxhandles[3];
			if (!vmx.can.OpenReceiveStream(canrxhandles[0], 0x08041400, 0x0FFFFFF0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 1.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 1, handle:  %d\n", canrxhandles[0]);
			}
			if (!vmx.can.OpenReceiveStream(canrxhandles[1], 0x08041480, 0x0FFFFFF0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 2.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 2, handle:  %d\n", canrxhandles[1]);
			}
			if (!vmx.can.OpenReceiveStream(canrxhandles[2], 0x0, 0x0, 100, &vmxerr)) {
				printf("Error opening CAN RX Stream 2.\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Opened CAN Receive Stream 2, handle:  %d\n", canrxhandles[2]);
			}

			/* Flush Rx/Tx fifo not necessary if invoking reset above. */
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

			vmx.can.DisplayMasksAndFilters(); /* NOTE:  Must be in config mode to display these. */

			if (!vmx.can.SetMode(VMXCAN::VMXCAN_NORMAL)) {
				printf("Error setting CAN Mode to NORMAL\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Set CAN Mode to Normal.\n");
			}

#if 0
			if (!vmx.can.SetMode(VMXCAN::VMXCAN_LOOPBACK)) {
				printf("Error setting CAN Mode to LOOPBACK\n");
				DisplayVMXError(vmxerr);
			} else {
				printf("Set CAN Mode to Loopback.\n");
			}
#endif

			/* It's recommended to delay 20 Milliseconds after transitioning modes -
			 * to allow the CAN circuitry to stabilize; otherwise, sometimes
			 * there will be errors transmitting data during this period.
			 */
			vmx.time.DelayMilliseconds(20);

			for ( int i = 0; i < 10; i++) {
				VMXCANMessage msg;
				msg.messageID = 0x8041403; /* This is an extended ID */
				msg.dataSize = 8;
				memcpy(msg.data,"Hellox!",msg.dataSize);
				msg.data[5] = uint8_t('A') + i;
				if (!vmx.can.SendMessage(msg, 0, &vmxerr)) {
					printf("Error sending CAN message %d\n", i);
					DisplayVMXError(vmxerr);
				}
			}

			/* Allow time for some CAN messages to be received. */
			for (int i = 0; i < 100; i++) {
				vmx.time.DelayMilliseconds(100);
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
					if(can_bus_status.busOffCount > 0) {
						printf("CAN Bus Tx Off Count:  %d\n", can_bus_status.busOffCount);
					}
					if(can_bus_status.txFullCount > 0) {
						printf("CAN Bus Tx Full Count:  %d\n", can_bus_status.txFullCount);
					}
					if(can_bus_status.hwRxOverflow) {
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
			}

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

			for (int i = 0; i < 3; i++) {
				bool done = false;
				while (!done) {
					VMXCANTimestampedMessage stream_msg;
					uint32_t num_msgs_read;
					if (!vmx.can.ReadReceiveStream(canrxhandles[i], &stream_msg, 1, num_msgs_read, &vmxerr)) {
						printf("Error invoking CAN ReadReceiveStream for stream %d.\n", canrxhandles[i]);
						DisplayVMXError(vmxerr);
						done = true;
					}
					if (num_msgs_read == 0) {
						done = true;
					} else {
						bool is_eid = true;
						if (stream_msg.messageID & VMXCAN_IS_FRAME_11BIT) {
							is_eid = false;
							stream_msg.messageID &= ~VMXCAN_IS_FRAME_11BIT;
						}
						stream_msg.messageID &= ~VMXCAN_IS_FRAME_REMOTE;
						printf("[%10d] CAN Stream %d:  %d bytes from %s 0x%x:  ",
								stream_msg.timeStampMS,
								canrxhandles[i],
								stream_msg.dataSize,
								(is_eid ? "EID" : "ID "),
								stream_msg.messageID);
						for ( int j = 0; j < stream_msg.dataSize; j++) {
							printf("%02X ", stream_msg.data[j]);
						}
						printf("\n");
					}
				}
			}

			/*
			 * Acquire Real-Time Clock and OS Clock values
			 */

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
			VMXTime::DaylightSavingsAdjustment dsa;
			if (vmx.time.GetRTCDaylightSavingsAdjustment(dsa, &vmxerr)) {
				if (dsa == VMXTime::DaylightSavingsAdjustment::DSAdjustmentSubtractOneHour) {
					printf("RTC Daylight Savings Adjustment:  Subtract One Hour\n");
				} else if (dsa == VMXTime::DaylightSavingsAdjustment::DSAdjustmentAddOneHour) {
					printf("RTC Daylight Savings Adjustment:  Add One Hour\n");
				} else {
					printf("RTC Daylight Savings Adjustment:  None\n");
				}
			} else {
				printf("Error retrieving RTC Daylight Savings Adjustment.");
				DisplayVMXError(vmxerr);
			}

		    time_t     now = time(0);
		    struct tm  tstruct;
		    tstruct = *localtime(&now);

		    printf("Current Time:  %02d:%02d:%02d - %s\n", tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec,
		    		tstruct.tm_isdst ? "(Daylight Savings)" : "");

		    if (vmx.time.SetRTCTime(tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec)) {
		    	printf("Set VMX RTC Time to current time.\n");
		    }
		    if (vmx.time.SetRTCDate(tstruct.tm_wday, tstruct.tm_mday, tstruct.tm_mon, tstruct.tm_year-100)) {
		    	printf("Set VMX RTC Date to current date.\n");
		    }

			uint64_t curr_os_time = vmx.time.GetCurrentOSTimeMicroseconds();
			uint32_t curr_sys_ticks = vmx.time.GetCurrentMicroseconds();
			uint64_t curr_sys_ticks_total = vmx.time.GetCurrentTotalMicroseconds();
			printf("Current OS Time:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_os_time, curr_os_time / 1000000);
			printf("Current Microseconds:  %u\n", curr_sys_ticks);
			printf("Current TotalMicroseconds:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_ticks_total, curr_sys_ticks_total / 1000000);
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


