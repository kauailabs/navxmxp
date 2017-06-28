/*
 * DaGamaClient.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include "DaGamaClient.h"
#include <pigpio.h>

static unsigned vmx_pi_comm_rcv_ready_signal_bcm_pin = 26;
const static int vmx_pi_ahrs_int_bcm_pin = 12;
const static int vmx_pi_can_int_bcm_pin = 13;
const static int vmx_pi_iocx_int_bcm_pin = 06;

static unsigned vmx_pi_rpi_gpio_to_bcm_pin_map[10] = {
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

DaGamaClient::DaGamaClient(uint8_t ahrs_update_rate_hz) :
	SPIClient(vmx_pi_comm_rcv_ready_signal_bcm_pin),
	AHRS((SPIClient&)*this, ahrs_update_rate_hz),
	IOCXClient((SPIClient&)*this),
	CANClient((SPIClient&)*this) {
}

DaGamaClient::~DaGamaClient() {
	this->Stop();
}

void DaGamaClient::gpio_isr(int gpio, int level, uint32_t tick, void *userdata)
{
	DaGamaClient *p_this = (DaGamaClient *)userdata;
	switch(gpio){
	case vmx_pi_iocx_int_bcm_pin:
		/* request IOCX Interrupt Status, indicating which VMX GPIO was the source */
		uint16_t iocx_int_status;
		if(p_this->get_gpio_interrupt_status(iocx_int_status)) {
			/* Notify HAL Client of Interrupt */
		} else {
			printf("Error retrieving VMX IOCX Interrupt Status in gpio_isr().\n");
		}
		break;

	case vmx_pi_can_int_bcm_pin:
		/* New CAN RX data is available. */
		/* Notify the CAN Client */
		break;

	case vmx_pi_ahrs_int_bcm_pin:
		/* Notify AHRS Client new data is ready. */
		break;

	default:
		/* This GPIO is one of the raspberry pi GPIOs */
		/* Notify HAL Client of Interrupt. */
		break;
	}
}

bool DaGamaClient::enable_rpi_gpio_interrupt(unsigned vmx_pi_gpio_num)
{
	if (vmx_pi_gpio_num >= (sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map)/sizeof(vmx_pi_rpi_gpio_to_bcm_pin_map[0]))) {
		return false;
	}
	int retcode = gpioSetISRFuncEx(vmx_pi_rpi_gpio_to_bcm_pin_map[vmx_pi_gpio_num], RISING_EDGE, 0 /* No Timeout */, DaGamaClient::gpio_isr, this);
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

bool DaGamaClient::disable_rpi_gpio_interrupt_int(unsigned vmx_pi_gpio_num){
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
