/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h> /* printf() */
#include <time.h>  /* nanosleep() */
#include <inttypes.h>

#include "VMXPiClient.h"

void DisplayVMXError(VMXErrorCode vmxerr) {
	const char *p_err_description = GetVMXErrorString(vmxerr);
	printf("VMXError %d:  %s\n", vmxerr, p_err_description);
}

int main(int argc, char *argv[])
{
	uint8_t update_rate_hz = 50;
	VMXPiClient vmx(update_rate_hz);
	try {
		if(vmx.IsOpen()) {
			printf("VMX is open.\n");

			float full_scale_voltage;
			if(vmx.misc.GetAnalogInputFullScaleVoltage(full_scale_voltage)) {
				printf("Analog Input Voltage:  %0.1f\n", full_scale_voltage);
			} else {
				printf("ERROR acquring Analog Input Voltage.\n");
			}

			/* Wait awhile for AHRS data (acquired in background thread) to accumulate */
			struct timespec ts_delay;
			ts_delay.tv_sec = 0;
			ts_delay.tv_nsec = 50000000;  /* 50 ms */
			nanosleep(&ts_delay, NULL);

			/* AHRS test */

			printf("AHRS Connected:  %s\n", (vmx.ahrs.IsConnected() ? "Yes" : "No"));
			for ( int i = 0; i < 10; i++) {
				printf("Yaw, Pitch, Roll:  %f, %f, %f\n", vmx.ahrs.GetYaw(), vmx.ahrs.GetPitch(), vmx.ahrs.GetRoll());
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 20000000; /* 20 ms */
				nanosleep(&ts, NULL);
			}

			vmx.ahrs.Stop(); /* Stop background AHRS data acquisition thread (during debugging, this can be useful... */

			/* IO test */

			VMXChannelIndex first_iocx_digital_channel;
			uint8_t num_iocx_digital_channels = vmx.io.GetNumChannelsByType(VMXChannelType::IOCX_D, first_iocx_digital_channel);
			printf("IOCX Digital Channel Indexes:  %d - %d\n", first_iocx_digital_channel, first_iocx_digital_channel + num_iocx_digital_channels - 1);
			VMXChannelIndex first_iocx_analog_channel;
			uint8_t num_iocx_analog_channels = vmx.io.GetNumChannelsByType(VMXChannelType::IOCX_A, first_iocx_analog_channel);
			printf("IOCX Analog Channel Indexes:  %d - %d\n", first_iocx_analog_channel, first_iocx_analog_channel + num_iocx_analog_channels - 1);
			VMXChannelIndex first_vmxpi_rpi_pwm_channel;
			uint8_t num_rpi_gpios = vmx.io.GetNumChannelsByType(VMXChannelType::PIGPIO, first_vmxpi_rpi_pwm_channel);
			printf("PIGPIO Channel Indexes:  %d - %d\n", first_vmxpi_rpi_pwm_channel, first_vmxpi_rpi_pwm_channel + num_rpi_gpios - 1);

			VMXChannelType channel_type;
			VMXChannelCapability capability_bits;
			if(vmx.io.GetChannelCapabilities(first_vmxpi_rpi_pwm_channel, channel_type, capability_bits)) {
				bool output = VMXChannelCapabilityCheck(capability_bits, VMXChannelCapability::DigitalOutput);
				printf("IOCX RPI PWM Header Direction:  %s\n", output ? "Output" : "Input");
			} else {
				printf("ERROR acquiring Channel Capability Flags for VMX-pi channel %d.\n", first_vmxpi_rpi_pwm_channel);
			}

			const VMXResourceIndex first_stm32_timer = 0;
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

				printf("Encoder ResourceHandle Type:  %d, Index %d\n",
						EXTRACT_VMX_RESOURCE_TYPE(encoder_res_handle),
						EXTRACT_VMX_RESOURCE_INDEX(encoder_res_handle));

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
					printf("Successfully Activated Encoder Resource %d with VMXChannels %d and %d\n", encoder_index, enc_channels[0], enc_channels[1]);
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

				printf("PWM Generator ResourceHandle Type:  %d, Index %d\n",
						EXTRACT_VMX_RESOURCE_TYPE(pwmgen_res_handle),
						EXTRACT_VMX_RESOURCE_INDEX(pwmgen_res_handle));

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
					printf("Successfully Activated PWMGenerator Resource %d with VMXChannels %d and %d\n", pwmgen_index, pwm_channels[0], pwm_channels[1]);
					for (uint8_t port_index = 0; port_index < 2; port_index++) {
						if (!vmx.io.PWMGenerator_SetDutyCycle(pwmgen_res_handle, port_index, 128, &vmxerr)) {
							printf("Failed to set DutyCycle for PWMGenerator Resource %d, Port %d.\n", pwmgen_index, port_index);
							DisplayVMXError(vmxerr);
						}
					}
				}
			}

			for ( int i = 0; i < 10; i++) {
				/* Display System (Battery) Voltage */
				float system_voltage = -1.0f;
				if(vmx.power.GetSystemVoltage(system_voltage)){
					printf("System (Battery) Voltage:  %f\n", system_voltage);
				}
				/* Display Analog Input Values */
				VMXChannelIndex first_analog_input_channel_index;
				uint8_t num_analog_inputs = vmx.io.GetNumChannelsByCapability(VMXChannelCapability::AccumulatorInput);
				for (int j = 0; j < num_analog_inputs; j++){
					float an_in_voltage = -1.0f;
					if(vmx.misc.get_accumulator_avg_voltage(j, an_in_voltage)){
						printf("Analog In %d Voltage:  %0.3f\n", j, an_in_voltage);
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
						printf("Encoder %d count:  %d.\n", encoder_index, counter);
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
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 10000000; /* 10 ms */
				nanosleep(&ts, NULL);
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
					printf("Digital Output Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(gpio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(gpio_res_handles[dio_channel_index]));

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

			/* 5) Reconfigure all GPIOs as outputs. */
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
					printf("Digital Input Channel %d activated on Resource type %d, index %d\n", dio_channel_index,
							EXTRACT_VMX_RESOURCE_TYPE(gpio_res_handles[dio_channel_index]),
							EXTRACT_VMX_RESOURCE_INDEX(gpio_res_handles[dio_channel_index]));

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
			vmx.can.reset(); /* Note:  may block for 3-4ms */
			/* Wait 10ms after resetting CAN transceiver/controller */
			struct timespec ts_reset;
			ts_reset.tv_sec = 0;
			ts_reset.tv_nsec = 10000000; /* 10ms */
			nanosleep(&ts_reset, NULL);

			vmx.can.set_mode(CAN_MODE::CAN_MODE_CONFIG);
			CAN_ID accept_filter_pdp;
			CAN_ID accept_mask;
			CAN_pack_extended_id(0x08041400, &accept_filter_pdp);
			CAN_pack_extended_id(0x0FFFFFF0, &accept_mask);
			vmx.can.set_rxb0_accept_filter(0, accept_filter_pdp);
			vmx.can.set_rxb0_accept_mask(accept_mask);
			CAN_pack_extended_id(0x08041480, &accept_filter_pdp);
			vmx.can.set_rxb1_accept_filter(0, accept_filter_pdp);
			vmx.can.set_rxb1_accept_mask(accept_mask);
			vmx.can.set_rxb0_filter_mode(CAN_RX_FILTER_EID_ONLY);
			vmx.can.set_rxb1_filter_mode(CAN_RX_FILTER_EID_ONLY);
			//vmx.can.flush_rx_fifo();
			vmx.can.flush_tx_fifo();
			vmx.can.set_mode(CAN_MODE::CAN_MODE_NORMAL);

#if 0
			vmx.can.set_mode(CAN_MODE::CAN_MODE_LOOP);
			for ( int i = 0; i < 10; i++) {
				CAN_TRANSFER tx_data;
				CAN_pack_extended_id(0x8041403,&tx_data.id);
				tx_data.payload.dlc.len = 8;
				tx_data.payload.dlc.rtr = false;
				memcpy(tx_data.payload.buff,"Hello!!",8);
				vmx.can.enqueue_transmit_data(&tx_data);
			}
#endif
			CAN_IFX_INT_FLAGS can_int_flags;
			uint8_t rx_fifo_count;

			/* Allow time for some CAN messages to be received. */

			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 50000000; /* 50ms */
			nanosleep(&ts, NULL);

			bool more_can_data_available = true;
			const int max_msgs_per_transfer = 2; /* This value * 17 is the total byte count. */
			const int limit_msgs_per_transfer = 254 / sizeof(TIMESTAMPED_CAN_TRANSFER); /* Limited by SPI Bus capabilities */
			while (more_can_data_available) {

				CAN_MODE can_mode;
				if(vmx.can.get_mode(can_mode)) {
					printf("Current CAN Mode:  ");
					switch(can_mode) {
					case CAN_MODE::CAN_MODE_LISTEN:
						printf("LISTEN");
						break;
					case CAN_MODE::CAN_MODE_LOOP:
						printf("LOOP");
						break;
					case CAN_MODE::CAN_MODE_NORMAL:
						printf("NORMAL");
						break;
					case CAN_MODE::CAN_MODE_CONFIG:
						printf("CONFIG");
						break;
					case CAN_MODE::CAN_MODE_SLEEP:
						printf("SLEEP");
						break;
					}
					printf("\n");
				} else {
					printf("Error retrieving current CAN Mode.\n");
				}

				CAN_ERROR_FLAGS can_error_flags;
				uint8_t tx_error_count = 0;
				uint8_t rx_error_count = 0;
				if(vmx.can.get_bus_errors(can_error_flags, tx_error_count, rx_error_count)){
					if(can_error_flags.can_bus_warn) {
						printf("CAN Bus Warning.\n");
					}
					if(can_error_flags.can_bus_err_pasv) {
						printf("CAN Bus in Passive mode due to errors.\n");
					}
					if(can_error_flags.can_bux_tx_off) {
						printf("CAN Bus Transmitter Off due to errors.\n");
					}
					if(tx_error_count > 0) {
						printf("CAN Bus Tx Error Count:  %d\n", tx_error_count);
					}
					if(rx_error_count > 0) {
						printf("CAN Bus Rx Error Count:  %d\n", rx_error_count);
					}
				}



				if(vmx.can.get_interrupt_status(can_int_flags, rx_fifo_count)) {
					if(can_int_flags.hw_rx_overflow) {
						hw_rx_overflow_detected = true;
						printf("CAN HW Receive Overflow detected.\n");
					}
					if(can_int_flags.sw_rx_overflow) {
						printf("CAN SW Receive Overflow detected.\n");
					}
					if(can_int_flags.bus_error) {
						printf("CAN Bus Error detected.\n");
					}
					if(can_int_flags.wake) {
						printf("CAN Bus Wake occured.\n");
					}
					if(can_int_flags.message_error) {
						printf("CAN Message Error detected.\n");
					}
					if(rx_fifo_count == 0) {
						more_can_data_available = false;
						break;
					} else {
						printf("%d CAN Packets available.\n", rx_fifo_count);
					}
					int success_receive_count = 0;
					while ( rx_fifo_count > 0 ) {
						int num_to_transfer = rx_fifo_count;
						if (num_to_transfer > max_msgs_per_transfer) {
							num_to_transfer = max_msgs_per_transfer;
						}
						rx_fifo_count -= num_to_transfer;
						TIMESTAMPED_CAN_TRANSFER rx_data[limit_msgs_per_transfer];
						uint8_t remaining_transfer_count;
						if(vmx.can.get_receive_data(&rx_data[0],num_to_transfer, remaining_transfer_count)){
							for ( int i = 0; i < num_to_transfer; i++ ) {
								if (!rx_data[i].transfer.id.sidl.invalid) {
									uint32_t eid;
									bool is_eid;
									if(rx_data[i].transfer.id.sidl.ide) {
										CAN_unpack_extended_id(&rx_data[i].transfer.id, &eid);
										is_eid = true;
									} else {
										CAN_unpack_standard_id(&rx_data[i].transfer.id, &eid);
										is_eid = false;
									}
									printf("%2d:  [%10d] Received %d bytes of CAN data from %s 0x%x:  ",
											success_receive_count++,
											rx_data[i].timestamp_ms,
											rx_data[i].transfer.payload.dlc.len,
											(is_eid ? "EID" : "ID "),
											eid);
									for ( int j = 0; j < rx_data[i].transfer.payload.dlc.len; j++) {
										printf("%x ", rx_data[i].transfer.payload.buff[j]);
									}
									printf("\n");
								} else {
									printf("%2d:  Empty Packet entry.\n", success_receive_count);
								}
							}
							printf("Remaining transfer count:  %d\n", remaining_transfer_count);
						} else {
							printf("Error retrieving received CAN data.\n");
						}
					}
				} else {
					printf("ERROR retrieving CAN Interrupt Status.\n");
					more_can_data_available = false;
				}
			}
			if(hw_rx_overflow_detected) {
				can_int_flags.hw_rx_overflow = true;
				printf("Clearing CAN HW Receive Overflow.\n");
				vmx.can.clear_interrupt_flags(can_int_flags);
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
		    for ( int i = 0; i < 10; i++) {
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
		    }
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


