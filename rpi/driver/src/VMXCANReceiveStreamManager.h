/*
 * VMXCANReceiveStreamManager.h
 *
 *  Created on: 4 Aug 2017
 *      Author: pi
 */

#ifndef VMXCANRECEIVESTREAMMANAGER_H_
#define VMXCANRECEIVESTREAMMANAGER_H_

#include "VMXCANReceiveStream.h"
#include <map>

typedef struct {
	size_t		num_filters_in_use;
	uint32_t	mask;
	uint32_t*	p_filters;
	size_t		num_filters;

	void Init(uint32_t *p_filters, uint32_t filter_count) {
		this->p_filters = p_filters;
		for (uint32_t i = 0; i < filter_count; i++) {
			p_filters[i] = 0xFFFFFFFF;
		}
		num_filters = filter_count;
		num_filters_in_use = 0;
		mask = 0xFFFFFFFF;
	}
} CANRXBFilterConfig;

class VMXCANReceiveStreamManager {
	std::map<uint32_t, VMXCANReceiveStream *> stream_map;

	uint32_t next_stream_id;

	uint32_t RXB0_Filters[2];
	uint32_t RXB1_Filters[4];

	CANRXBFilterConfig filter_configs[2];
public:
	VMXCANReceiveStreamManager();

	std::map<uint32_t, VMXCANReceiveStream *>& GetStreamMap() { return stream_map; }

	bool ReserveFilter(uint32_t filter, uint32_t mask, uint8_t& rxb_index, uint8_t& rxb_filter_index, bool& already_exists) {
		already_exists = false;
		for (size_t i = 0; i < (sizeof(filter_configs)/sizeof(filter_configs[0])); i++) {
			if (!filter_configs[i].num_filters_in_use) {
				rxb_index = i;
				rxb_filter_index = filter_configs[i].num_filters_in_use++;

				filter_configs[i].mask = mask;
				filter_configs[i].p_filters[rxb_filter_index] = filter;
				return true;
			} else {
				if (filter_configs[i].mask == mask) {
					for (size_t j = 0; j < filter_configs[i].num_filters; j++) {
						if (filter_configs[i].p_filters[j] == filter) {
							already_exists = true;
							rxb_index = i;
							rxb_filter_index = j;
							return true;
						}
					}
					if (filter_configs[i].num_filters_in_use < filter_configs[i].num_filters) {
						rxb_index = i;
						rxb_filter_index = filter_configs[i].num_filters_in_use++;
						filter_configs[i].p_filters[rxb_filter_index] = filter;
						return true;
					}
				}
			}
		}
		return false;
	}

	bool UnreserveFilter(uint8_t rxb_index, uint8_t rxb_filter_index, uint8_t& remaining_num_filters_in_use) {
		if (rxb_index >= (sizeof(filter_configs)/sizeof(filter_configs[0]))) return false;
		if (rxb_filter_index >= filter_configs[rxb_index].num_filters_in_use) return false;
		filter_configs[rxb_index].p_filters[rxb_filter_index] = 0xFFFFFFFF;
		if (filter_configs[rxb_index].num_filters_in_use > 0) {
			filter_configs[rxb_index].num_filters_in_use--;
		}
		if (filter_configs[rxb_index].num_filters_in_use == 0) {
			filter_configs[rxb_index].mask = 0xFFFFFFFF;
		}
		remaining_num_filters_in_use = filter_configs[rxb_index].num_filters_in_use;
		return true;
	}


	bool AddStream(VMXCANReceiveStream *p_stream, uint32_t& streamid) {
		streamid = next_stream_id;
		stream_map[streamid] = p_stream;
		next_stream_id++;
		return true;
	}

	VMXCANReceiveStream *GetStream(uint32_t streamid) {
		std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
		it = stream_map.find(streamid);
		if (it != stream_map.end()) {
			return it->second;
		}
		return 0;
	}

	bool GetStreamForMessageID(uint32_t messageID, uint32_t& stream_id) {
		std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
		for (it = stream_map.begin(); it != stream_map.end(); it++) {
			if (it->second->IDMatches(messageID)) {
				stream_id = it->first;
				return true;
			}
		}
		return false;
	}

	VMXCANReceiveStream *GetStreamForMessageID(uint32_t messageID) {
		std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
		for (it = stream_map.begin(); it != stream_map.end(); it++) {
			if (it->second->IDMatches(messageID)) {
				return it->second;
			}
		}
		return 0;
	}

	VMXCANReceiveStream *GetStreamForCAN_ID(CAN_ID& canid) {
		std::map<uint32_t, VMXCANReceiveStream *>::iterator it;
		for (it = stream_map.begin(); it != stream_map.end(); it++) {
			if (it->second->IDMatches(canid)) {
				return it->second;
			}
		}
		return 0;
	}

	bool RemoveStream(uint32_t streamid) {
		VMXCANReceiveStream *p_stream = GetStream(streamid);
		if (!p_stream) return false;
		delete p_stream;
		stream_map.erase(streamid);
		return true;
	}

	virtual ~VMXCANReceiveStreamManager();
};

#endif /* VMXCANRECEIVESTREAMMANAGER_H_ */
