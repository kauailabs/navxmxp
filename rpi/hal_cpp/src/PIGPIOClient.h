/*
 * PIGPIOClient.h
 *
 *  Created on: 28 Jun 2017
 *      Author: pi
 */

#ifndef PIGPIOCLIENT_H_
#define PIGPIOCLIENT_H_

#include <stdint.h>

#include "IPIGPIOInterruptSinks.h"
#include "VMXHandlers.h"

#define VMX_NUM_EXT_DEDICATED_GPIO_PINS	10
#define VMX_NUM_EXT_SPI_PINS 		4
#define VMX_NUM_EXT_UART_PINS 		2
#define VMX_NUM_EXT_I2C_PINS   		2
#define VMX_NUM_INT_SPI_PINS		4
#define VMX_NUM_INT_INTERRUPT_PINS	4
#define VMX_NUM_EXT_TOTAL_GPIO_PINS	(VMX_NUM_EXT_DEDICATED_GPIO_PINS + VMX_NUM_EXT_UART_PINS + VMX_NUM_EXT_SPI_PINS)

typedef uint8_t PIGPIOChannelIndex;

typedef void (*PIGPIOClientTimerFuncEx_t) (void *userdata);

/* Manages resources and associated access to Raspberry Pi GPIO resources via the
 * pigpio library.
 * This library also encapsulates the interface to the pigpio library.
 */
class PIGPIOClient {
private:
	bool ext_spi_enabled;
	bool ext_uart_enabled;
	bool ext_i2c_enabled;

	bool pigpio_initialized;
	int spi_handle;
	int synthetic_i2c_handle;
	int internal_i2c_handle;
	unsigned curr_i2c_slave_address;
	bool curr_i2c_bitbang;
	unsigned curr_i2c_bitbang_baudrate;

	IIOInterruptSink *volatile p_io_interrupt_sink;
	ICANInterruptSink *volatile p_can_interrupt_sink;
	IAHRSInterruptSink *volatile p_ahrs_interrupt_sink;
public:

	typedef enum { INT_EDGE_RISING, INT_EDGE_FALLING, INT_EDGE_BOTH } PIGPIOIntEdge;

	PIGPIOClient(bool realtime);

	void SetIOInterruptSink(IIOInterruptSink *p_io_int_sink) {
		this->p_io_interrupt_sink = p_io_int_sink;
	}

	void SetCANInterruptSink(ICANInterruptSink *p_can_int_sink) {
		this->p_can_interrupt_sink = p_can_int_sink;
	}

	void SetAHRSInterruptSink(IAHRSInterruptSink *p_ahrs_int_sink) {
		this->p_ahrs_interrupt_sink = p_ahrs_int_sink;
	}

	bool IsOpen() { return pigpio_initialized; }

	uint8_t get_max_num_external_gpio_interrupts() {
		return (VMX_NUM_EXT_TOTAL_GPIO_PINS);
	}

	uint8_t get_max_num_external_gpios() {
		return (VMX_NUM_EXT_TOTAL_GPIO_PINS);
	}

	uint8_t get_curr_num_external_interrupts() {
		/* Note:  the ext i2c pins are open drain and are thus not
		 * made available for use as GPIOs and Interrupts.
		 */
		return VMX_NUM_EXT_TOTAL_GPIO_PINS;
	}

	uint32_t GetCurrentMicrosecondTicks();
	uint64_t GetTotalCurrentMicrosecondTicks();
	uint32_t GetCurrentMicrosecondTicksHighPortion();
	uint32_t Delay(uint32_t delay_us);
	uint64_t GetTotalSystemTimeOfTick(uint32_t tick);

	bool RegisterTimerNotification(int notification_slot /* 0-9 */, PIGPIOClientTimerFuncEx_t handler, unsigned millis, void *param);
	bool DeregisterTimerNotification(int notification_slot);

	bool ConfigureGPIOAsOutput(PIGPIOChannelIndex channel_index);
	bool ConfigureGPIOAsInput(PIGPIOChannelIndex channel_index);
	bool ConfigureGPIOInputPullUpDown(PIGPIOChannelIndex channel_index, bool enable_pulling, bool pullup);
	bool ConfigurePWMFrequency(PIGPIOChannelIndex channel_index, unsigned frequency_hz);
	bool UARTOpen(unsigned baud, int& handle);
	bool UARTClose(int handle);

	bool SPIMasterOpen(unsigned baud, unsigned mode /* 0-3 */, bool cs_active_low, bool msbfirst, int& spi_handle);
	bool SPIMasterClose(int handle);

	bool I2CMasterOpen(bool bitbang, unsigned baudrate /* 50-500000 */, int& i2c_handle);
	bool I2CMasterClose(int i2c_handle);

	bool I2CMasterWrite(int i2c_handle, uint8_t deviceAddress_7bit, uint8_t* dataToSend, int32_t sendSize);
	bool I2CMasterRead(int i2c_handle, uint8_t deviceAddress_7bit, uint8_t* buffer, int32_t count);
	bool I2CMasterTransaction(int i2c_handle, uint8_t deviceAddress_7bit,
	                    uint8_t* dataToSend, uint16_t sendSize,
	                    uint8_t* dataReceived, uint16_t receiveSize);

	uint8_t get_num_internal_interrupts() {
		return VMX_NUM_INT_INTERRUPT_PINS;
	}

	/* The following "ext" comm enables only allocate the gpio resources */
	/* to the communication function.  Separate methods for configuring  */
	/* each communication port are used to perform configuration.        */

	bool set_ext_spi_enabled(bool enable_ext_spi);
	bool get_ext_spi_enabled() { return ext_spi_enabled; }
	bool set_ext_uart_enabled(bool enable_ext_uart);
	bool get_ext_uart_enabled() { return ext_uart_enabled; }
	bool set_ext_i2c_enabled(bool enable_ext_i2c);
	bool get_ext_i2c_enabled() { return ext_i2c_enabled; }

	bool int_spi_transmit(uint8_t *p_data, uint8_t len, bool write);
	bool int_spi_receive(uint8_t *p_data, uint8_t len);

	/* Tests */
	void test_gpio_outputs();
	void test_pwm_outputs();
	void test_gpio_inputs(int iteration_count);
	void test_ext_serial();
	void test_ext_spi();
	void test_ext_i2c();

	~PIGPIOClient();

	bool EnableGPIOInterrupt(unsigned vmx_pi_gpio_num, PIGPIOIntEdge edge_type);
	bool DisableGPIOInterrupt(unsigned vmx_pi_gpio_num);
	bool EnableIOCXInterrupt();
	bool DisableIOCXInterrupt();
	bool EnableCANInterrupt();
	bool DisableCANInterrupt();
	bool EnableAHRSInterrupt();
	bool DisableAHRSInterrupt();

private:
	static void gpio_isr(int gpio, int level, uint32_t tick, void *userdata);
	static void signal_func(int signum, void *userdata);

	bool I2CMasterOpenInternal(uint8_t i2c_slave_address, bool bitbang, unsigned bitbang_baudrate, int& i2c_handle);
	bool I2CMasterReopenInternal(uint8_t new_i2c_slave_address);

};

#endif /* PIGPIOCLIENT_H_ */
