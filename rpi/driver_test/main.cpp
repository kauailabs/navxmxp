/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h>

#include "DaGamaClient.h"
#include "AHRS.h"
#include <pigpio.h>
#include <unistd.h> /* sleep() */
#include <time.h> /* nanosleep() */
#include <string.h>

/* Forward declaration */
void test_navx_pi_spi();
void test_navx_pi_gpio_outputs();
void test_navx_pi_gpio_inputs();
void test_navx_pi_ext_serial();
void test_navx_pi_ext_spi();
void test_navx_pi_ext_i2c();

/* TODO:  Move the following pin definitions into a shared interface file. */

static unsigned navx_pi_rpi_gpio_to_bcm_pin_map[10] = {
		18,
		17,
		27,
		23,
		22,
		24,
		25,
		5,
		6,
		12
};

int main(int argc, char *argv[])
{
	uint8_t update_rate_hz = 50;
	DaGamaClient client(update_rate_hz);
	try {
		if(client.is_open()) {

			/* Wait awhile for AHRS data (acquired in background thread) to accumulate */
			struct timespec ts_delay;
			ts_delay.tv_sec = 0;
			ts_delay.tv_nsec = 50000000;  /* 50 ms */
			nanosleep(&ts_delay, NULL);

			/* AHRS test */

			printf("AHRS Connected:  %s\n", (client.IsConnected() ? "Yes" : "No"));
			for ( int i = 0; i < 10; i++) {
				printf("Yaw, Pitch, Roll:  %f, %f, %f\n", client.GetYaw(), client.GetPitch(), client.GetRoll());
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 20000000; /* 20 ms */
				nanosleep(&ts, NULL);
			}

			client.Stop(); /* Stop background AHRS data acquisition thread (during debugging, this can be useful... */

			/* IOCX test */

			const int first_stm32_gpio = 0;
			const int num_stm32_gpios = client.get_num_gpios();

			const int first_stm32_timer = 0;
			const int num_stm32_timers = client.get_num_timers();
			/* Configure Quad Encoder Inputs (driven by timers 0-3) */
			/* These quad encoders consume to STM32 GPIOs 4-15      */
			const int first_qe_timer = 0;
			const int num_qe_timers = 4;
			for ( int qe_timer = first_qe_timer; qe_timer < first_qe_timer + num_qe_timers; qe_timer++) {
				client.set_timer_config(qe_timer, TIMER_MODE_QUAD_ENCODER);
			}

			/* Configure first 4 STM32 GPIOs as PWM Outputs */
			const int first_pwm_timer = 4;
			const int num_pwm_timers = 2;
			/* Disable PWM timers during configuration */
			for (int pwm_timer = first_pwm_timer; pwm_timer < first_pwm_timer + num_pwm_timers; pwm_timer++ ) {
				client.set_timer_config(pwm_timer, TIMER_MODE_DISABLED);
			}

			for ( int pwm_out_index = 0; pwm_out_index < 4; pwm_out_index++) {
				client.set_gpio_config(pwm_out_index, GPIO_TYPE_AF,
						GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
				// All timers driven off of 48Mhz clock.
				client.set_timer_prescaler(pwm_out_index, 48000); 	/* 1 ms/tick */
				client.set_timer_aar(pwm_out_index, 50); 			/* Frame Period:  50 ticks */
				client.set_timer_chx_ccr(pwm_out_index,0,25);		/* Duty Cycle:  25 ticks */
				client.set_timer_config(4, TIMER_MODE_PWM_OUT);
			}

			/* Re-enable PWM timers now that configuration is complete. */
			for (int pwm_timer = first_pwm_timer; pwm_timer < first_pwm_timer + num_pwm_timers; pwm_timer++ ) {
				client.set_timer_config(pwm_timer, TIMER_MODE_PWM_OUT);
			}

			for ( int i = 0; i < 10; i++) {
				/* Display External Power Voltage */
				float ext_power_voltage = -1.0f;
				if(client.get_ext_power_voltage(ext_power_voltage)){
					printf("External Power Voltage:  %f\n", ext_power_voltage);
				}
				/* Display Analog Input Values */
				for (int j = 0; j < client.get_num_analog_inputs(); j++){
					float an_in_voltage = -1.0f;
					if(client.get_analog_input_voltage(j, an_in_voltage)){
						printf("Analog In %d Voltage:  %f\n", j, an_in_voltage);
					}
				}
				/* Display Quad Encoder Input Counts */
				for ( int qe_timer = first_qe_timer; qe_timer < first_qe_timer + num_qe_timers; qe_timer++) {
					uint16_t counter = 65535;
					if(client.get_timer_counter(qe_timer, counter)){
						printf("QE %d counter:  %d.\n", qe_timer, counter);
						uint8_t timer_status;
						if(client.get_timer_status(qe_timer, timer_status)) {
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
				client.set_timer_config(i, TIMER_MODE_DISABLED);
			}

			/* 2) Configure all GPIOs as outputs. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				client.set_gpio_config(i, GPIO_TYPE_OUTPUT_PUSHPULL, GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
			}

			/* 3) Set all GPIOs high. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				client.set_gpio(i, 1);
			}

			/* 4) Set all GPIOs low. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				client.set_gpio(i, 0);
			}

			/* 5) Reconfigure all GPIOs as outputs. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				client.set_gpio_config(i, GPIO_TYPE_INPUT, GPIO_INPUT_FLOAT, GPIO_INTERRUPT_DISABLED);
			}

			/* 6) Display current input values. */
			for ( int i = first_stm32_gpio; i < first_stm32_gpio + num_stm32_gpios; i++) {
				bool gpio;
				client.get_gpio(i, gpio);
				printf("GPIO Input %d value:  %s.\n", i, gpio ? "High": "Low");
			}

			bool hw_rx_overflow_detected = false;

			/* It is recommended, but not strictly required to enter CAN_MODE_CONFIG */
			/* whenever modifying acceptance filters or masks.                       */
			client.reset(); /* NEEDS A LONGER TIMEOUT! */
			/* Wait 10ms after resetting CAN transceiver/controller */
			struct timespec ts_reset;
			ts_reset.tv_sec = 0;
			ts_reset.tv_nsec = 10000000; /* 10ms */
			nanosleep(&ts_reset, NULL);

			client.set_mode(CAN_MODE::CAN_MODE_CONFIG);
			CAN_ID accept_filter_pdp;
			CAN_ID accept_mask;
			CAN_pack_extended_id(0x08041400, &accept_filter_pdp);
			CAN_pack_extended_id(0x0FFFFFF0, &accept_mask);
			client.set_rxb0_accept_filter(0, accept_filter_pdp);
			client.set_rxb0_accept_mask(accept_mask);
			CAN_pack_extended_id(0x08041480, &accept_filter_pdp);
			client.set_rxb1_accept_filter(0, accept_filter_pdp);
			client.set_rxb1_accept_mask(accept_mask);
			client.set_rxb0_filter_mode(CAN_RX_FILTER_EID_ONLY);
			client.set_rxb1_filter_mode(CAN_RX_FILTER_EID_ONLY);
			//client.flush_rx_fifo();
			client.flush_tx_fifo();
			client.set_mode(CAN_MODE::CAN_MODE_NORMAL);

#if 0
			client.set_mode(CAN_MODE::CAN_MODE_LOOP);
			for ( int i = 0; i < 10; i++) {
				CAN_TRANSFER tx_data;
				CAN_pack_extended_id(0x8041403,&tx_data.id);
				tx_data.payload.dlc.len = 8;
				tx_data.payload.dlc.rtr = false;
				memcpy(tx_data.payload.buff,"Hello!!",8);
				client.enqueue_transmit_data(&tx_data);
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
				if(client.get_mode(can_mode)) {
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
				if(client.get_bus_errors(can_error_flags, tx_error_count, rx_error_count)){
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



				if(client.get_interrupt_status(can_int_flags, rx_fifo_count)) {
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
						if(client.get_receive_data(&rx_data[0],num_to_transfer, remaining_transfer_count)){
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
				}
			}
			if(hw_rx_overflow_detected) {
				can_int_flags.hw_rx_overflow = true;
				printf("Clearing CAN HW Receive Overflow.\n");
				client.clear_interrupt_flags(can_int_flags);
			}
		//test_navx_pi_spi();
		} else {
			printf("Error:  Unable to open client.  Is pigpio functional/running?");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}

void test_navx_pi_gpio_inputs()
{
	for ( int i = 0; i < 10; i++ ) {
		gpioSetMode(navx_pi_rpi_gpio_to_bcm_pin_map[i], PI_INPUT);
	}

	while(true) {
		for ( int i = 0; i < 10; i++ ) {
			gpioSetPullUpDown(navx_pi_rpi_gpio_to_bcm_pin_map[i], PI_PUD_DOWN); // Sets a pull-down.
		}

		for ( int i = 0; i < 10; i++ ) {
			int on = gpioRead(navx_pi_rpi_gpio_to_bcm_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("\n");
		sleep(1);

		for ( int i = 0; i < 10; i++ ) {
			gpioSetPullUpDown(navx_pi_rpi_gpio_to_bcm_pin_map[i], PI_PUD_UP); // Sets a pull-up.
		}

		for ( int i = 0; i < 10; i++ ) {
			int on = gpioRead(navx_pi_rpi_gpio_to_bcm_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("\n");
		sleep(1);

	}
}

void test_navx_pi_gpio_outputs()
{
	for ( int i = 0; i < 10; i++ ) {
		gpioSetMode(navx_pi_rpi_gpio_to_bcm_pin_map[i], PI_OUTPUT);
	}
	int curr_output = 0;
	while(true) {
		printf("Setting navX-PI RPI GPIO Output %d high.\n", curr_output + 1);
		for ( int i = 0; i < 10; i++ ) {
			gpioWrite(navx_pi_rpi_gpio_to_bcm_pin_map[i],(i == curr_output) ? 1 : 0);
		}
		sleep(2);
		curr_output++;
		if ( curr_output > 9 ) {
			curr_output = 0;
		}
	}
}

void test_navx_pi_spi()
{
   unsigned spi_open_flags = 0;
   /* Mode 0, active low (all channels), reserve GPIO for SPI, 8-bits/word, std 4-wire SPI, MSB first */
   spi_open_flags |= 0x100; /* Auxiliary SPI Device */
   unsigned navxpi_aux_spi_chan = 2;

   unsigned baud = 3900000; /* APB Clock (250MHz) / 64 = 3.9Mhz */ //500000;
   int navxpi_spi_handle = spiOpen(navxpi_aux_spi_chan, baud, spi_open_flags);
   if ( navxpi_spi_handle >= 0 ) {
	   for ( int i = 0; i < 100; i++ )
	   {
		   char read_buf[5];
		   char write_buf[4];
		   write_buf[0] = 0;  /* Bank 0 (IMU) */
		   write_buf[1] = 1;  /* Starting Register Address. */
		   write_buf[2] = 4;  /* #Bytes to Read. */
		   write_buf[3] = 67; /* CRC. */
		   spiWrite(navxpi_spi_handle,write_buf, sizeof(write_buf));
//#define USE_NANOSLEEP_FOR_DELAY

		   /* Delay XXX? microseconds */
		   uint32_t delay_us;
#ifdef USE_NANOSLEEP_FOR_DELAY
		   struct timespec tim, tim2;
		   tim.tv_sec = 0;
		   tim.tv_nsec = 10000;
		   nanosleep(&tim, &tim2);
		   delay_us = tim2.tv_nsec / 1000;
#else
		   delay_us = gpioDelay(20); /* Delays < 100us are implemented as busy wait. */
#endif

		   int read_ret = spiRead(navxpi_spi_handle,read_buf,sizeof(read_buf));
		   if ( read_ret < 0 ) {
			   printf("navxpi read failure %d\n", read_ret);
		   } else {
			   printf("read %d bytes from navxpi:  ", read_ret);
			   for ( size_t j = 0; j < sizeof(read_buf); j++) {
				   if ( j > 0 ) {
					   printf(", ");
				   }
				   printf("%X", read_buf[j]);
			   }
			   printf(" (%d us delay)", delay_us);
			   printf("\n");
		   }
		   sleep(1);
	   }
	   spiClose(navxpi_spi_handle);
   } else {
	   printf("Err %d opening navxpi spi port.\n", navxpi_spi_handle);
   }
}

void test_navx_pi_ext_serial()
{
	int serial_port_handle = serOpen((char *)"/dev/ttyS0", 57600, 0);
	if ( serial_port_handle >= 0 ) {
	   for ( int i = 0; i < 1000; i++ )
	   {
		   char *pData = (char *)"ABCD";
		   int ret = serWrite(serial_port_handle, pData, 4 );
		   if ( ret != 0 ) {
			   printf("Error %d from serWrite.\n", ret);
		   } else {
			   printf("Wrote %s to serial port.\n", pData);
		   }
		   sleep(1);
	   }
	}
}


void test_navx_pi_ext_spi()
{
	   unsigned spi_open_flags = 0;
	   /* Mode 0, primary spi device, active low (all channels), reserve GPIO for SPI, 8-bits/word, std 4-wire SPI, MSB first */
	   unsigned navxpi_ext_spi_chan = 1;

	   //spi_open_flags |= 0x1;
	   //spi_open_flags |= 0x2;
	   //spi_open_flags |= 0x3; /* Enable Mode 3 */

	   unsigned baud = 500000;
	   int navxpi_spi_handle = spiOpen(navxpi_ext_spi_chan, baud, spi_open_flags);
	   if ( navxpi_spi_handle >= 0 ) {
		   for ( int i = 0; i < 1000; i++ )
		   {
			   char read_buf[4];
			   char write_buf[3];
			   write_buf[0] = (char)0xA3;
			   write_buf[1] = (char)0x11;
			   write_buf[2] = (char)0x22;
			   spiWrite(navxpi_spi_handle,write_buf, sizeof(write_buf));

			   int read_ret = spiRead(navxpi_spi_handle,read_buf,sizeof(read_buf));
			   if ( read_ret < 0 ) {
				   printf("navxpi read failure %d\n", read_ret);
			   } else {
				   printf("read %d bytes from navxpi:  ", read_ret);
				   for ( size_t j = 0; j < sizeof(read_buf); j++) {
					   if ( j > 0 ) {
						   printf(", ");
					   }
					   printf("%X", read_buf[j]);
				   }
				   printf("\n");
			   }
			   sleep(1);
		   }
		   spiClose(navxpi_spi_handle);
	   } else {
		   printf("Err %d opening navxpi spi port.\n", navxpi_spi_handle);
	   }

}


void test_navx_pi_ext_i2c()
{
	unsigned i2c_bus = 0; /* 0 = user; 1 = eeprom */
	unsigned eprom_addr_7bit = 0x50;
	unsigned i2c_flags = 0;
	int i2c_device_handle = i2cOpen(i2c_bus, eprom_addr_7bit, i2c_flags);
	if ( i2c_device_handle >= 0 ) {
			for ( int i = 0; i < 100; i++ ) {
			int ret = i2cReadByteData(i2c_device_handle, 0);
			if ( ret >= 0 ) {
				printf("Read byte %d from i2c device.\n", ret);
			} else {
				printf("Error %d reading byte from i2c device.\n", ret);
			}
			sleep(1);
		}
		i2cClose(i2c_device_handle);
	}
}


