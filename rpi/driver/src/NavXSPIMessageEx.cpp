/*
 * NavXSPIMessage.cpp
 *
 *  Created on: Jan 8, 2017
 *      Author: Scott
 */

#include <assert.h>
#include <string.h>
#include "NavXSPIMessageEx.h"
#include "IMURegisters.h"

/* Build write message (single-byte count allowed for Bank 0; variable number of bytes in length for other banks. */
NavXSPIMessageEx::NavXSPIMessageEx(uint8_t bank, uint8_t reg_addr, uint8_t count, uint8_t* p_values) {
	if(bank == 0) {
		assert(count <= 1);
	} else {
		assert(count <= (MAX_SPI_MSG_LEN - TOTAL_SPI_MSG_OVERHEAD_BYTES)); /* Max Transfer size minus non-data bytes */
	}

	p_variable_len_message = (uint8_t *)new uint8_t[count + TOTAL_SPI_MSG_OVERHEAD_BYTES];

	p_variable_len_message[0] = bank;
	p_variable_len_message[1] = 0x80 |reg_addr;
	memset(p_variable_len_message+3,0xFF,sizeof(this->data));
	if (bank == 0) {
		p_variable_len_message[2] = p_values[0];
	} else {
		p_variable_len_message[2] = count;
		memcpy(p_variable_len_message+3,p_values,count);
	}
	p_variable_len_message[count+3] = IMURegisters::getCRCWithTable(NavXSPIMessage::crc_lookup_table, p_variable_len_message, count+3);
}

