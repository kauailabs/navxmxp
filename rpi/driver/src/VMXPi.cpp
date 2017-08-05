/*
 * VMXPi.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: pi
 */

#include "VMXPi.h"
#include "VMXPiImpl.h"
#include "Logger.h"
#include <pigpio.h>
#include <list>
#include <string>

Logger logger;
std::list<std::string> error_message_list;

VMXPi::VMXPi(bool realtime, uint8_t ahrs_update_rate_hz) :
	p_impl(new VMXPiImpl(realtime)),
	ahrs(p_impl->spi, p_impl->pigpio, ahrs_update_rate_hz),
	time(p_impl->pigpio, p_impl->misc),
	io(p_impl->pigpio, p_impl->iocx, p_impl->misc, p_impl->chan_mgr, p_impl->rsrc_mgr, time),
	can(p_impl->can),
	power(p_impl->misc),
	version(ahrs)
{
	VMXChannelManager::Init();
}

VMXPi::~VMXPi() {
	version.ReleaseResources();
	power.ReleaseResources();
	can.ReleaseResources();
	io.ReleaseResources();
	time.ReleaseResources();
	can.ReleaseResources();
	ahrs.Stop();	/* Stop AHRS Data IO Thread */
	delete p_impl;
}

bool VMXPi::IsOpen() {
	return p_impl->pigpio.IsOpen();
}

