/*
 * VMXHandlers.h
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#ifndef VMXHANDLERS_H_
#define VMXHANDLERS_H_

typedef enum {
	FALLING_EDGE_INTERRUPT = 0,
	RISING_EDGE_INTERRUPT = 1,
} InterruptEdgeType;

typedef void (*VMXIO_InterruptHandler)(uint32_t vmx_channel_index, InterruptEdgeType edge, void* param, uint64_t timestamp_us);

/* Handler invoked when a registered notification has been triggered */
typedef void (*VMXNotifyHandler)(void *param, uint64_t timestamp_us);

typedef VMXNotifyHandler CAN_NewRxDataNotifyHandler;

typedef VMXNotifyHandler AHRS_NewRxDataNotifyHandler;

#endif /* VMXHANDLERS_H_ */
