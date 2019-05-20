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
TIMESTAMPED_CAN_TRANSFER read_buffer[READ_BUFFER_TRANSFER_COUNT];

void enable_SPI_interrupts() {
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

void disable_SPI_interrupts() {
    HAL_NVIC_DisableIRQ(SPI1_IRQn);
}

#define CAN_DEVICE_UPDATE_RESET			0x00000001
#define CAN_DEVICE_UPDATE_DEVICE_MODE	0x00000002
#define CAN_DEVICE_UPDATE_MODE_0		0x00000004
#define CAN_DEVICE_UPDATE_MODE_1		0x00000008
#define CAN_DEVICE_UPDATE_MASK_0		0x00000010
#define CAN_DEVICE_UPDATE_MASK_1		0x00000020
#define CAN_DEVICE_UPDATE_FILTER_0		0x00000040
#define CAN_DEVICE_UPDATE_FILTER_1		0x00000080
#define CAN_DEVICE_UPDATE_FILTER_2		0x00000100
#define CAN_DEVICE_UPDATE_FILTER_3		0x00000200
#define CAN_DEVICE_UPDATE_FILTER_4		0x00000400
#define CAN_DEVICE_UPDATE_FILTER_5		0x00000800
#define CAN_DEVICE_UPDATE_FLUSH_RXFIFO	0x00001000
#define CAN_DEVICE_UPDATE_FLUSH_TXFIFO	0x00002000
#define CAN_DEVICE_UPDATE_CLEAR_RXOVRF	0x00004000 // TODO:  Rview this, it appears incorrect! (should be 0000004000)
#define CAN_DEVICE_UPDATE_RESET_500KBPS	0x00008000 // TODO:  Needs to be tested
#define CAN_DEVICE_UPDATE_RESET_250KBPS 0x00010000 // TODO:  Needs to be tested

static volatile uint32_t can_pending_updates = 0;

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

/* NOTE:  This function should not ever be invoked in such a way that it */
/* can be re-entered.                                                    */
void CAN_ISR_Flag_Function(CAN_IFX_INT_FLAGS mask, CAN_IFX_INT_FLAGS flags)
{
	can_regs.rx_fifo_entry_count = p_CAN->get_rx_fifo().get_count();

	uint8_t umask = (uint8_t)*(uint8_t *)&mask;
	uint8_t uflags = (uint8_t)*(uint8_t *)&flags;
	uint8_t masked_flags_to_set = uflags & umask;
    *(uint8_t *)(&can_regs.int_flags) &= ~umask; /* Clear masked bits */
	*(uint8_t *)(&can_regs.int_flags) |= masked_flags_to_set; /* Set bits */
	can_regs.int_status = can_regs.int_flags; /* Update shadow of flags */
	uint8_t curr_flags = *(uint8_t *)(&can_regs.int_flags);
	uint8_t curr_enable_mask = *(uint8_t *)(&can_regs.int_enable);
	if((curr_flags & curr_enable_mask) != 0) {
		HAL_CAN_Int_Assert();
	} else {
		HAL_CAN_Int_Deassert();
	}
}

static void CAN_int_flags_modified(uint8_t first_offset, uint8_t count) {
	if(can_regs.int_flags.hw_rx_overflow){
		can_pending_updates |= CAN_DEVICE_UPDATE_CLEAR_RXOVRF;
		can_regs.int_flags.hw_rx_overflow = false;
	}
	can_regs.int_status = can_regs.int_flags;
	uint8_t curr_flags = *(uint8_t *)&can_regs.int_flags;
	uint8_t curr_enable_mask = *(uint8_t *)&can_regs.int_enable;
	if((curr_flags & curr_enable_mask) == 0) {
		HAL_CAN_Int_Deassert();
	}
}

_EXTERN_ATTRIB void CAN_init()
{
	can_regs.capability_flags.can_2_0b = true;
	can_regs.capability_flags.set_bitrate = true;
	can_regs.capability_flags.us_timestamp = true;
	can_regs.capability_flags.unused = 0;
	p_CAN = new CANInterface();
	p_CAN->register_interrupt_flag_function(CAN_ISR_Flag_Function); /* Updated in ISR */
	p_CAN->init(CAN_MODE_NORMAL, Mbps_1); /* Default to 1Mbps bitrate. */
	HAL_CAN_Status_LED_On(0);
}

static uint32_t last_loop_timestamp = 0;
static uint32_t last_can_bus_error_check_timestamp = 0;
#define NUM_MS_BETWEEN_SUCCESSIVE_LOOPS 1
#define NUM_MS_BETWEEN_SUCCESSIVE_CAN_BUS_ERROR_CHECKS 50

static void CAN_process_pending_updates()
{
	if (can_pending_updates) {

		/* Transfer pending requests to local memory storage,
		 * and while doing so disable further SPI interrupts.
		 * This code needs to execute very quickly.
		 */

		CAN_REGS can_regs_pending;
		disable_SPI_interrupts();
		uint32_t pending_updates = can_pending_updates;
		if (pending_updates & CAN_DEVICE_UPDATE_DEVICE_MODE) {
			can_regs_pending.opmode = can_regs.opmode;
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MODE_0) {
			can_regs_pending.rx_filter_mode[0] =
				can_regs.rx_filter_mode[0];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MODE_1) {
			can_regs_pending.rx_filter_mode[1] =
				can_regs.rx_filter_mode[1];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MASK_0) {
			can_regs_pending.accept_mask_rxb0 =
				can_regs.accept_mask_rxb0;
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MASK_1) {
			can_regs_pending.accept_mask_rxb1 =
				can_regs.accept_mask_rxb1;
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_0) {
			can_regs_pending.accept_filter_rxb0[0] =
				can_regs.accept_filter_rxb0[0];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_1) {
			can_regs_pending.accept_filter_rxb0[1] =
				can_regs.accept_filter_rxb0[1];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_2) {
			can_regs_pending.accept_filter_rxb1[0] =
				can_regs.accept_filter_rxb1[0];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_3) {
			can_regs_pending.accept_filter_rxb1[1] =
				can_regs.accept_filter_rxb1[1];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_4) {
			can_regs_pending.accept_filter_rxb1[2] =
				can_regs.accept_filter_rxb1[2];
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_5) {
			can_regs_pending.accept_filter_rxb1[3] =
				can_regs.accept_filter_rxb1[3];
		}
		can_pending_updates = 0;
		enable_SPI_interrupts();

		/* After re-enabling SPI interrupts, process the new requests */
		if (pending_updates & CAN_DEVICE_UPDATE_RESET) {
			p_CAN->init(p_CAN->get_current_can_mode(), Mbps_1);
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_RXFIFO;
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_TXFIFO;
		} else if (pending_updates & CAN_DEVICE_UPDATE_RESET_500KBPS) {
			p_CAN->init(p_CAN->get_current_can_mode(), Kbps_500);
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_RXFIFO;
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_TXFIFO;
		} else if (pending_updates & CAN_DEVICE_UPDATE_RESET_250KBPS) {
			p_CAN->init(p_CAN->get_current_can_mode(), Kbps_250);
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_RXFIFO;
			pending_updates |= CAN_DEVICE_UPDATE_FLUSH_TXFIFO;
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FLUSH_RXFIFO) {
			p_CAN->flush_rx_fifo();
			if (can_regs.int_flags.sw_rx_overflow) {
				can_regs.int_flags.sw_rx_overflow = false;
				CAN_int_flags_modified(0,0);
			}
			can_regs.rx_fifo_entry_count = p_CAN->get_rx_fifo().get_count();
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FLUSH_TXFIFO) {
			p_CAN->flush_tx_fifo();
			can_regs.int_flags.tx_fifo_empty = 1;
			CAN_int_flags_modified(0,0);
			can_regs.tx_fifo_entry_count = p_CAN->get_tx_fifo().get_count();
		}
		if (pending_updates & CAN_DEVICE_UPDATE_DEVICE_MODE) {
			CAN_MODE new_mode = (CAN_MODE)can_regs_pending.opmode;
			bool transition_from_config_mode = false;
			if ((p_CAN->get_current_can_mode() == CAN_MODE_CONFIG) &&
				(new_mode != CAN_MODE_CONFIG)) {
				transition_from_config_mode = true;
			}
			p_CAN->set_mode(new_mode);
			if (transition_from_config_mode) {
				/* Transitioning to/from CONFIG opmode from NORMAL mode may */
				/* set some errors that need clearing. */
				bool rx_overflow;
				p_CAN->get_errors(rx_overflow, can_regs.bus_error_flags, can_regs.tx_err_count, can_regs.rx_err_count);
				if (rx_overflow) {
					p_CAN->clear_rx_overflow();
				}
				p_CAN->clear_error_interrupt_flags();
			}
		}
		if (pending_updates & CAN_DEVICE_UPDATE_CLEAR_RXOVRF) {
			p_CAN->clear_rx_overflow(true);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MODE_0) {
			p_CAN->rx_config((MCP25625_RX_BUFFER_INDEX)0,
					(MCP25625_RX_MODE)can_regs_pending.rx_filter_mode[0],
					true);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MODE_1) {
			p_CAN->rx_config((MCP25625_RX_BUFFER_INDEX)1,
					(MCP25625_RX_MODE)can_regs_pending.rx_filter_mode[1],
					true);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MASK_0) {
			p_CAN->mask_config(RXB0, &can_regs_pending.accept_mask_rxb0, &can_regs.accept_mask_rxb0);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_MASK_1) {
			p_CAN->mask_config(RXB1, &can_regs_pending.accept_mask_rxb1, &can_regs.accept_mask_rxb1);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_0) {
			p_CAN->filter_config(rxb0_filter_indices[0], &can_regs_pending.accept_filter_rxb0[0],
					&can_regs.accept_filter_rxb0[0]);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_1) {
			p_CAN->filter_config(rxb0_filter_indices[1], &can_regs_pending.accept_filter_rxb0[1],
					&can_regs.accept_filter_rxb0[1]);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_2) {
			p_CAN->filter_config(rxb1_filter_indices[0], &can_regs_pending.accept_filter_rxb1[0],
					&can_regs.accept_filter_rxb1[0]);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_3) {
			p_CAN->filter_config(rxb1_filter_indices[1], &can_regs_pending.accept_filter_rxb1[1],
					&can_regs.accept_filter_rxb1[1]);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_4) {
			p_CAN->filter_config(rxb1_filter_indices[2], &can_regs_pending.accept_filter_rxb1[2],
					&can_regs.accept_filter_rxb1[2]);
		}
		if (pending_updates & CAN_DEVICE_UPDATE_FILTER_5) {
			p_CAN->filter_config(rxb1_filter_indices[3], &can_regs_pending.accept_filter_rxb1[3],
					&can_regs.accept_filter_rxb1[3]);
		}
	}
}

_EXTERN_ATTRIB void CAN_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();

	/* Process pending updates, this is a very high priority task. */
	CAN_process_pending_updates();

	if ( curr_loop_timestamp < last_loop_timestamp) {
		/* Timestamp rollover */
		last_loop_timestamp = curr_loop_timestamp;
		last_can_bus_error_check_timestamp = curr_loop_timestamp;
	} else {
		if ((curr_loop_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_LOOPS){
			last_loop_timestamp = curr_loop_timestamp;

			CAN_IFX_INT_FLAGS CAN_loop_int_mask, CAN_loop_int_flags = {0};
			*(uint8_t *)&CAN_loop_int_mask = 0;
			CAN_loop_int_mask.tx_fifo_empty = true;

			if(!CAN_loop_int_flags.tx_fifo_empty) {
				p_CAN->process_transmit_fifo();
				disable_SPI_interrupts();
				can_regs.tx_fifo_entry_count = p_CAN->get_tx_fifo().get_count();
				CAN_loop_int_flags.tx_fifo_empty = (can_regs.tx_fifo_entry_count == 0);
				enable_SPI_interrupts();
			}
			p_CAN->disable_CAN_interrupts();
			CAN_ISR_Flag_Function(CAN_loop_int_mask, CAN_loop_int_flags);
			p_CAN->enable_CAN_interrupts();
		}

		/* Retrieve current CAN bus TX/RX Error Counts. */

		if ((curr_loop_timestamp - last_can_bus_error_check_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_CAN_BUS_ERROR_CHECKS){
			last_can_bus_error_check_timestamp = curr_loop_timestamp;
			can_regs.bus_off_count = p_CAN->get_bus_off_count();
			CAN_IFX_INT_FLAGS CAN_loop_int_mask, CAN_loop_int_flags = {0};
			*(uint8_t *)&CAN_loop_int_mask = 0;
			CAN_MODE current_mode = p_CAN->get_current_can_mode();
			bool rx_overflow = false;
			if ((current_mode == CAN_MODE_NORMAL) ||
				(current_mode == CAN_MODE_LOOP) ||
				(current_mode == CAN_MODE_LISTEN)) {
				// Check if an overflow or error condition exists
				if (p_CAN->get_errors(rx_overflow, can_regs.bus_error_flags, can_regs.tx_err_count, can_regs.rx_err_count)) {
					CAN_loop_int_mask.hw_rx_overflow = true;
					CAN_loop_int_flags.hw_rx_overflow = rx_overflow;
					p_CAN->disable_CAN_interrupts();
					CAN_ISR_Flag_Function(CAN_loop_int_mask, CAN_loop_int_flags);
					p_CAN->enable_CAN_interrupts();
				}
			}
			// Update CAN LED state
			bool CAN_led_on = false;
			if ((current_mode == CAN_MODE_NORMAL) ||
				(current_mode == CAN_MODE_LISTEN)) {
				CAN_led_on = true;
				if (rx_overflow ||
				    can_regs.bus_error_flags.can_bus_err_pasv ||
				    can_regs.bus_error_flags.can_bus_tx_off) {
					CAN_led_on = false;
				}
			}
			HAL_CAN_Status_LED_On(CAN_led_on ? 1 : 0);
		}
	}
}

static uint8_t * CAN_rxfifo_read(uint8_t requested_byte_count, uint16_t* size) {
	int num_transfers_in_buffer = 0;
	int remainder_bytes;
	int num_requested_transfers = 0;
	if(requested_byte_count >= sizeof(TIMESTAMPED_CAN_TRANSFER)) {
		num_requested_transfers = requested_byte_count / sizeof(TIMESTAMPED_CAN_TRANSFER);
		remainder_bytes = requested_byte_count % sizeof(TIMESTAMPED_CAN_TRANSFER);
		for ( int i = 0; i < num_requested_transfers; i++) {
			TIMESTAMPED_CAN_TRANSFER_PADDED *p_rx = p_CAN->get_rx_fifo().dequeue();
			if(p_rx != NULL) {
				memcpy(&read_buffer[i], &p_rx->transfer, sizeof(p_rx->transfer));
				p_CAN->get_rx_fifo().dequeue_return(p_rx);
				read_buffer[i].transfer.id.sidl.invalid = false;
				num_transfers_in_buffer++;
			} else {
				break; /* Fifo now empty */
			}
		}

		/* Once packets are transferred from fifo, update status registers. */
		if (num_transfers_in_buffer > 0) {
			if (can_regs.int_flags.sw_rx_overflow) {
				can_regs.int_flags.sw_rx_overflow = false;
				CAN_int_flags_modified(0,0);
			}
			can_regs.rx_fifo_entry_count = p_CAN->get_rx_fifo().get_count();
		}

		if(p_CAN->get_rx_fifo().is_empty()) {
			/* Clear the rx fifo nonempty status */
			CAN_IFX_INT_FLAGS mask;
			*(uint8_t *)&mask = 0;
			mask.rx_fifo_nonempty = 1;
			CAN_IFX_INT_FLAGS new_value;
			*(uint8_t *)&new_value = 0;
			CAN_ISR_Flag_Function(mask, new_value);
		}
	}
	/* If more transfers were requested than were available, send empty */
	/* (invalid) transfers, to ensure the requested number of bytes is  */
	/* actually returned.                                               */
	if (num_transfers_in_buffer < num_requested_transfers) {
		for ( int i = num_transfers_in_buffer; i < num_requested_transfers; i++) {
			/* Indicate invalid transfers via invalid flag. */
			read_buffer[i].transfer.id.sidl.invalid = true;
			num_transfers_in_buffer++;
		}
	}
	/* If any extra bytes are requested, transmit the count of remaining */
	/* transfers in the first byte following the requested count of transfers. */
	if(remainder_bytes > 0) {
		*(uint8_t *)(&read_buffer[num_transfers_in_buffer]) =
				p_CAN->get_rx_fifo().get_count();
	}
	*size = (num_transfers_in_buffer * sizeof(TIMESTAMPED_CAN_TRANSFER)) + remainder_bytes;
	return (uint8_t *)&read_buffer;
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
				can_regs.tx_full_count++;
				break; /* Fifo now full */
			}
		}
	}
}

static void CAN_int_enable_modified(uint8_t first_offset, uint8_t count) {
	/* No-op */
}

static void CAN_opmode_modified(uint8_t first_offset, uint8_t count) {
	can_pending_updates |= CAN_DEVICE_UPDATE_DEVICE_MODE;
}

static void CAN_command_modified(uint8_t first_offset, uint8_t count) {
	switch(can_regs.command){
	case CAN_CMD_RESET:
		can_pending_updates |= CAN_DEVICE_UPDATE_RESET;
		break;
	case CAN_CMD_SETBITRATE_500KBPS:
		can_pending_updates |= CAN_DEVICE_UPDATE_RESET_500KBPS;
		break;
	case CAN_CMD_SETBITRATE_250KBPS:
		can_pending_updates |= CAN_DEVICE_UPDATE_RESET_250KBPS;
		break;
	case CAN_CMD_FLUSH_RXFIFO:
		can_pending_updates |= CAN_DEVICE_UPDATE_FLUSH_RXFIFO;
		break;
	case CAN_CMD_FLUSH_TXFIFO:
		can_pending_updates |= CAN_DEVICE_UPDATE_FLUSH_TXFIFO;
		break;
	default:
		break;
	}
	can_regs.command = 0;
}

static void CAN_rx_filter_mode_modified(uint8_t first_offset, uint8_t count) {
	if (first_offset < NUM_RX_BUFFERS) {
		if (count > (NUM_RX_BUFFERS - first_offset)) {
			count = NUM_RX_BUFFERS - first_offset;
		}
		uint8_t offset = first_offset;
		while (offset < (first_offset + count)) {
			if (offset == 0) {
				can_pending_updates |= CAN_DEVICE_UPDATE_MODE_0;
			} else if (offset == 1) {
				can_pending_updates |= CAN_DEVICE_UPDATE_MODE_1;
			}
			offset++;
		}
	}
}

static void CAN_accept_mask_rxb0_modified(uint8_t first_offset, uint8_t count) {
	can_pending_updates |= CAN_DEVICE_UPDATE_MASK_0;
}

static void CAN_accept_mask_rxb1_modified(uint8_t first_offset, uint8_t count) {
	can_pending_updates |= CAN_DEVICE_UPDATE_MASK_1;
}

static void CAN_accept_filter_rxb0_modified(uint8_t first_offset, uint8_t count) {
	int curr_offset = 0;
	int last_offset = first_offset + count - 1;
	for ( size_t i = 0; i < SIZEOF_STRUCT(rxb0_filter_indices); i++) {
		curr_offset = i * sizeof(CAN_ID);
		if ((curr_offset >= first_offset) &&
			(curr_offset <= last_offset)){
			if (i == 0) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_0;
			} else if (i == 1) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_1;
			}
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
			if (i == 0) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_2;
			} else if (i == 1) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_3;
			} else if (i == 2) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_4;
			} else if (i == 3) {
				can_pending_updates |= CAN_DEVICE_UPDATE_FILTER_5;
			}
		}
	}
}

static void CAN_advanced_config_modified(uint8_t first_offset, uint8_t count) {
	/* Todo:  Not Yet Implemented. */
}

WritableRegSet CAN_reg_sets[] =
{
	/* Contiguous registers, increasing order of offset  */
	{ offsetof(struct CAN_REGS, int_enable), sizeof(CAN_REGS::int_enable), CAN_int_enable_modified },
	{ offsetof(struct CAN_REGS, int_flags), sizeof(CAN_REGS::int_flags), CAN_int_flags_modified },
	{ offsetof(struct CAN_REGS, opmode), sizeof(CAN_REGS::opmode), CAN_opmode_modified },
	{ offsetof(struct CAN_REGS, command), sizeof(CAN_REGS::command), CAN_command_modified },
	{ offsetof(struct CAN_REGS, rx_filter_mode), sizeof(CAN_REGS::rx_filter_mode), CAN_rx_filter_mode_modified },
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

