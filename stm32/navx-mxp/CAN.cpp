/*
 * CAN.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: Scott
 */

#include "CAN.h"
#include "CANInterface.h"
#include "FIFO.h"
#include "navx-mxp_hal.h"
#include <string.h>

static CANInterface *p_CAN;

#define MAX_SPI_READ_DATA_SIZE   	 254 /* Plus one byte for CRC */
#define READ_BUFFER_TRANSFER_COUNT	 MAX_SPI_READ_DATA_SIZE / sizeof(CAN_TRANSFER)

CAN_REGS can_regs;
CAN_TRANSFER read_buffer[READ_BUFFER_TRANSFER_COUNT];

void CAN_ISR_Flag_Function(CAN_IFX_INT_FLAGS mask, CAN_IFX_INT_FLAGS flags)
{
	can_regs.rx_fifo_entry_count = p_CAN->get_rx_fifo().get_count();

	uint8_t umask = (uint8_t)*(uint8_t *)&mask;
	uint8_t uflags = (uint8_t)*(uint8_t *)&flags;
	uint8_t masked_flags = umask & uflags;
	*(uint8_t *)(&can_regs.int_status) |= masked_flags;
	can_regs.int_flags = can_regs.int_status;
	uint8_t curr_flags = *(uint8_t *)(&can_regs.int_status);
	uint8_t curr_enable_mask = *(uint8_t *)(&can_regs.int_enable);
	if((curr_flags & curr_enable_mask) != 0) {
		HAL_RPI_CAN_Int_Assert();
	}
}

_EXTERN_ATTRIB void CAN_init()
{
	can_regs.capability_flags.can_2_0b = true;
	can_regs.capability_flags.unused = 0;
	p_CAN = new CANInterface();
	p_CAN->register_interrupt_flag_function(CAN_ISR_Flag_Function); /* Updated in ISR */
	p_CAN->init(CAN_MODE_LISTEN);
}

static uint32_t last_loop_timestamp;
static uint32_t last_overflow_check_timestamp;
#define NUM_MS_BETWEEN_SUCCESSIVE_LOOPS 1
#define NUM_MS_BETWEEN_SUCCESSIVE_OVERFLOW_CHECKS 10

int num_received_messages = 0;
int num_rx_overflows = 0;

_EXTERN_ATTRIB void CAN_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();
	if ( curr_loop_timestamp < last_loop_timestamp) {
		/* Timestamp rollover */
		last_loop_timestamp = curr_loop_timestamp;
		last_overflow_check_timestamp = curr_loop_timestamp;
	} else {
		if ((curr_loop_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_LOOPS){
			last_loop_timestamp = curr_loop_timestamp;

			CAN_IFX_INT_FLAGS CAN_loop_int_mask, CAN_loop_int_flags = {0};
			CAN_loop_int_mask.tx_fifo_empty = true;

			can_regs.tx_fifo_entry_count = p_CAN->get_tx_fifo().get_count();
			CAN_loop_int_flags.tx_fifo_empty = (can_regs.tx_fifo_entry_count == 0);

			if(!CAN_loop_int_flags.tx_fifo_empty) {
				p_CAN->process_transmit_fifo();
			}

			while(!p_CAN->get_rx_fifo().is_empty()){
				CAN_TRANSFER_PADDED *p_rx = p_CAN->get_rx_fifo().dequeue();
				if(p_rx){
					num_received_messages++;
					uint32_t id;
					if(p_rx->transfer.id.sidl.ide){
						CAN_unpack_extended_id(&p_rx->transfer.id,&id);
					} else {
						CAN_unpack_standard_id(&p_rx->transfer.id,&id);
					}
					uint8_t len = p_rx->transfer.payload.dlc.len;
					uint8_t buff[8];
					int copy_len = len;
					if ( copy_len > 8) {
						copy_len = 8;
					}

					memcpy(buff,p_rx->transfer.payload.buff,copy_len);
					bool RTR = p_rx->transfer.payload.dlc.rtr;
					p_CAN->get_rx_fifo().dequeue_return(p_rx);
					uint8_t len_copy = len;
				}
			}

			/* If CAN Rx is not currently active (because of inbound buffer
			 * overflow), re-enable reception.
			 */

			if ((curr_loop_timestamp - last_overflow_check_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_OVERFLOW_CHECKS){
				last_overflow_check_timestamp = curr_loop_timestamp;
				CAN_loop_int_mask.hw_rx_overflow = true;
				CAN_loop_int_flags.hw_rx_overflow = false;
				bool rx_overflow;

				if(p_CAN->get_errors(rx_overflow, can_regs.bus_error_flags, can_regs.tx_err_count, can_regs.rx_err_count)){
					if(rx_overflow) {
						CAN_loop_int_flags.hw_rx_overflow = true;
						/* Note:  This condition must be cleared in order to
						 * receive additional data from CAN bus.
						 */
					}
				}
			}

			CAN_ISR_Flag_Function(CAN_loop_int_mask, CAN_loop_int_flags);
#if 0
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
#endif
		}
	}
}

static uint8_t * CAN_rxfifo_read(uint8_t requested_count, uint16_t* size) {
	int num_transfers_in_buffer = 0;
	uint8_t *read_buffer_base = (uint8_t *)&read_buffer;
	if(requested_count >= sizeof(CAN_TRANSFER)) {
		int num_requested_transfers = sizeof(CAN_TRANSFER) / requested_count;
		for ( int i = 0; i < num_requested_transfers; i++) {
			CAN_TRANSFER_PADDED *p_rx = p_CAN->get_rx_fifo().dequeue();
			if(p_rx != NULL) {
				memcpy(&read_buffer[i], &p_rx->transfer, sizeof(p_rx->transfer));
				p_CAN->get_rx_fifo().dequeue_return(p_rx);
				num_transfers_in_buffer++;
			} else {
				break; /* Fifo now empty */
			}
		}
		if(p_CAN->get_rx_fifo().is_empty()) {
			/* TODO:  Deassert rx_fifo_nonempty interrupt flag */
		}
	}
	if (num_transfers_in_buffer == 0) {
		/* Fifo empty when read request received */
		/* Transmit a single transfer w/invalid flag set. */
		read_buffer[0].id.sidl.invalid = true;
		num_transfers_in_buffer++;
	}
	*size = num_transfers_in_buffer * sizeof(CAN_TRANSFER);
	return read_buffer_base;
}

_EXTERN_ATTRIB uint8_t *CAN_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint8_t requested_count, uint16_t* size )
{
	if ( bank == CAN_REGISTER_BANK) {
	    if ( register_offset >= offsetof(struct CAN_REGS, end_of_bank) ) {
	        size = 0;
	        return 0;
	    }
	    if(register_offset == offsetof(struct CAN_REGS, rx_fifo_tail)) {
	    	return CAN_rxfifo_read(requested_count, size);
	    } else {
	    	uint8_t *register_base = (uint8_t *)&can_regs;
	    	*size = offsetof(struct CAN_REGS, end_of_bank) - register_offset;
	    	return register_base + register_offset;
	    }
	}
	return 0;
}

static void CAN_txfifo_write(uint8_t *p_data, uint8_t count) {
	if(count >= sizeof(CAN_TRANSFER)) {
		int num_requested_transfers = sizeof(CAN_TRANSFER) / count;
		for ( int i = 0; i < num_requested_transfers; i++) {
			CAN_TRANSFER_PADDED *p_tx = p_CAN->get_tx_fifo().enqueue_reserve();
			if(p_tx != NULL) {
				memcpy(&p_tx->transfer, p_data, sizeof(p_tx->transfer));
				p_CAN->get_tx_fifo().enqueue_commit(p_tx);
			} else {
				break; /* Fifo now full */
			}
		}
	}
}

static void CAN_int_enable_modified(uint8_t first_offset, uint8_t count) {
	/* No-op */
}

static void CAN_int_flags_modified(uint8_t first_offset, uint8_t count) {
	if(can_regs.int_flags.hw_rx_overflow){
		p_CAN->clear_rx_overflow();
		can_regs.int_flags.hw_rx_overflow = false;
	}
	can_regs.int_status = can_regs.int_flags;
	uint8_t curr_int_status = *(uint8_t *)&can_regs.int_status;
	uint8_t curr_int_enable = *(uint8_t *)&can_regs.int_enable;
	if((curr_int_status & curr_int_enable) == 0) {
		HAL_RPI_CAN_Int_Deassert();
	}
}

static void CAN_opmode_modified(uint8_t first_offset, uint8_t count) {
	if ((first_offset == 0) && (count > 0)) {
		p_CAN->set_mode(can_regs.opmode);
	}
}

static void CAN_reset_modified(uint8_t first_offset, uint8_t count) {

}

static void CAN_accept_mask_rxb0_modified(uint8_t first_offset, uint8_t count) {
	p_CAN->mask_config(RXB0, &can_regs.accept_mask_rxb0);
}

static void CAN_accept_mask_rxb1_modified(uint8_t first_offset, uint8_t count) {
	p_CAN->mask_config(RXB1, &can_regs.accept_mask_rxb1);
}

static MCP25625_RX_FILTER_INDEX rxb0_filter_indices[NUM_ACCEPT_FILTERS_RX0_BUFFER] =
{
	RXF_0,
	RXF_1
};

static MCP25625_RX_FILTER_INDEX rxb1_filter_indices[NUM_ACCEPT_FILTERS_RX1_BUFFER] =
{
	RXF_2,
	RXF_3,
	RXF_4,
	RXF_5
};

static void CAN_accept_filter_rxb0_modified(uint8_t first_offset, uint8_t count) {
	int curr_offset = 0;
	int last_offset = first_offset + count - 1;
	for ( size_t i = 0; i < SIZEOF_STRUCT(rxb0_filter_indices); i++) {
		curr_offset = i * sizeof(CAN_ID);
		if ((curr_offset >= first_offset) &&
			(curr_offset <= last_offset)){
			p_CAN->filter_config(rxb0_filter_indices[i], &can_regs.accept_filter_rxb0[i]);
		}
	}
}

static void CAN_accept_filter_rxb1_modified(uint8_t first_offset, uint8_t count) {
	int curr_offset = 0;
	int last_offset = first_offset + count - 1;
	for ( size_t i = 0; i < SIZEOF_STRUCT(rxb1_filter_indices); i++) {
		curr_offset = i * sizeof(CAN_ID);
		if ((curr_offset >= first_offset) &&
			(curr_offset <= last_offset)){
			p_CAN->filter_config(rxb1_filter_indices[i], &can_regs.accept_filter_rxb1[i]);
		}
	}
}

static void CAN_advanced_config_modified(uint8_t first_offset, uint8_t count) {

}

WritableRegSet CAN_reg_sets[] =
{
	/* Contiguous registers, increasing order of offset  */
	{ offsetof(struct CAN_REGS, int_enable), sizeof(CAN_REGS::int_enable), CAN_int_enable_modified },
	{ offsetof(struct CAN_REGS, int_flags), sizeof(CAN_REGS::int_flags), CAN_int_flags_modified },
	{ offsetof(struct CAN_REGS, opmode), sizeof(CAN_REGS::opmode), CAN_opmode_modified },
	{ offsetof(struct CAN_REGS, reset), sizeof(CAN_REGS::reset), CAN_reset_modified },
	{ offsetof(struct CAN_REGS, accept_mask_rxb0), sizeof(CAN_REGS::accept_mask_rxb0), CAN_accept_mask_rxb0_modified },
	{ offsetof(struct CAN_REGS, accept_mask_rxb1), sizeof(CAN_REGS::accept_mask_rxb1), CAN_accept_mask_rxb1_modified },
	{ offsetof(struct CAN_REGS, accept_filter_rxb0), sizeof(CAN_REGS::accept_filter_rxb0), CAN_accept_filter_rxb0_modified },
	{ offsetof(struct CAN_REGS, accept_filter_rxb1), sizeof(CAN_REGS::accept_filter_rxb1), CAN_accept_filter_rxb1_modified },
	{ offsetof(struct CAN_REGS, advanced_config), sizeof(CAN_REGS::advanced_config), CAN_advanced_config_modified },
};

WritableRegSetGroup CAN_writable_reg_set_groups[] =
{
	{ CAN_reg_sets[0].start_offset,
		CAN_reg_sets[SIZEOF_STRUCT(CAN_reg_sets)-1].start_offset + CAN_reg_sets[SIZEOF_STRUCT(CAN_reg_sets)-1].num_bytes,
		CAN_reg_sets,
		SIZEOF_STRUCT(CAN_reg_sets) },
};

_EXTERN_ATTRIB void CAN_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values )
{
	if(bank==CAN_REGISTER_BANK){
		if(reg_offset == offsetof(struct CAN_REGS, tx_fifo_head)) {
			/* Perform write to transmit fifo, as long as it is not full. */
			CAN_txfifo_write(p_new_values, count);
		} else {
			for ( size_t i = 0; i < SIZEOF_STRUCT(CAN_writable_reg_set_groups); i++) {
				if ( (reg_offset >= CAN_writable_reg_set_groups[i].first_offset) &&
					 (reg_offset <= CAN_writable_reg_set_groups[i].last_offset)) {
					for ( int j = 0; j < CAN_writable_reg_set_groups[i].set_count; j++) {
						WritableRegSet *p_set = &CAN_writable_reg_set_groups[i].p_sets[j];
						if ((reg_offset >= p_set->start_offset) &&
							(reg_offset < (p_set->start_offset + p_set->num_bytes))){
							int first_offset = (reg_offset - p_set->start_offset);
							int max_bytes_to_write_in_set = p_set->num_bytes-first_offset;
							int num_bytes_in_set_to_modify = (count < max_bytes_to_write_in_set) ? count : max_bytes_to_write_in_set;
							int num_bytes_in_set_changed = 0;
							while (num_bytes_in_set_changed < num_bytes_in_set_to_modify )  {
								*p_reg++ = *p_new_values++;
								reg_offset++;
								num_bytes_in_set_changed++;
								count--;
							}
							if (num_bytes_in_set_changed > 0){
								/* At least one byte in this set was modified. */
								p_set->changed(first_offset, num_bytes_in_set_changed);
							}
							if (count == 0) {
								break;
							}
						}
					}
				}
			}
		}
	}
}

