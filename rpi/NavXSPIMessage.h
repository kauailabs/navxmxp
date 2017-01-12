/*
 * NavXSPIMessage.h
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#ifndef NAVXSPIMESSAGE_H_
#define NAVXSPIMESSAGE_H_

#define NAVXPI_SPI_MESSSAGE_LEN 20 /* sizeof(NavXSPIMessage) */
struct __attribute__ ((__packed__)) NavXSPIMessage
{
public:

	uint8_t bank;
	uint8_t reg_addr;
	uint8_t count;
	uint8_t data[16];
	uint8_t crc;

	static uint8_t crc_lookup_table[256];
	static void init_crc_table();

public:
	/* Build a read request message */
	NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count, uint8_t* p_values);
	/* Validate Read Response */
	NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count);
	static bool validate_read_response(uint8_t *p_data, uint8_t len);
	uint8_t *get_packet_ptr() { return &this->bank; }
	uint8_t get_packet_size() { return NAVXPI_SPI_MESSSAGE_LEN; };
};

#endif /* NAVXSPIMESSAGE_H_ */
