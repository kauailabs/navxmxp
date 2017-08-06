/*
 * VMXCANReceiveStream.cpp
 *
 *  Created on: 4 Aug 2017
 *      Author: pi
 */

#include "VMXCANReceiveStream.h"

VMXCANReceiveStream::VMXCANReceiveStream(uint32_t id, uint32_t mask, uint8_t rxb_id, uint8_t rxbfilter_id, uint32_t NumMessages)
{
	p_rx_data = new boost::circular_buffer<VMXCANTimestampedMessage>(NumMessages);
	CANIDFilter = id;
	CANIDMask = mask;
	this->rxb_id = rxb_id;
	this->rxb_filter_id = rxb_filter_id;
}


VMXCANReceiveStream::~VMXCANReceiveStream() {
	delete p_rx_data;
}


