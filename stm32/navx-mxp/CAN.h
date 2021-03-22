/*
 * CAN.h
 *
 *  Created on: Jan 21, 2017
 *      Author: Scott
 */

#ifndef CAN_H_
#define CAN_H_

#include "CANRegisters.h"

#ifdef __cplusplus
#define _EXTERN_ATTRIB extern "C"
#else
#define _EXTERN_ATTRIB
#endif

#include <stdint.h>

_EXTERN_ATTRIB void CAN_init();
_EXTERN_ATTRIB void CAN_loop();
_EXTERN_ATTRIB uint8_t *CAN_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint8_t requested_count, uint16_t* size );
_EXTERN_ATTRIB void CAN_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values );

// Local access functions
_EXTERN_ATTRIB uint8_t CAN_get_rx_fifo_entry_count();
_EXTERN_ATTRIB uint8_t CAN_get_tx_fifo_entry_count();
_EXTERN_ATTRIB int CAN_get_next_rx_transfer(TIMESTAMPED_CAN_TRANSFER *p_rx_transfer); // Returns 0 if p_rx_transfer contains new data
_EXTERN_ATTRIB int CAN_add_tx_transfer(CAN_TRANSFER *transfer_to_transmit); // Returns 0 if successful
_EXTERN_ATTRIB void CAN_set_opmode(CAN_MODE mode);
_EXTERN_ATTRIB void CAN_command(uint8_t command);
_EXTERN_ATTRIB void CAN_set_buffer_filter_mode(uint8_t rx_buffer_index, CAN_RX_FILTER_MODE filter_mode); // Valid Indices 0-1
_EXTERN_ATTRIB void CAN_set_filter(uint8_t rx_buffer_index, uint8_t rx_filter_index, CAN_ID filter); // Valid Indices 0.0;0.1;1.0;1.1;1.2;1.3
_EXTERN_ATTRIB void CAN_set_mask(uint8_t rx_buffer_index, CAN_ID mask); // Valid Indices 0-1

#endif /* CAN_H_ */
