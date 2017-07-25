/*
 * Handlers.h
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#ifndef HANDLERS_H_
#define HANDLERS_H_

/* Handler for up to 32 different IO Resources [16 on VMX-pi, 16 on RPI] */
typedef void (*IO_InterruptHandler)(uint32_t io_interrupt_num, void* param, uint64_t timestamp_us);

/* Handler invoked when a registered notification has been triggered */
typedef void (*NotifyHandler)(void *param, uint64_t timestamp_us);

typedef NotifyHandler CAN_NewRxDataNotifyHandler;

typedef NotifyHandler AHRS_NewRxDataNotifyHandler;

#endif /* HANDLERS_H_ */
