/*
 * InterruptRegistrar.h
 *
 *  Created on: 26 Jul 2017
 *      Author: pi
 */

#ifndef INTERRUPTREGISTRAR_H_
#define INTERRUPTREGISTRAR_H_

#include "Handlers.h"

class IInterruptRegistrar {
public:
	virtual bool RegisterIOInterruptHandler(uint8_t index, IO_InterruptHandler handler, void *param) = 0;
	virtual bool DeregisterIOInterruptHandler(uint8_t index) = 0;

	virtual bool RegisterCANNewRxDataHandler(CAN_NewRxDataNotifyHandler handler, void *param) = 0;
	virtual bool DeregisterCANNewRxDataHandler() = 0;

	virtual bool RegisterAHRSNewRxDataHandler(AHRS_NewRxDataNotifyHandler handler, void *param) = 0;
	virtual bool DeregisterAHRSNewRxDataHandler() = 0;

	virtual ~IInterruptRegistrar() {}
};


#endif /* INTERRUPTREGISTRAR_H_ */
