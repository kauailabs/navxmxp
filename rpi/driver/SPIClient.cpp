/*
 * SPIClient.cpp
 *
 *  Created on: 20 Feb 2017
 *      Author: pi
 */

#include <mutex>
#include <stdio.h>
#include <pigpio.h>

#include "SPIClient.h"
#include "NavXSPIMessage.h"

static std::mutex vmx_spi_mutex;
static unsigned rcv_ready_bcm_signal_pin;
static const unsigned rpi_aux_spi_cs2_pin = 16;

#define USE_RCV_READY_SIGNAL /* Comment out to use simple timed waits */

SPIClient::SPIClient(unsigned comm_rcv_ready_bcm_signal_pin) {
	int ret = gpioInitialise();
	pigpio_initialized = (ret >= 0);
	if (pigpio_initialized)
	{
		printf("pigpio library version %d opened.\n", ret);
	} else {
	   printf("Error initializing pigpio library.\n");
	}

	spi_handle = PI_BAD_HANDLE;
	NavXSPIMessage::init_crc_table();
	if(pigpio_initialized){
		unsigned spi_open_flags = 0;
		/* Mode 0, active low (all channels), reserve GPIO for SPI, 8-bits/word, std 4-wire SPI, MSB first */
		spi_open_flags |= 0x100; /* Auxiliary SPI Device */
		unsigned navxpi_aux_spi_chan = 2;

		/* SPI Bitrate table */
		/*                   */
		/* The SPI Bitrate on the Raspberry Pi is a multiple of the Broadcom ARM CPU's APB Clock (125Mhz) */
		/* 125MHZ / 128 =  976562.5 */
		/* 125Mhz /  64 = 1953125   */
		/* 125Mhz /  32  = 3906250   */
		/* 125Mhz /  16  = 7812500   */
		/* Note:  Gen 2 VMX Pi board used a 2.2KOhm resistor between Rpi and STM32, limiting this bitrate to 1Mhz */

		unsigned baud = 4000000; /* APB Clock (125MHz) / 128 = 1.95mHZ */;
		spi_handle = spiOpen(navxpi_aux_spi_chan, baud, spi_open_flags);
		if ( spi_handle >= 0 ) {
		   printf("SPI Aux Channel 2 opened.\n");
		} else {
			printf("Error opening SPI AUX Channel 2.\n");
		}
		rcv_ready_bcm_signal_pin = comm_rcv_ready_bcm_signal_pin;
		gpioSetMode(rcv_ready_bcm_signal_pin, PI_INPUT);
		gpioSetPullUpDown(rcv_ready_bcm_signal_pin, PI_PUD_UP);

		gpioSetMode(rpi_aux_spi_cs2_pin, PI_OUTPUT);
		gpioWrite(rpi_aux_spi_cs2_pin, 1);
	}
}

SPIClient::~SPIClient() {
	if(spi_handle != PI_BAD_HANDLE){
	   if(spiClose(spi_handle)==0){
		   printf("Closed SPI Aux Channel 2.\n");
	   } else {
			printf("Error opening SPI AUX Channel 2.\n");
	   }
	}
	if (pigpio_initialized) {
	   gpioTerminate();
	   printf("pigpio library closed.\n");
	}
}

static void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result) {
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

const uint32_t comm_ready_sample_quantum_us = 10;  /* NOTE:  This value should be < 100 to ensure CPU busywait is used. */

static bool wait_for_slave_comm_ready(bool slave_ready_to_receive, uint32_t timeout_us) {
#ifdef USE_RCV_READY_SIGNAL
	int begin_signal = gpioRead(rcv_ready_bcm_signal_pin);
	bool begin_ready = (slave_ready_to_receive ? (begin_signal == 0) : (begin_signal != 0));
	struct timespec ts_begin;
	struct timespec ts_end;
	clock_gettime(CLOCK_MONOTONIC, &ts_begin);
#endif
	uint32_t remaining_us = timeout_us;
	while (remaining_us > 0) {
		uint32_t wait_quantum_us = comm_ready_sample_quantum_us;
		if (remaining_us < wait_quantum_us) {
			wait_quantum_us = remaining_us;
		}
		gpioDelay(wait_quantum_us);
		remaining_us -= wait_quantum_us;
#ifdef USE_RCV_READY_SIGNAL
		int signal = gpioRead(rcv_ready_bcm_signal_pin);
		bool ready = (slave_ready_to_receive ? (signal == 0) : (signal != 0));
		if (ready) {
			struct timespec ts_diff;
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			timespec_diff(&ts_begin, &ts_end, &ts_diff);
			if (ts_diff.tv_nsec > 300000) {
				printf("Unexpectedly long delay (%d microseconds) in wait_for_comm_ready(%s).\n", (int)(ts_diff.tv_nsec / 1000), (slave_ready_to_receive ? "true" : "false"));
			}
			return true;
		}
#endif
	}
#ifdef USE_RCV_READY_SIGNAL
	struct timespec ts_diff;
	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	timespec_diff(&ts_begin, &ts_end, &ts_diff);
	printf("Timeout in wait_for_comm_ready(%s) after %d microseconds.\n", (slave_ready_to_receive ? "true" : "false"), timeout_us);
	int end_signal = gpioRead(rcv_ready_bcm_signal_pin);
	bool end_ready = (slave_ready_to_receive ? (end_signal == 0) : (end_signal != 0));
#endif
	return false;
}

static bool transmit_internal(int spi_handle, uint8_t *p_data, uint8_t len, bool write) {
	if(spi_handle == PI_BAD_HANDLE) return false;
	wait_for_slave_comm_ready(true, 2000);
	int write_ret = spiWrite(spi_handle, (char *)p_data, len);
	if ( write_ret < 0 ) {
	   printf("navxpi write failure %d\n", write_ret);
	} else {
		if(write){
			wait_for_slave_comm_ready(true, 2000);
		}
	}
	return (write_ret == len);
}

bool receive_internal(int spi_handle, uint8_t *p_data, uint8_t len) {
	if(spi_handle == PI_BAD_HANDLE) return false;
	wait_for_slave_comm_ready(false, 2000);
	int read_ret = spiRead(spi_handle,(char *)p_data, len);
	if ( read_ret < 0 ) {
		printf("navxpi read failure %d\n", read_ret);
	}
	return (read_ret == len);
}

bool SPIClient::transmit(uint8_t *p_data, uint8_t len, bool write)
{
	std::unique_lock<std::mutex> sync(vmx_spi_mutex);
	if (write && (len > NavXSPIMessage::get_standard_packet_size())) {
		/* If this is a write request w/larger-than-standard packet size, switch to temporary variable write length mode */
		NavXSPIMessage var_write_request_msg(VMXPI_SPECIALMODE_BANK,
				VMXPI_SPECIALMODE_REG_VARIABLEWRITE | 0x80,
				len,
				NULL);
		if(!transmit_internal(spi_handle, var_write_request_msg.get_packet_ptr(), var_write_request_msg.get_packet_size(), true)) {
			return false;
		}
	}

	return transmit_internal(spi_handle, p_data, len, write);
}

/* Note:  rx_len should include one byte for the CRC, in addition to the
 * amount of expected data.
 */
bool SPIClient::transmit_and_receive(uint8_t *p_tx_data, uint8_t tx_len, uint8_t *p_rx_data, uint8_t rx_len, bool write)
{
	std::unique_lock<std::mutex> sync(vmx_spi_mutex);
	if(transmit_internal(spi_handle, p_tx_data, tx_len, write)) {
		return receive_internal(spi_handle, p_rx_data, rx_len);
	} else {
		return false;
	}
}

bool SPIClient::write(NavXSPIMessage& request)
{
	return transmit(request.get_packet_ptr(), request.get_packet_size(), true);
}

/* Note:  response_len should include one byte for the CRC, in addition to the
 * amount of expected data.
 */
bool SPIClient::read(NavXSPIMessage& request, uint8_t *p_response, uint8_t response_len)
{
	if (transmit_and_receive(request.get_packet_ptr(), request.get_packet_size(), p_response, response_len, false)){
		if (NavXSPIMessage::validate_read_response(p_response,response_len)){
			//printf("Read Complete:  %d bytes, CRC:  %u, Data:  %u %u %u %u %u\n", response_len, p_response[response_len-1], p_response[0], p_response[1], p_response[2], p_response[3], p_response[4]);
			return true;
		} else {
			printf("CRC error during read response.\n");
		}
	}
	return false;
}
