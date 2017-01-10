/*
 * NavXSPIMessage.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#include <stdint.h>
#include <string.h>
#include "NavXSPIMessage.h"
#include "IMURegisters.h"

uint8_t NavXSPIMessage::crc_lookup_table[256];

NavXSPIMessage::NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count) {
	this->bank = bank;
	this->reg_addr = reg_addr;
	this->count = count;
	memset(this->data,0,sizeof(this->data));
	crc = IMURegisters::getCRCWithTable(NavXSPIMessage::crc_lookup_table, &this->bank, NAVXPI_SPI_MESSSAGE_LEN-1);
}
/* Build write message (single-byte for Bank 0; 1-16 bytes in length for other banks. */
NavXSPIMessage::NavXSPIMessage(uint8_t bank, uint8_t reg_addr, uint8_t count, uint8_t* p_values) {
	this->bank = bank;
	this->reg_addr = 0x80 |reg_addr;
	if (bank == 0) {
		this->count = p_values[0];
		memset(this->data,0,sizeof(this->data));
	} else {
		this->count = count;
		memcpy(this->data,p_values,count);
	}
	crc = IMURegisters::getCRCWithTable(NavXSPIMessage::crc_lookup_table, &this->bank, NAVXPI_SPI_MESSSAGE_LEN-1);
}

void NavXSPIMessage::init_crc_table() {
    IMURegisters::buildCRCLookupTable(NavXSPIMessage::crc_lookup_table, sizeof(crc_lookup_table));
}

bool NavXSPIMessage::validate_read_response(uint8_t *p_data, uint8_t len) {
	return (p_data[len-1] == IMURegisters::getCRCWithTable(NavXSPIMessage::crc_lookup_table, p_data, len-1));
}
