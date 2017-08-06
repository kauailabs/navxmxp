/*
 * VMXPiImpl.cpp
 *
 *  Created on: 31 Jul 2017
 *      Author: pi
 */

#include "VMXPiImpl.h"

VMXPiImpl::VMXPiImpl(bool realtime) :
	pigpio(realtime),
	spi(pigpio),
	iocx(spi),
	can(spi, pigpio),
	misc(spi)
{
	// TODO Auto-generated constructor stub

}

VMXPiImpl::~VMXPiImpl() {
	// TODO Auto-generated destructor stub
}

