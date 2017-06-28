/*
 * NavXSPIMessage.h
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#ifndef NAVXSPIMESSAGE_H_
#define NAVXSPIMESSAGE_H_

#include <stdint.h>

#define NAVXPI_SPI_MESSSAGE_LEN 8 /* sizeof(NavXSPIMessage) */
struct __attribute__ ((__packed__)) NavXSPIMessage
{
public:

	uint8_t bank;
	uint8_t reg_addr;
	uint8_t count;
	uint8_t data[4];
	uint8_t crc;

	static uint8_t crc_lookup_table[256];
	static void init_crc_table();
	static uint8_t get_standard_packet_size();

public:
	/* Build a Read request message */
	NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count);
	/* Build a (fixed-length) Write request message */
	NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count, uint8_t* p_values);
	virtual ~NavXSPIMessage() {}
	static bool validate_read_response(uint8_t *p_data, uint8_t len);
	virtual uint8_t *get_packet_ptr() { return &this->bank; }
	virtual uint8_t get_packet_size() { return NAVXPI_SPI_MESSSAGE_LEN; };
protected:
	NavXSPIMessage() {
		bank = 0;
		reg_addr = 0;
		count = 0;
		crc = 0;
	}
};

#endif /* NAVXSPIMESSAGE_H_ */
