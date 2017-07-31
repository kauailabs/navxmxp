/*
 * VMXPiClient.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include "VMXPiClient.h"
#include "Handlers.h"
#include "Logger.h"
#include <pigpio.h>
#include <mutex>
#include <list>
#include <string>

static std::mutex handler_mutex;

Logger logger;
std::list<std::string> error_message_list;

VMXPiClient::VMXPiClient(uint8_t ahrs_update_rate_hz) :
	pigpio((IPIGPIOInterruptSink&)*this),
	spi((PIGPIOClient&)pigpio),
	iocx((SPIClient&)spi),
	ahrs((SPIClient&)spi, ahrs_update_rate_hz),
	can((SPIClient&)spi),
	misc((SPIClient&)spi),
	io(pigpio, iocx, misc, chan_mgr, rsrc_mgr, (IInterruptRegistrar&)*this),
	time((PIGPIOClient&)pigpio, (MISCClient&)misc),
	power((MISCClient&)misc),
	version(ahrs)
{
	VMXChannelManager::Init();
	for (size_t i = 0; i < (sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])); i++ ) {
		interrupt_notifications[i].Init();
	}
	can_notification.Init();
}

VMXPiClient::~VMXPiClient() {
	/* Deregister all handlers */
	for (size_t i = 0; i < (sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])); i++ ) {
		if (interrupt_notifications[i].interrupt_func) {
			DeregisterIOInterruptHandler(i);
		}
	}
	DeregisterCANNewRxDataHandler();
	DeregisterAHRSNewRxDataHandler();

	ahrs.Stop();	/* Stop AHRS Data IO Thread */
}

void VMXPiClient::iocx_interrupt(int level, uint32_t tick) {

	/* Request IOCX Interrupt Status, indicating which VMX GPIO was the source
	 * Note:  The act of reading the interrupt status register will automatically
	 * clear all pending interrupts.
	 * It's possible that multiple interrupts will be indicated in the
	 * iocx_int_status bits.  In this case, interrupt funcs can be triggered for each
	 * interrupt received. */

	uint16_t iocx_int_status;
	uint16_t iocx_last_interrupt_edges;
	if(iocx.get_gpio_interrupt_status(iocx_int_status, iocx_last_interrupt_edges)) {
		/* Notify HAL Client of Interrupt(s) */
		uint64_t curr_timestamp = time.GetTotalSystemTimeOfTick(tick);
		for ( int i = 0; i < 16; i++) {
			if(iocx_int_status & 0x0001) {
				if (interrupt_notifications[i].interrupt_func) {
					std::unique_lock<std::mutex> sync(handler_mutex);
					if (interrupt_notifications[i].interrupt_func) {
						InterruptEdgeType edge = InterruptEdgeType(iocx_last_interrupt_edges & 0x0001);
						interrupt_notifications[i].interrupt_func(i, edge, interrupt_notifications[i].interrupt_params, curr_timestamp);
					}
				}
			}
			iocx_int_status >>= 1;
			if (iocx_int_status == 0) {
				break;
			}
			iocx_last_interrupt_edges >>= 1;
		}
	} else {
		printf("Error retrieving VMX IOCX Interrupt Status in gpio_isr().\n");
	}
}

void VMXPiClient::can_interrupt(int level, uint32_t tick) {
	uint64_t curr_timestamp = time.GetTotalSystemTimeOfTick(tick);

	std::unique_lock<std::mutex> sync(handler_mutex);
	if (can_notification.handler_func) {
		can_notification.handler_func(can_notification.param, curr_timestamp);
	}
}

void VMXPiClient::ahrs_interrupt(int level, uint32_t tick) {
	uint64_t curr_timestamp = time.GetTotalSystemTimeOfTick(tick);
	std::unique_lock<std::mutex> sync(handler_mutex);
	if (ahrs_notification.handler_func) {
		ahrs_notification.handler_func(can_notification.param, curr_timestamp);
	}
}

void VMXPiClient::pigpio_interrupt(int gpio_num, int level, uint32_t tick) {
	uint8_t vmx_interrupt_number = iocx.get_num_interrupts() + gpio_num;
	if (interrupt_notifications[vmx_interrupt_number].interrupt_func) {
		InterruptEdgeType edge;
		if (level == FALLING_EDGE) {
			edge = FALLING_EDGE_INTERRUPT;
		} else {
			edge = RISING_EDGE_INTERRUPT;
		}
		uint64_t curr_timestamp = time.GetTotalSystemTimeOfTick(tick);
		std::unique_lock<std::mutex> sync(handler_mutex);
		if (interrupt_notifications[vmx_interrupt_number].interrupt_func) {
			interrupt_notifications[vmx_interrupt_number].interrupt_func(vmx_interrupt_number, edge, interrupt_notifications[vmx_interrupt_number].interrupt_params, curr_timestamp);
		}
	}
}

bool VMXPiClient::RegisterIOInterruptHandler(uint8_t index, IO_InterruptHandler handler, void *param)
{
	if (index >= uint8_t((sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])))) return false;
	std::unique_lock<std::mutex> sync(handler_mutex);
	if (interrupt_notifications[index].interrupt_func) return false;
	if (!handler) return false;
	interrupt_notifications[index].interrupt_func = handler;
	interrupt_notifications[index].interrupt_params = param;
	return true;
}

bool VMXPiClient::DeregisterIOInterruptHandler(uint8_t index)
{
	if (index >= uint8_t((sizeof(interrupt_notifications)/sizeof(interrupt_notifications[0])))) return false;
	std::unique_lock<std::mutex> sync(handler_mutex);
	interrupt_notifications[index].Init();
	return true;
}

bool VMXPiClient::RegisterCANNewRxDataHandler(CAN_NewRxDataNotifyHandler handler, void *param)
{
	std::unique_lock<std::mutex> sync(handler_mutex);
	if (can_notification.handler_func) return false;
	if (!handler) return false;
	can_notification.handler_func = handler;
	can_notification.param = param;
	return true;
}

bool VMXPiClient::DeregisterCANNewRxDataHandler()
{
	std::unique_lock<std::mutex> sync(handler_mutex);
	can_notification.Init();
	return true;
}

bool VMXPiClient::RegisterAHRSNewRxDataHandler(AHRS_NewRxDataNotifyHandler handler, void *param)
{
	std::unique_lock<std::mutex> sync(handler_mutex);
	if (ahrs_notification.handler_func) return false;
	if (!handler) return false;
	ahrs_notification.handler_func = handler;
	ahrs_notification.param = param;
	return true;
}

bool VMXPiClient::DeregisterAHRSNewRxDataHandler()
{
	std::unique_lock<std::mutex> sync(handler_mutex);
	ahrs_notification.Init();
	return true;
}



