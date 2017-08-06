/*
 * VMXCANReceiveStreamManager.cpp
 *
 *  Created on: 4 Aug 2017
 *      Author: pi
 */

#include "VMXCANReceiveStreamManager.h"

VMXCANReceiveStreamManager::VMXCANReceiveStreamManager() {
	filter_configs[0].Init(RXB0_Filters, (sizeof(RXB0_Filters)/sizeof(RXB0_Filters[0])));
	filter_configs[1].Init(RXB1_Filters, (sizeof(RXB1_Filters)/sizeof(RXB1_Filters[0])));
	next_stream_id = 0;
}


VMXCANReceiveStreamManager::~VMXCANReceiveStreamManager() {
	std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
	for (it = stream_map.begin(); it != stream_map.end(); it++) {
		delete it->second;
	}
	stream_map.clear();
}
