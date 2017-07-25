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

int main(int argc, char *argv[])
{
	uint8_t update_rate_hz = 50;
	VMXPiClient vmx(update_rate_hz);
	try {
		if(vmx.is_open()) {
			printf("VMX is open.\n");

			IOCX_CAPABILITY_FLAGS iocx_capabilities;
			if(vmx.iocx.get_capability_flags(iocx_capabilities)) {
				printf("IOCX RPI PWM Header Direction:  %s\n", iocx_capabilities.rpi_gpio_out ? "Output" : "Input");
			} else {
				printf("ERROR acquiring IOCX Capability Flags.\n");
			}

			MISC_CAPABILITY_FLAGS misc_capabilities;
			if(vmx.misc.get_capability_flags(misc_capabilities)) {
				printf("IOCX Analog In Voltage:  %s\n", misc_capabilities.an_in_5V ? "5V" : "3.3V");
			} else {
				printf("ERROR acquring MISC Capability Flags.\n");
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

			/* IOCX test */

			const int first_stm32_gpio = 0;
			const int num_stm32_gpios = vmx.iocx.GetNumGpios();

			const int first_stm32_timer = 0;
			const int num_stm32_timers = vmx.iocx.get_num_timers();
			/* Configure Quad Encoder Inputs (driven by timers 0-3) */
			/* These quad encoders consume to STM32 GPIOs 4-15      */
			const int first_qe_timer = 0;
			const int num_qe_timers = 4;
			for ( int qe_timer = first_qe_timer; qe_timer < first_qe_timer + num_qe_timers; qe_timer++) {
				vmx.iocx.set_timer_config(qe_timer, TIMER_MODE_QUAD_ENCODER);
			}

			/* Configure first 4 STM32 GPIOs as PWM Outputs */
			const int first_pwm_timer = 4;
			const int num_pwm_timers = 2;
			/* Disable PWM timers during configuration */
			for (int pwm_timer = first_pwm_timer; pwm_timer < first_pwm_timer + num_pwm_timers; pwm_timer++ ) {
				vmx.iocx.set_timer_config(pwm_timer, TIMER_MODE_DISABLED);
			}

			for ( int pwm_out_index = 0; pwm_out_index < 4; pwm_out_index++) {
				vmx.iocx.set_gpio_config(pwm_out_index, GPIO_TYPE_AF,
						GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
				// All timers driven off of 48Mhz clock.
				vmx.iocx.set_timer_prescaler(pwm_out_index, 48000); 	/* 1 ms/tick */
				vmx.iocx.set_timer_aar(pwm_out_index, 50); 			/* Frame Period:  50 ticks */
				vmx.iocx.set_timer_chx_ccr(pwm_out_index,0,25);		/* Duty Cycle:  25 ticks */
				vmx.iocx.set_timer_config(4, TIMER_MODE_PWM_OUT);
			}

			/* Re-enable PWM timers now that configuration is complete. */
			for (int pwm_timer = first_pwm_timer; pwm_timer < first_pwm_timer + num_pwm_timers; pwm_timer++ ) {
				vmx.iocx.set_timer_config(pwm_timer, TIMER_MODE_PWM_OUT);
			}

			for ( int i = 0; i < 10; i++) {
				/* Display External Power Voltage */
				float ext_power_voltage = -1.0f;
				if(vmx.misc.get_extpower_voltage(ext_power_voltage)){
					printf("External Power Voltage:  %f\n", ext_power_voltage);
				}
				/* Display Analog Input Values */
				for (int j = 0; j < vmx.get_num_analog_inputs(); j++){
					float an_in_voltage = -1.0f;
					if(vmx.misc.get_accumulator_avg_voltage(j, an_in_voltage)){
						printf("Analog In %d Voltage:  %0.3f\n", j, an_in_voltage);
					}
				}
				/* Display Quad Encoder Input Counts */
				for ( int qe_timer = first_qe_timer; qe_timer < first_qe_timer + num_qe_timers; qe_timer++) {
					uint16_t counter = 65535;
					if(vmx.iocx.get_timer_counter(qe_timer, counter)){
						printf("QE %d counter:  %d.\n", qe_timer, counter);
						uint8_t timer_status;
						if(vmx.iocx.get_timer_status(qe_timer, timer_status)) {
							IOCX_TIMER_DIRECTION direction = iocx_decode_timer_direction(&timer_status);
							printf("QE %d direction:  %s.\n", qe_timer, (direction == UP) ? "Positive" : "Negative");
						}
					}
				}
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 10000000; /* 10 ms */
				nanosleep(&ts, NULL);
			}

			/* Next, reconfigure all GPIOs as outputs, and toggle them. */
			/* 1) disable the timers which had been enabled previously. */
			for ( int i = first_stm32_timer; i < num_stm32_timers; i++) {
				vmx.iocx.set_timer_config(i, TIMER_MODE_DISABLED);
			}

			/* 2) Configure all GPIOs as outputs. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				vmx.iocx.set_gpio_config(i, GPIO_TYPE_OUTPUT_PUSHPULL, GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
			}

			/* 3) Set all GPIOs high. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				vmx.iocx.set_gpio(i, 1);
			}

			/* 4) Set all GPIOs low. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				vmx.iocx.set_gpio(i, 0);
			}

			/* 5) Reconfigure all GPIOs as outputs. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				vmx.iocx.set_gpio_config(i, GPIO_TYPE_INPUT, GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
			}

			/* 6) Display current input values. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				bool gpio;
				vmx.iocx.get_gpio(i, gpio);
				printf("GPIO Input %d value:  %s.\n", i, gpio ? "High": "Low");
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
			//vmx.test_pigpio_pwm_outputs();
			//vmx.test_pigpio_ext_i2c();
			//vmx.test_pigpio_gpio_inputs(100);
			uint8_t hour, minute, second;
			uint32_t subsecond;
			if (vmx.misc.get_rtc_time(hour,minute,second,subsecond)) {
				printf("RTC Time:  %02d:%02d:%02d:%04d\n", hour, minute, second, subsecond);
			} else {
				printf("Error retrieving RTC time.\n");
			}
			uint8_t weekday, date, month, year;
			if (vmx.misc.get_rtc_date(weekday, date, month, year)) {
				printf("RTC Time:  %02d/%02d/20%02d (Weekday:  %d)\n", month, date, year, weekday);
			} else {
				printf("Error retrieving RTC time.\n");
			}

		    time_t     now = time(0);
		    struct tm  tstruct;
		    tstruct = *localtime(&now);

		    printf("Current Time:  %02d:%02d:%02d\n", tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec);
		    if (vmx.misc.set_rtc_time(tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec)) {
		    	printf("Set VMX RTC Time to current time.\n");
		    }
		    if (vmx.misc.set_rtc_date(tstruct.tm_wday, tstruct.tm_mday, tstruct.tm_mon, tstruct.tm_year-100)) {
		    	printf("Set VMX RTC Date to current date.\n");
		    }
		    for ( int i = 0; i < 10; i++) {
		    	uint64_t curr_sys_time = vmx.clocks.GetCurrentSystemTimeInMicroseconds();
		    	uint64_t curr_sys_time_alt = vmx.clocks.SystemClocks::GetCurrentSystemTimeInMicrosecondsAlt();
		    	uint32_t curr_sys_ticks = vmx.clocks.GetCurrentMicroseconds();
		    	uint64_t curr_sys_ticks_total = vmx.clocks.GetCurrentTotalMicroseconds();
		    	printf("CurrSysTime:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_time, curr_sys_time / 1000000);
		    	printf("CurrSysTimeAlt:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_time_alt, curr_sys_time_alt / 1000000);
		    	printf("CurrMicroseconds:  %u\n", curr_sys_ticks);
		    	printf("CurrTotalMicroseconds:  %" PRIu64 " (Seconds:  %" PRIu64 ")\n", curr_sys_ticks_total, curr_sys_ticks_total / 1000000);
		    	uint64_t divider = 1;
		    	divider <<= 32;
		    	printf("TotalMicrosecondsRemainder:  %" PRIu64 " (High Portion:  %" PRIu64 ")\n", (curr_sys_ticks_total % divider), (curr_sys_ticks_total / divider));
		    }
		} else {
			printf("Error:  Unable to open VMX Client.  Is pigpio functional/running?");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}


