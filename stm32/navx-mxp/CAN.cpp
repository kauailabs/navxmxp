/*
 * CAN.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: Scott
 */

#include "CAN.h"
#include "CANInterface.h"
#include "FIFO.h"
#include <string.h>

static CANInterface *p_CAN;

_EXTERN_ATTRIB void CAN_init()
{
	p_CAN = new CANInterface();
	p_CAN->init(CAN_MODE_LOOP);

	CAN_pack_extended_id(0x1234, &p_CAN->get_own_id());
	p_CAN->get_own_id().sidl.ide = true;
	p_CAN->get_own_id().sidl.srr = false;
}

static uint32_t last_loop_timestamp;
#define NUM_MS_BETWEEN_SUCCESSIVE_LOOPS 2

_EXTERN_ATTRIB void CAN_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();
	if ( curr_loop_timestamp < last_loop_timestamp) {
		/* Timestamp rollover */
	} else {
		if ((curr_loop_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_LOOPS){
			last_loop_timestamp = curr_loop_timestamp;

			while(!p_CAN->get_rx_fifo().is_empty()){
				CAN_TRANSFER *p_rx = p_CAN->get_rx_fifo().dequeue();
				if(p_rx){
					uint32_t id;
					if(p_rx->id.sidl.ide){
						CAN_unpack_extended_id(&p_rx->id,&id);
					} else {
						CAN_unpack_standard_id(&p_rx->id,&id);
					}
					uint8_t len = p_rx->payload.dlc.len;
					uint8_t buff[8];
					memcpy(buff,p_rx->payload.buff,len);
					bool RTR = p_rx->payload.dlc.rtr;
				}
			}

			CAN_ERROR_FLAGS errors;
			uint8_t tx_err_cnt;
			uint8_t rx_err_cnt;
			bool unclearable_error;

			if(p_CAN->get_errors(errors, tx_err_cnt, rx_err_cnt)){
				if(errors.rx_overflow) {
					p_CAN->clear_rx_overflow();
				}
			}

			while (!p_CAN->get_tx_fifo().is_full()) {
				CAN_DATA *p_tx = p_CAN->get_tx_fifo().enqueue_reserve();
				if(p_tx) {
					*(uint8_t *)&(p_tx->dlc) = 0;
					p_tx->dlc.len = 8;
					p_tx->dlc.rtr = false;
					p_tx->buff[0] = 'H';
					p_tx->buff[1] = 'e';
					p_tx->buff[2] = 'l';
					p_tx->buff[3] = 'l';
					p_tx->buff[4] = 'o';
					p_tx->buff[5] = '!';
					p_tx->buff[6] = '!';
					p_tx->buff[7] = 0;
					p_CAN->get_tx_fifo().enqueue_commit(p_tx);
				}
			}
		}
	}
	p_CAN->process_transmit_fifo();
}

_EXTERN_ATTRIB uint8_t *CAN_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint16_t* size )
{
	return 0;
}

_EXTERN_ATTRIB void CAN_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values )
{
}

