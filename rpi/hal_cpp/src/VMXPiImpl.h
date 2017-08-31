/*
 * VMXPiImpl.h
 *
 *  Created on: 31 Jul 2017
 *      Author: pi
 */

#ifndef VMXPIIMPL_H_
#define VMXPIIMPL_H_

#include <stdint.h>
#include "PIGPIOClient.h"
#include "SPIClient.h"
#include "IOCXClient.h"
#include "CANClient.h"
#include "MISCClient.h"
#include "VMXChannelManager.h"
#include "VMXResourceManager.h"

class VMXPiImpl {

public:
	PIGPIOClient pigpio;
	SPIClient spi;
	IOCXClient iocx;
	CANClient can;
	MISCClient 	misc;
	VMXChannelManager chan_mgr;
	VMXResourceManager rsrc_mgr;

	VMXPiImpl(bool realtime);
	virtual ~VMXPiImpl();
};

#endif /* VMXPIIMPL_H_ */
