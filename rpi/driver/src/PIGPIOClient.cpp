/*
 * PIGPIOClient.cpp
 *
 *  Created on: 28 Jun 2017
 *      Author: pi
 */

#include "PIGPIOClient.h"
#include <pigpio.h>
#include <unistd.h> /* sleep() */
#include <time.h> /* nanosleep() */
#include <stdio.h>

/* VMX Pi Interrupt Signal Pin Mappings */
const static unsigned vmx_pi_comm_rcv_ready_signal_bcm_pin = 26;
const static unsigned vmx_pi_ahrs_int_bcm_pin = 12;
const static unsigned vmx_pi_can_int_bcm_pin = 13;
const static unsigned vmx_pi_iocx_int_bcm_pin = 06;

const static unsigned vmx_pi_interrupt_signal_to_bcm_pin_map[VMX_NUM_INT_INTERRUPT_PINS] = {
	vmx_pi_comm_rcv_ready_signal_bcm_pin,
	vmx_pi_ahrs_int_bcm_pin,
	vmx_pi_can_int_bcm_pin,
	vmx_pi_iocx_int_bcm_pin
};

/* Broadcom Pin #s corresponding to VMX PWM headers.   */
/* Pins on VMX PWM Headers are listed left-to-right    */
/* as seen from the board edge.                        */
const static unsigned vmx_pi_rpi_gpio_to_bcm_pin_map[VMX_NUM_EXT_GPIO_PINS] = {
	4,
	18,
	17,
	27,
	23,
	22,
	24,
	25,
	5,
	7,
};

/* Broadcom Pin #s corresponding to VMX EXT SPI port.  */
/* Pins on VMX EXT SPI Port are listed left-to-right   */
/* as seen from the board edge.                        */
const static unsigned vmx_pi_ext_spi_pin_map[VMX_NUM_EXT_SPI_PINS] = {
	11, // CLK
	10, // MOSI
	9, // MISO
	8, // CS (CE0)
};

/* Broadcom Pin #s corresponding to VMX EXT UART port. */
/* Pins on VMX EXT UART Port are listed left-to-right  */
/* as seen from the board edge.                        */
const static unsigned vmx_pi_ext_uart_pin_map[VMX_NUM_EXT_UART_PINS] = {
	14, // TX
	15, // RX
};

/* Broadcom Pin #s corresponding to VMX EXT I2C port.  */
/* Pins on VMX EXT I2C Port are listed left-to-right   */
/* as seen from the board edge.                        */
const static unsigned vmx_pi_ext_i2c_pin_map[VMX_NUM_EXT_I2C_PINS] = {
	2, // SDA
	3, // SCL
};

static const unsigned rpi_aux_spi_cs2_pin = 16;

/* Broadcom Pin #s corresponding to VMX INT SPI port.  */
const static unsigned vmx_pi_int_spi_pin_map[VMX_NUM_INT_SPI_PINS] = {
		21, // CLK
		20, // MOSI
		19, // MISO
		rpi_aux_spi_cs2_pin // CS (CE2)
};

typedef enum {
	GPIO,
	UART,
	SPI,
	I2C
} PIGPIOPinType;

typedef struct {
	PIGPIOPinType type;
	uint8_t first_index;
	uint8_t count;
	const unsigned *p_bcm_pin_num_array;
} PIGPIOPinRange;

static PIGPIOPinRange pin_ranges[] =
{
	PIGPIOPinType::GPIO,  0, 10, vmx_pi_rpi_gpio_to_bcm_pin_map,
	PIGPIOPinType::UART, 10,  2, vmx_pi_ext_uart_pin_map,
	PIGPIOPinType::SPI,  12,  4, vmx_pi_ext_spi_pin_map,
	PIGPIOPinType::I2C,  16,  2, vmx_pi_ext_i2c_pin_map
};

static bool PIGPIOChannelIndexToPinTypeAndBcmPinNumber(PIGPIOChannelIndex pigpio_channel_index, PIGPIOPinType& pin_type, unsigned& bcm_pin_number)
{
	for (size_t i = 0; i < (sizeof(pin_ranges)/sizeof(pin_ranges[0])); i++) {
		if (pin_ranges[i].first_index <= pigpio_channel_index) {
			if (pin_ranges[i].first_index + pin_ranges[i].count > pigpio_channel_index) {
				pin_type = pin_ranges[i].type;
				bcm_pin_number = pin_ranges[i].p_bcm_pin_num_array[pigpio_channel_index - pin_ranges[i].first_index];
				return true;
			}
		}
	}
	return false;
}

static const unsigned default_i2c_slave_address = 0x40;
static const unsigned default_i2c_bitbang_baudrate = 100000;
static const bool default_i2c_bitbang = false;

#define USE_RCV_READY_SIGNAL /* Comment out to use simple timed waits */

PIGPIOClient::PIGPIOClient(bool realtime)
{
	p_io_interrupt_sink = 0;
	p_can_interrupt_sink = 0;
	p_ahrs_interrupt_sink = 0;

	ext_spi_enabled = false;
	ext_uart_enabled = false;
	ext_i2c_enabled = false;
	synthetic_i2c_handle = PI_BAD_HANDLE;
	internal_i2c_handle = PI_BAD_HANDLE;
	curr_i2c_slave_address = default_i2c_slave_address;
	curr_i2c_bitbang = default_i2c_bitbang;
	curr_i2c_bitbang_baudrate = default_i2c_bitbang_baudrate;

	/* If requested, enable realtime priority in the pigpio library.
	 * Note that gpioCfgSetInternals() must be invoked BEFORE invoking
	 * gpioInitialise() below.
	 */
	if (realtime) {
		gpioCfgSetInternals(PI_CFG_RT_PRIORITY);
	}

	int ret = gpioInitialise();
	pigpio_initialized = (ret >= 0);
	if (pigpio_initialized)
	{
		printf("pigpio library version %d opened.\n", ret);
	} else {
	   printf("Error initializing pigpio library.\n");
	}

	spi_handle = PI_BAD_HANDLE;
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
		/* Gen 3 VMX Pi board uses 1KOhm resistor, enabling ~8Mhz bitrate */

		unsigned baud = 8000000; /* APB Clock (125MHz) / 16 = 7.8mHZ */;
		spi_handle = spiOpen(navxpi_aux_spi_chan, baud, spi_open_flags);
		if ( spi_handle >= 0 ) {
		   printf("SPI Aux Channel 2 opened.\n");
		} else {
			printf("Error opening SPI AUX Channel 2.\n");
		}
		gpioSetMode(vmx_pi_comm_rcv_ready_signal_bcm_pin, PI_INPUT);
		gpioSetPullUpDown(vmx_pi_comm_rcv_ready_signal_bcm_pin, PI_PUD_UP);

		gpioSetMode(rpi_aux_spi_cs2_pin, PI_OUTPUT);
		gpioWrite(rpi_aux_spi_cs2_pin, 1);
	}
}

PIGPIOClient::~PIGPIOClient()
{
	if(spi_handle != PI_BAD_HANDLE){
	   if(spiClose(spi_handle)==0){
		   printf("Closed SPI Aux Channel 2.\n");
	   } else {
			printf("Error closing SPI AUX Channel 2.\n");
	   }
	   spi_handle = PI_BAD_HANDLE;
	}
	if (pigpio_initialized) {
		pigpio_initialized = false;
		gpioTerminate();
		printf("pigpio library closed.\n");
	}
}

uint32_t PIGPIOClient::GetCurrentMicrosecondTicks()
{
	return gpioTick();
}

uint64_t PIGPIOClient::GetTotalCurrentMicrosecondTicks() {
	uint64_t bigtimestamp = (uint64_t)GetCurrentMicrosecondTicksHighPortion();
	bigtimestamp <<= 32;
	bigtimestamp += gpioTick();
	return bigtimestamp;
}

uint32_t PIGPIOClient::GetCurrentMicrosecondTicksHighPortion() {
	return gpioTickHigh();
}

uint32_t PIGPIOClient::Delay(uint32_t delay_us)
{
	return gpioDelay(delay_us);
}

bool PIGPIOClient::RegisterTimerNotification(int notification_slot /* 0-9 */, PIGPIOClientTimerFuncEx_t handler, unsigned millis, void *param)
{
	return (gpioSetTimerFuncEx(notification_slot, millis, handler, param) == 0);
}

bool PIGPIOClient::DeregisterTimerNotification(int notification_slot)
{
	return (gpioSetTimerFuncEx(notification_slot, 0, NULL, NULL) == 0);
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

static bool wait_for_vmx_int_spi_comm_ready(bool slave_ready_to_receive, uint32_t timeout_us) {
#ifdef USE_RCV_READY_SIGNAL
	int begin_signal = gpioRead(vmx_pi_comm_rcv_ready_signal_bcm_pin);
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
		int signal = gpioRead(vmx_pi_comm_rcv_ready_signal_bcm_pin);
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
	int end_signal = gpioRead(vmx_pi_comm_rcv_ready_signal_bcm_pin);
	bool end_ready = (slave_ready_to_receive ? (end_signal == 0) : (end_signal != 0));
#endif
	return false;
}

bool PIGPIOClient::int_spi_transmit(uint8_t *p_data, uint8_t len, bool write) {
	if (spi_handle == PI_BAD_HANDLE) return false;
	if (!pigpio_initialized) return false;

	wait_for_vmx_int_spi_comm_ready(true, 2000);
	int write_ret = spiWrite(spi_handle, (char *)p_data, len);
	if ( write_ret < 0 ) {
	   printf("navxpi write failure %d\n", write_ret);
	} else {
		if(write){
			wait_for_vmx_int_spi_comm_ready(true, 2000);
		}
	}
	return (write_ret == len);
}

bool PIGPIOClient::int_spi_receive(uint8_t *p_data, uint8_t len) {
	if (spi_handle == PI_BAD_HANDLE) return false;
	if (!pigpio_initialized) return false;
	wait_for_vmx_int_spi_comm_ready(false, 2000);
	int read_ret = spiRead(spi_handle,(char *)p_data, len);
	if ( read_ret < 0 ) {
		printf("navxpi read failure %d\n", read_ret);
	}
	return (read_ret == len);
}

void PIGPIOClient::gpio_isr(int gpio, int level, uint32_t tick, void *userdata)
{
	PIGPIOClient *p_this = (PIGPIOClient *)userdata;
	if (!p_this) return;
	uint64_t curr_timestamp = p_this->GetTotalSystemTimeOfTick(tick);
	switch(gpio){
	case vmx_pi_iocx_int_bcm_pin:
		{
			IIOInterruptSink *p_io_interrupt_sink = p_this->p_io_interrupt_sink;
			if (p_io_interrupt_sink) {
				p_io_interrupt_sink->IOCXInterrupt(PIGPIOInterruptEdge(level), curr_timestamp);
			}
		}
		break;

	case vmx_pi_can_int_bcm_pin:
		{
			ICANInterruptSink *p_can_interrupt_sink = p_this->p_can_interrupt_sink;
			if (p_can_interrupt_sink) {
				p_can_interrupt_sink->CANInterrupt(curr_timestamp);
			}
		}
		break;

	case vmx_pi_ahrs_int_bcm_pin:
		{
			IAHRSInterruptSink *p_ahrs_interrupt_sink = p_this->p_ahrs_interrupt_sink;
			if (p_ahrs_interrupt_sink) {
				p_ahrs_interrupt_sink->AHRSInterrupt(curr_timestamp);
			}
		}
		break;

	default:
		{
			IIOInterruptSink *p_io_interrupt_sink = p_this->p_io_interrupt_sink;
			if (p_io_interrupt_sink) {
				p_io_interrupt_sink->PIGPIOInterrupt(gpio, PIGPIOInterruptEdge(level), curr_timestamp);
			}
		}
		break;
	}
}

bool PIGPIOClient::EnableGPIOInterrupt(unsigned vmx_pi_gpio_num)
{
	if (vmx_pi_gpio_num >= (sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map)/sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map[0]))) {
		return false;
	}
	int retcode = gpioSetISRFuncEx(vmx_pi_rpi_gpio_to_bcm_pin_map[vmx_pi_gpio_num], RISING_EDGE, 0 /* No Timeout */, PIGPIOClient::gpio_isr, this);
	if(retcode == 0) {
		return true;
	} else {
		if (retcode == PI_BAD_GPIO) printf("RPI GPIO Interrupt Enable:  PI_BAD_GPIO.\n");
		else if (retcode == PI_BAD_EDGE) printf("RPI GPIO Interrupt Enable:  PI_BAD_EDGE.\n");
		else if (retcode == PI_BAD_ISR_INIT) printf("RPI GPIO Interrupt Enable:  PI_BAD_ISR_INIT.\n");
		else printf("RPI_GPIO_InterruptEnable:  unknown error code %d.\n", retcode);
		return false;
	}
}

bool PIGPIOClient::DisableGPIOInterrupt(unsigned vmx_pi_gpio_num){
	if (vmx_pi_gpio_num >= (sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map)/sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map[0]))) {
		return false;
	}
	int retcode = gpioSetISRFunc(vmx_pi_rpi_gpio_to_bcm_pin_map[vmx_pi_gpio_num], RISING_EDGE, 0 /* No Timeout */, NULL);
	if(retcode == 0) {
		return true;
	} else {
		if (retcode == PI_BAD_GPIO) printf("RPI GPIO Interrupt Enable:  PI_BAD_GPIO.\n");
		else if (retcode == PI_BAD_EDGE) printf("RPI GPIO Interrupt Enable:  PI_BAD_EDGE.\n");
		else if (retcode == PI_BAD_ISR_INIT) printf("RPI GPIO Interrupt Enable:  PI_BAD_ISR_INIT.\n");
		else printf("RPI_GPIO_InterruptEnable:  unknown error code %d.\n", retcode);
		return false;
	}
}

void PIGPIOClient::test_gpio_inputs(int iteration_count)
{
	for ( int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++ ) {
		gpioSetMode(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_INPUT);
	}
	for (int i = 0; i < VMX_NUM_EXT_SPI_PINS; i++) {
		gpioSetMode(vmx_pi_ext_spi_pin_map[i], PI_INPUT);
		gpioSetPullUpDown(vmx_pi_ext_spi_pin_map[i], PI_PUD_OFF); /* SPI/UART must disable pull resistors */
	}
	for (int i = 0; i < VMX_NUM_EXT_UART_PINS; i++) {
		gpioSetMode(vmx_pi_ext_uart_pin_map[i], PI_INPUT);
		gpioSetPullUpDown(vmx_pi_ext_uart_pin_map[i], PI_PUD_OFF); /* SPI/UART must disable pull resistors */
	}

	while(iteration_count-- > 0) {
		for ( int i = 0; i < 10; i++ ) {
			gpioSetPullUpDown(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_PUD_DOWN); // Sets a pull-down.
		}
		sleep(1);

		for ( int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++ ) {
			int on = gpioRead(vmx_pi_rpi_gpio_to_bcm_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("    ");
		for (int i = 0; i < VMX_NUM_EXT_SPI_PINS; i++) {
			int on = gpioRead(vmx_pi_ext_spi_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("    ");
		for (int i = 0; i < VMX_NUM_EXT_UART_PINS; i++) {
			int on = gpioRead(vmx_pi_ext_uart_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("\n");

		for ( int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++ ) {
			gpioSetPullUpDown(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_PUD_UP); // Sets a pull-up.
		}
		sleep(1);
		for ( int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++ ) {
			int on = gpioRead(vmx_pi_rpi_gpio_to_bcm_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("    ");
		for (int i = 0; i < VMX_NUM_EXT_SPI_PINS; i++) {
			int on = gpioRead(vmx_pi_ext_spi_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("    ");
		for (int i = 0; i < VMX_NUM_EXT_UART_PINS; i++) {
			int on = gpioRead(vmx_pi_ext_uart_pin_map[i]);
			if ( i != 0 ) {
				printf(", ");
			}
			printf("%d", on);
		}
		printf("\n");
	}
}

void PIGPIOClient::test_gpio_outputs()
{
	for ( int i = 0; i < 10; i++ ) {
		gpioSetMode(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_OUTPUT);
	}
	int curr_output = 0;
	while(true) {
		printf("Setting navX-PI RPI GPIO Output %d high.\n", curr_output + 1);
		for ( int i = 0; i < 10; i++ ) {
			gpioWrite(vmx_pi_rpi_gpio_to_bcm_pin_map[i],(i == curr_output) ? 1 : 0);
		}
		sleep(2);
		curr_output++;
		if ( curr_output > 9 ) {
			curr_output = 0;
		}
	}
}

bool PIGPIOClient::ConfigureGPIOAsOutput(PIGPIOChannelIndex gpio_index)
{
	PIGPIOPinType pintype;
	unsigned bcm_pin_number;
	if(!PIGPIOChannelIndexToPinTypeAndBcmPinNumber(gpio_index, pintype, bcm_pin_number)) return false;
	return (gpioSetMode(bcm_pin_number, PI_OUTPUT) == 0);
}

bool PIGPIOClient::ConfigureGPIOAsInput(PIGPIOChannelIndex gpio_index)
{
	PIGPIOPinType pintype;
	unsigned bcm_pin_number;
	if(!PIGPIOChannelIndexToPinTypeAndBcmPinNumber(gpio_index, pintype, bcm_pin_number)) return false;
	return (gpioSetMode(bcm_pin_number, PI_INPUT) == 0);
}

bool PIGPIOClient::ConfigureGPIOInputPullUpDown(PIGPIOChannelIndex gpio_index, bool enable_pulling, bool pullup)
{
	if (!enable_pulling) {
		gpioSetPullUpDown(gpio_index, PI_PUD_OFF);  // Clear any pull-ups/downs.
	} else {
		gpioSetPullUpDown(gpio_index, pullup ? PI_PUD_UP : PI_PUD_DOWN);   // Sets a pull-up/down.
	}
	return true;
}

bool PIGPIOClient::ConfigurePWMFrequency(PIGPIOChannelIndex gpio_index, unsigned frequency_hz)
{
	PIGPIOPinType pintype;
	unsigned bcm_pin_number;
	if(!PIGPIOChannelIndexToPinTypeAndBcmPinNumber(gpio_index, pintype, bcm_pin_number)) return false;
	return (gpioSetPWMfrequency(bcm_pin_number, frequency_hz) == 0);
}



/* This function configures all VMX-pi 10 RPI PGIO Pins as PWM,
 * as well as all SPI and UART pins, and outputs PWM signals
 * at varying duty cycles onto all pins.
 */
void PIGPIOClient::test_pwm_outputs()
{
	for (int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++) {
		gpioSetMode(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_OUTPUT);
		gpioSetPWMfrequency(vmx_pi_rpi_gpio_to_bcm_pin_map[i], 200);
		gpioPWM(vmx_pi_rpi_gpio_to_bcm_pin_map[i], 10 + (i*10));
	}
	for (int i = 0; i < VMX_NUM_EXT_SPI_PINS; i++) {
		gpioSetMode(vmx_pi_ext_spi_pin_map[i], PI_OUTPUT);
		gpioSetPWMfrequency(vmx_pi_ext_spi_pin_map[i], 200);
		gpioPWM(vmx_pi_ext_spi_pin_map[i], 10 + (i*10));

	}
	for (int i = 0; i < VMX_NUM_EXT_UART_PINS; i++) {
		gpioSetMode(vmx_pi_ext_uart_pin_map[i], PI_OUTPUT);
		gpioSetPWMfrequency(vmx_pi_ext_uart_pin_map[i], 200);
		gpioPWM(vmx_pi_ext_uart_pin_map[i], 10 + (i*10));
	}
	sleep(20);
	for (int i = 0; i < VMX_NUM_EXT_GPIO_PINS; i++) {
		gpioSetMode(vmx_pi_rpi_gpio_to_bcm_pin_map[i], PI_INPUT);
	}
	for (int i = 0; i < VMX_NUM_EXT_SPI_PINS; i++) {
		gpioSetMode(vmx_pi_ext_spi_pin_map[i], PI_INPUT);
	}
	for (int i = 0; i < VMX_NUM_EXT_UART_PINS; i++) {
		gpioSetMode(vmx_pi_ext_uart_pin_map[i], PI_INPUT);
	}

}

bool PIGPIOClient::UARTOpen(unsigned baud, int& handle)
{
	int serial_port_handle = serOpen((char *)"/dev/ttyS0", baud, 0);
	if (serial_port_handle >= 0) {
		handle = serial_port_handle;
		return true;
	}
	return false;
}

bool PIGPIOClient::UARTClose(int handle)
{
	return (serClose(handle) == 0);
}

bool PIGPIOClient::SPIMasterOpen(unsigned baud, unsigned mode /* 0-3 */, bool cs_active_low, bool msbfirst, int& spi_handle)
{
	   unsigned spi_open_flags = 0;   /* Primary spi dev, cs active low, 8-bits/word, 4-wire SPI, MSB first, Mode 0 */
	   unsigned vmx_ext_spi_chan = 0; /* CS0 is routed in the VMX hardware (CS1 is not). */

	   mode &= 0x3;
	   spi_open_flags |= mode;		  /* Set requested mode (limit to 0-3) */

	   if (!cs_active_low) {
		   spi_open_flags |= 4;		  /* If CS is active high, OR in 'p0' bit */
	   }

	   if(!msbfirst) {
		   spi_open_flags |= 0x4000;  /* If LSBFirst, OR in 'T' and 'R' bits */
		   spi_open_flags |= 0x8000;
	   }

	   int vmx_spi_handle = spiOpen(vmx_ext_spi_chan, baud, spi_open_flags);

	   if (vmx_spi_handle >= 0) {
		   spi_handle = vmx_spi_handle;
		   return true;
	   }

	   return false;
}

bool PIGPIOClient::SPIMasterClose(int handle)
{
	return (spiClose(handle) == 0);
}

bool PIGPIOClient::I2CMasterOpenInternal(uint8_t i2c_slave_address, bool bitbang, unsigned bitbang_baudrate /* 50-500000 */, int& i2c_handle)
{
	/* Note:  if using hardware i2c, the default baudrate is 100Khz, however this can be increased */
	/* see:  http://arduino-pi.blogspot.com/2014/03/speeding-up-i2c-bus-on-raspberry-pi-and.html */
	/* Todo:  Add bitbang variant */
	if (internal_i2c_handle == PI_BAD_HANDLE) {
		internal_i2c_handle = i2cOpen(1 /* VMX Board I2C Bus */, i2c_slave_address, 0 /* Flags */ );
		if (internal_i2c_handle >= 0) {
			curr_i2c_slave_address = i2c_slave_address;
			curr_i2c_bitbang = bitbang;
			curr_i2c_bitbang_baudrate = bitbang_baudrate;
			synthetic_i2c_handle = 0;
			i2c_handle = synthetic_i2c_handle;
			return true;
		}
	}
	return false;
}

bool PIGPIOClient::I2CMasterOpen(bool bitbang, unsigned baudrate /* 50-500000 */, int& i2c_handle)
{
	return I2CMasterOpenInternal(curr_i2c_slave_address, bitbang, baudrate, i2c_handle);
}

bool PIGPIOClient::I2CMasterClose(int i2c_handle)
{
	if (i2c_handle == synthetic_i2c_handle) {
		if (internal_i2c_handle >= 0) {
			i2cClose(internal_i2c_handle);
			internal_i2c_handle = PI_BAD_HANDLE;
			return true;
		}
	}
	return false;
}

bool PIGPIOClient::I2CMasterReopenInternal(uint8_t new_i2c_slave_address) {
	if (I2CMasterClose(synthetic_i2c_handle)) {
		int unused_handle;
		return I2CMasterOpenInternal(new_i2c_slave_address, curr_i2c_bitbang, curr_i2c_bitbang_baudrate, unused_handle);
	}
	return false;
}

bool PIGPIOClient::I2CMasterWrite(int i2c_handle, uint8_t deviceAddress_7bit, uint8_t* dataToSend, int32_t sendSize)
{
	if ((i2c_handle == synthetic_i2c_handle) && (internal_i2c_handle >= 0)) {
		if (deviceAddress_7bit != curr_i2c_slave_address) {
			I2CMasterReopenInternal(deviceAddress_7bit);
		}
		return i2cWriteDevice(internal_i2c_handle, (char *)dataToSend, unsigned(sendSize));
	}
	return false;
}

bool PIGPIOClient::I2CMasterRead(int i2c_handle, uint8_t deviceAddress_7bit, uint8_t* buffer, int32_t count)
{
	if ((i2c_handle == synthetic_i2c_handle) && (internal_i2c_handle >= 0)) {
		if (deviceAddress_7bit != curr_i2c_slave_address) {
			I2CMasterReopenInternal(deviceAddress_7bit);
		}
		return i2cReadDevice(internal_i2c_handle, (char *)buffer, unsigned(count));
	}
	return false;
}

bool PIGPIOClient::I2CMasterTransaction(int i2c_handle, uint8_t deviceAddress_7bit,
                    uint8_t* dataToSend, uint16_t sendSize,
                    uint8_t* dataReceived, uint16_t receiveSize)
{
	if ((i2c_handle == synthetic_i2c_handle) && (internal_i2c_handle >= 0)) {
		if (deviceAddress_7bit != curr_i2c_slave_address) {
			I2CMasterReopenInternal(deviceAddress_7bit);
		}

		pi_i2c_msg_t segments[2];
		segments[0].addr = deviceAddress_7bit;
		segments[0].flags = 0; /* Write */
		segments[0].buf = dataToSend;
		segments[0].len = sendSize;

		segments[1].addr = deviceAddress_7bit;
		segments[1].flags = 1; /* Read */
		segments[1].buf = dataReceived;
		segments[1].len = receiveSize;

		return (i2cSegments(internal_i2c_handle, segments, 2) == 0);
	}
	return false;
}

void PIGPIOClient::test_ext_serial()
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

void PIGPIOClient::test_ext_spi()
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

void PIGPIOClient::test_ext_i2c()
{
	unsigned i2c_bus = 1; /* 0 = RPI Internal I2C (reserved for eeprom); 1 = User (VMX-Pi) */
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

uint64_t PIGPIOClient::GetTotalSystemTimeOfTick(uint32_t tick) {
	uint64_t curr_timestamp = GetTotalCurrentMicrosecondTicks();
	if (uint32_t(curr_timestamp) < tick) {
		/* Low portion of timestamp has rolled over.  Decrease curr timestamp's "high" portion by 1 */
		uint32_t curr_timestamp_high = (uint32_t)(curr_timestamp >> 16);
		curr_timestamp_high -= 1;
		curr_timestamp = (((uint64_t)curr_timestamp_high) << 16);
	} else {
		curr_timestamp &= 0xFFFFFFFF00000000;
	}
	curr_timestamp += tick;
	return curr_timestamp;
}
