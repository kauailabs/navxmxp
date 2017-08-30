/*
 * NavXSPIMessageEx.h
 *
 *  Created on: Jun 24, 2017
 *      Author: Scott
 */

#ifndef NAVXSPIMESSAGEEX_H_
#define NAVXSPIMESSAGEEX_H_

#include <stdint.h>
#include "NavXSPIMessage.h"

struct __attribute__ ((__packed__)) NavXSPIMessageEx : public NavXSPIMessage
{
public:

	uint8_t *p_variable_len_message;

public:
	/* Build a (variable-length) Write request message */
	NavXSPIMessageEx(uint8_t bank, uint8_t reg_addr, uint8_t count, uint8_t* p_values);
	~NavXSPIMessageEx() { delete p_variable_len_message; }
	virtual uint8_t *get_packet_ptr() { return p_variable_len_message; }
	virtual uint8_t get_packet_size() { return p_variable_len_message[2] + 4; };
};

#endif /* NAVXSPIMESSAGE_H_ */
