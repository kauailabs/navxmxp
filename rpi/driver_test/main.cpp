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

/* Forward declaration */
void test_navx_pi_spi();
void test_navx_pi_gpio_outputs();
void test_navx_pi_gpio_inputs();
void test_navx_pi_ext_serial();
void test_navx_pi_ext_spi();
void test_navx_pi_ext_i2c();

int main(int argc, char *argv[])
{
	DaGamaClient client;
	uint8_t update_rate_hz = 200;
	AHRS ahrs(&client, update_rate_hz);
	if(client.is_open()) {

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
			float ext_power_voltage;
			if(client.get_ext_power_voltage(ext_power_voltage)){
				printf("External Power Voltage:  %f\n", ext_power_voltage);
			}
			/* Display Analog Input Values */
			for (int j = 0; j < client.get_num_analog_inputs(); j++){
				float an_in_voltage;
				if(client.get_analog_input_voltage(j, an_in_voltage)){
					printf("Analog In %d Voltage:  %f\n", j, an_in_voltage);
				}
			}
			/* Display Quad Encoder Input Counts */
			for ( int qe_timer = first_qe_timer; qe_timer < first_qe_timer + num_qe_timers; qe_timer++) {
				uint16_t counter;
				if(client.get_timer_counter(qe_timer, counter)){
					printf("QE %d counter:  %d.\n", qe_timer, counter);
					uint8_t timer_status;
					if(client.get_timer_status(qe_timer, timer_status)) {
						IOCX_TIMER_DIRECTION direction = iocx_decode_timer_direction(&timer_status);
						printf("QE %d direction:  %s.\n", qe_timer, (direction == UP) ? "Positive" : "Negative");
					}
				}
			}
			gpioDelay(10000);
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

		//test_navx_pi_spi();
	}
}

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


