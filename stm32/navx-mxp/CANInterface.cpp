/*
 * CANInterface.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#include "CANInterface.h"
#include "CAN_hal.h"
#include <string.h>
#include "ext_interrupts.h"

/******************************************************************************
 * Private Function Definitions
 *******************************************************************************/

/* TODO:  When debugging complete, make these variables static */
CANInterface *CANInterface::p_singleton = (CANInterface *) NULL;
CAN_IFX_INT_FLAGS CAN_isr_flag_mask;
CAN_TRANSFER_PADDED CAN_isr_overflow_read_buffer;

void CANInterface::mcp25625_isr(void) {
	if (p_singleton != NULL) {
		p_singleton->interrupt_handler();
	}
}

CANInterface::CANInterface(uint16_t stm32_gpio_pin) {
	p_singleton = this;
	p_isr_flag_func = NULL;
	/* The following states are updated in the CAN isr */
	*(uint8_t *)&CAN_isr_flag_mask = 0;
	/* The following flags will always be set/cleared by the CAN ISR */
	CAN_isr_flag_mask.bus_error = true;
	CAN_isr_flag_mask.message_error = true;
	CAN_isr_flag_mask.wake = true;
	/* The following flags will only be set (not cleared) by the CAN ISR */
	CAN_isr_flag_mask.sw_rx_overflow = false;
	CAN_isr_flag_mask.hw_rx_overflow = false;
	CAN_isr_flag_mask.rx_fifo_nonempty = false;
	/* The following flag is never set or cleared by the CAN ISR */
	CAN_isr_flag_mask.tx_fifo_empty = false;
	current_mode = CAN_MODE_NORMAL;
	attachInterrupt(stm32_gpio_pin, &mcp25625_isr, FALLING);
}

void CANInterface::register_interrupt_flag_function(CAN_interrupt_flag_func p_isr_flag_func) {
	disable_CAN_interrupts();
	this->p_isr_flag_func = p_isr_flag_func;
	enable_CAN_interrupts();
}

void CANInterface::interrupt_handler() {
	bool rx_fifo_full;
	bool done = false;
	bool rx_fifo_nonempty;
	bool hw_rx_overflow;
	CAN_isr_flag_mask.rx_fifo_nonempty = false;
	CAN_isr_flag_mask.hw_rx_overflow = false;
	CAN_isr_flag_mask.sw_rx_overflow = false;

	//disable_SPI_interrupts();
	if(HAL_MCP25625_HW_Ctl_Get(&ie_ctl_isr)==HAL_OK){
		while (!done) {
			rx_fifo_full = false;
			rx_fifo_nonempty = false;
			hw_rx_overflow = false;
			if(ie_ctl_isr.rx0) {
				/* Read Receive Buffer 0 Contents */
				TIMESTAMPED_CAN_TRANSFER_PADDED *p_rx = rx_fifo.enqueue_reserve();
				if(p_rx) {
					if(HAL_MCP25625_HW_Data_Get(RXB0, (CAN_TRANSFER_PADDED *)p_rx)==HAL_OK) {
						p_rx->transfer.timestamp_ms = HAL_GetTick();
						rx_fifo.enqueue_commit(p_rx);
						rx_fifo_nonempty = true;
						ie_ctl_isr.rx0 = false;
						rx0_complete_count++;
						//memcpy(&(last_rx_packet[0]), p_rx, sizeof(TIMESTAMPED_CAN_TRANSFER_PADDED));
					}
				} else {
					rx_fifo_full = true;
				}
			}
			if(ie_ctl_isr.rx1) {
				/* Read Receive Buffer 1 Contents */
				TIMESTAMPED_CAN_TRANSFER_PADDED *p_rx = rx_fifo.enqueue_reserve();
				if(p_rx) {
					if(HAL_MCP25625_HW_Data_Get(RXB1, (CAN_TRANSFER_PADDED *)p_rx)==HAL_OK) {
						p_rx->transfer.timestamp_ms = HAL_GetTick();
						rx_fifo.enqueue_commit(p_rx);
						rx_fifo_nonempty = true;
						ie_ctl_isr.rx1 = false;
						rx1_complete_count++;
						//memcpy(&(last_rx_packet[1]), p_rx, sizeof(TIMESTAMPED_CAN_TRANSFER_PADDED));
					}
				} else {
					rx_fifo_full = true;
				}
			}
			uint8_t remaining_int_flags = *((uint8_t *)&ie_ctl_isr);
			if (remaining_int_flags != 0){
				if (ie_ctl_isr.merre) {
					message_error_interrupt_count++;
				}
				if (ie_ctl_isr.err) {
					if(HAL_MCP25625_HW_Ctl_Get(&eflg_ctl_isr)==HAL_OK){
						if (eflg_ctl_isr.rx0ovr || eflg_ctl_isr.rx1ovr) {
							/* HW Receive Overflow */
							CAN_isr_flag_mask.hw_rx_overflow = true;
							/* Clear the corresponding error flags */
							eflg_ctl_isr.rx0ovr = false;
							eflg_ctl_isr.rx1ovr = false;
							eflg_ctl_isr.mask = (MCP25625_ERRORFLAG_CTL_MASK)(FLG_RX0OVR | FLG_RX1OVR);
							HAL_MCP25625_HW_Ctl_Update(&eflg_ctl);
							hw_rx_overflow = true;
						}
						if (eflg_ctl_isr.txbo) {
							if (!bus_off) {
								bus_off = true;
								bus_off_count++;
							}
						} else {
							bus_off = false;
						}
					}
					error_interrupt_count++;
				}
				if (ie_ctl_isr.wak) {
					wake_interrupt_count++;
				}
				if (ie_ctl_isr.tx0) {
					tx0_complete_count++;
				}
				if (ie_ctl_isr.tx1) {
					tx1_complete_count++;
				}
				if (ie_ctl_isr.tx2) {
					tx2_complete_count++;
				}
				uint8_t clear_all = 0;
				HAL_MCP25625_BitModify(ie_ctl_isr.reg, remaining_int_flags, &clear_all);
			}
			if(HAL_MCP25625_HW_Ctl_Get(&ie_ctl_isr)==HAL_OK){
				if ((*((uint8_t *)&ie_ctl_isr) == 0) ||
					(rx_fifo_full)) {
					if (rx_fifo_full) {
						rx_buff_full_count++;
					}
					done = true;
				} else {
					more_interrupt_pending_count++;
				}
			} else {
				done = true;
			}
		}
		if(p_isr_flag_func != NULL){
			CAN_isr_flag_mask.rx_fifo_nonempty = rx_fifo_nonempty;
			CAN_isr_flag_mask.hw_rx_overflow = hw_rx_overflow;
			CAN_isr_flag_mask.sw_rx_overflow = rx_fifo_full;
			CAN_IFX_INT_FLAGS int_flags;
			int_flags.rx_fifo_nonempty = rx_fifo_nonempty;
			int_flags.sw_rx_overflow = rx_fifo_full;
			int_flags.hw_rx_overflow = hw_rx_overflow;
			int_flags.bus_error = ie_ctl_isr.err;
			int_flags.message_error = ie_ctl_isr.merre;
			int_flags.wake = ie_ctl_isr.wak;
			p_isr_flag_func(CAN_isr_flag_mask, int_flags);
		}
	}
	//enable_SPI_interrupts();
}

bool CANInterface::clear_rx_overflow() {
	eflg_ctl.rx0ovr = false;
	eflg_ctl.rx1ovr = false;
	eflg_ctl.mask = (MCP25625_ERRORFLAG_CTL_MASK)(FLG_RX0OVR | FLG_RX1OVR);
	uint8_t clear_all = 0;
	uint8_t clear_rx_full_flags_mask = (uint8_t)QSTAT_RX0IF | (uint8_t)QSTAT_RX1IF | 0x20/* ERRIF*/;
	disable_CAN_interrupts();
	/* Clear RX0/RX1 Overflow Error Flags */
	CAN_INTERFACE_STATUS s = HAL_MCP25625_HW_Ctl_Update(&eflg_ctl);
	/* Clear RX Buffer Full & Error Interrupt Flags */
	s = HAL_MCP25625_BitModify(REG_INT_FLG, clear_rx_full_flags_mask, &clear_all);
	enable_CAN_interrupts();
	return (s == MCP25625_OK);
}

bool CANInterface::get_errors(bool& rx_overflow, CAN_ERROR_FLAGS& error_flags,
		uint8_t& tx_err_count, uint8_t& rx_err_count) {
	rx_overflow = false;
	*((uint8_t *)&eflg_ctl) = 0;
	uint8_t err_cnt[2] = {0, 0};
	*((uint8_t *)&error_flags) = 0;
	eflg_ctl.reg = REG_CTL_EFLG;

	disable_CAN_interrupts();
	CAN_INTERFACE_STATUS s1 = HAL_MCP25625_HW_Ctl_Get((void*)&eflg_ctl);
	CAN_INTERFACE_STATUS s2 = HAL_MCP25625_Read(REG_STAT_TEC, err_cnt, sizeof(err_cnt));
	enable_CAN_interrupts();
	if(s1 == MCP25625_OK){
		rx_overflow = ((eflg_ctl.rx0ovr | eflg_ctl.rx1ovr) != 0);
		error_flags.can_bus_warn = eflg_ctl.ewarn;
		error_flags.can_bus_err_pasv = (eflg_ctl.rxerr | eflg_ctl.txerr);
		error_flags.can_bus_tx_off = (eflg_ctl.txbo);
	}
	if(s2 == MCP25625_OK){
		tx_err_count = err_cnt[0];
		rx_err_count = err_cnt[1];
	} else {
		tx_err_count = 0;
		rx_err_count = 0;
	}
	return ((*((uint8_t *)&error_flags) !=0) || rx_overflow);
}

CAN_INTERFACE_STATUS CANInterface::flush_rx_fifo(){
	disable_CAN_interrupts();
	this->rx_fifo.flush();
	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::flush_tx_fifo(){
	disable_CAN_interrupts();
	this->tx_fifo.flush();
	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::get_quick_status(MCP25625_CAN_QUICK_STATUS& status) {
	disable_CAN_interrupts();
	HAL_StatusTypeDef spi_status = HAL_MCP25625_Read_Status(&status);
	enable_CAN_interrupts();
	return spi_status;
}

void CANInterface::process_transmit_fifo() {
	MCP25625_CAN_QUICK_STATUS status;
	MCP25625_INT_CTL ctl;
	disable_CAN_interrupts();
	HAL_StatusTypeDef spi_status = HAL_MCP25625_Read_Status(&status);
	get_interrupt_enables(ctl);
	enable_CAN_interrupts();
	if ( (current_mode == CAN_MODE_NORMAL) ||
		 (current_mode == CAN_MODE_LOOP) ||
		 (current_mode == CAN_MODE_LISTEN)) {
		if ((!ctl.rx0) || (!ctl.rx1)) {
			CAN_ID mask_0;
			CAN_ID filter0_0;
			MCP25625_CAN_STATUS_REG statreg;
			statreg.reg = REG_STAT_CAN;
			disable_CAN_interrupts();
			HAL_MCP25625_HW_Filter_Get(RXF_0, &filter0_0);
			HAL_MCP25625_HW_Mask_Get(RXB0, &mask_0);
			HAL_MCP25625_HW_Ctl_Get((void*) &statreg);
			// can_stat.opmod is current hardware op mode.
			// Configure CAN bus timing
			uint8_t BRP, SJW, PRSEG, PHSEG1, PHSEG2;
			bool SAM, BTLMODE, WAKFIL, SOFR;
			get_btl_config(BRP, SJW, PRSEG, PHSEG1, PHSEG2, SAM, BTLMODE, WAKFIL, SOFR);
			//btl_config(0,2,7,2,2,1,1,0,0); /* 24Mhz Crystal 2TQ; 11-meter max bus len */

			enable_CAN_interrupts();
			clear_rx_overflow();
			enable_controller_interrupts();
		}
	}
	if(spi_status==HAL_OK){
		int count = tx_fifo.get_count();
		if (uint32_t(count) > tx_fifo_highwatermark_count) {
			tx_fifo_highwatermark_count = uint32_t(count);
		}
		while(!tx_fifo.is_empty()) {
			MCP25625_TX_BUFFER_INDEX available_tx_index;
			if(!(status & QSTAT_TX0REQ)) {
				available_tx_index = TXB0;
			} else if(!(status & QSTAT_TX1REQ)) {
				available_tx_index = TXB1;
			} else if(!(status & QSTAT_TX2REQ)) {
				available_tx_index = TXB2;
			} else {
				break;
			}
			CAN_TRANSFER_PADDED *p_data = tx_fifo.dequeue();
			if(p_data) {
				disable_CAN_interrupts();
				CAN_INTERFACE_STATUS s = msg_load(available_tx_index, p_data);
				if(s == MCP25625_OK){
					if(this->msg_send(available_tx_index)==MCP25625_OK){
						if(available_tx_index == TXB0) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX0REQ);
							tx0_sent_count++;
						} else if (available_tx_index == TXB1) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX1REQ);
							tx1_sent_count++;
						} else if (available_tx_index == TXB2) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX2REQ);
							tx2_sent_count++;
						}
						tx_fifo.dequeue_return(p_data);
					}
				}
				enable_CAN_interrupts();
			}
		}
	}
}

CAN_INTERFACE_STATUS CANInterface::init(CAN_MODE mode) {

	disable_CAN_interrupts();
	message_error_interrupt_count =
		error_interrupt_count =
		wake_interrupt_count =
		tx0_complete_count =
		tx1_complete_count =
		tx2_complete_count =
		rx0_complete_count =
		rx1_complete_count =
		rx_buff_full_count =
		more_interrupt_pending_count =
		tx0_sent_count =
		tx1_sent_count =
		tx2_sent_count =
		tx0_err_count =
		tx1_err_count =
		tx2_err_count =
		tx_fifo_highwatermark_count =
		bus_off_count = 0;

	bus_off = false;

	tx_pending[0] = tx_pending[1] = tx_pending[2] = false;

	can_ctl.reg = REG_CTL_CAN;
	rts_pins.reg = REG_CTL_RTS;
	rx_pins.reg = REG_CTL_RXP;
	tx_ctl.reg = REG_CTL_TXB;
	rx0_ctl.reg = REG_CTL_RXB0;
	rx1_ctl.reg = REG_CTL_RXB1;
	cnf_1.reg = REG_CTL_CNF1;
	cnf_2.reg = REG_CTL_CNF2;
	cnf_3.reg = REG_CTL_CNF3;
	eflg_ctl.reg = REG_CTL_EFLG;
	eflg_ctl_isr.reg = REG_CTL_EFLG;
	status_reg.reg = REG_STAT_CAN;
	ie_ctl_isr.reg = REG_INT_FLG;
	default_mode = mode;

	HAL_MCP25625_PinReset(); /* Upon reset, the opmode should be CONFIG */
	HAL_MCP25625_PinWake();  /* Wake the Transceiver */

	HAL_MCP25625_HWReset();	/* Just to be extra sure, send the SPI Reset command, too */

	/* Verify the CANSTAT register power on reset (POR) values */
	bool is_config_mode = false;
	if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==MCP25625_OK) {
		can_stat = status_reg.value;
		is_config_mode = (can_stat.opmod == CAN_MODE_CONFIG);
	}

	/* Verify the CANCTL register power on reset (POR) values */
	if (HAL_MCP25625_HW_Ctl_Get((void*) &can_ctl))
		goto error_cleanup;

	// Setup Control
	can_ctl.clkpre = 1; /* Divide Crystal oscillator /1 */
	can_ctl.clken = true;
	can_ctl.abat = true; /* Clear any transmission in progress */
	can_ctl.osm = false;
	can_ctl.reqop = CAN_MODE_CONFIG;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &can_ctl))
		goto error_cleanup;

	HAL_Delay(2);

	/* Clear the abort transmission flag. */

	can_ctl.abat = false;
	if (HAL_MCP25625_HW_Ctl_Set((void*) &can_ctl))
		goto error_cleanup;


	/* Re-verify that configuration mode is successfully entered. */
	if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==MCP25625_OK) {
		can_stat = status_reg.value;
		 is_config_mode = (can_stat.opmod == CAN_MODE_CONFIG);
	}

	// Deactivate TX RTS pins ( TX0, TX1 )
	rts_pins.p0_mode = false;
	rts_pins.p1_mode = false;
	rts_pins.p2_mode = false;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &rts_pins))
		goto error_cleanup;

	// Deactivate RX pins ( RX0, RX1 )
	rx_pins.p0_mode = false;
	rx_pins.p0_enab = false;
	rx_pins.p1_mode = false;
	rx_pins.p1_enab = false;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins))
		goto error_cleanup;

	// Configure TXB registers( same cofig for all txb regs )
	tx_ctl.txp = 2; /* Highest priority */
	tx_ctl.txreq = false;
	tx_ctl.buffer = TXB0;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		goto error_cleanup;

	tx_ctl.txp = 1; /* High Priority */
	tx_ctl.buffer = TXB1;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		goto error_cleanup;

	tx_ctl.txp = 0; /* Medium Priority */
	tx_ctl.buffer = TXB2;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		goto error_cleanup;

	// Configure RXB registers
	rx0_ctl.bukt = true;				/* Enable receive overflow into RXB1. */
	rx0_ctl.rxm = MCP25625_RX_MODE_ALL; /* Enable all (default) masks and filters */
	rx1_ctl.rxm = MCP25625_RX_MODE_ALL; /* Enable all (default) masks and filters */
	if (HAL_MCP25625_HW_Ctl_Set((void*) &rx0_ctl)
			|| HAL_MCP25625_HW_Ctl_Set((void*) &rx1_ctl))
		goto error_cleanup;

	HAL_MCP25625_HW_Ctl_Get(&rx0_ctl);
	HAL_MCP25625_HW_Ctl_Get(&rx1_ctl);

	clear_rx_overflow();
	clear_all_interrupt_flags();

	// Configure CAN bus timing
	uint8_t BRP, SJW, PRSEG, PHSEG1, PHSEG2;
	bool SAM, BTLMODE, WAKFIL, SOFR;
	get_btl_config(BRP, SJW, PRSEG, PHSEG1, PHSEG2, SAM, BTLMODE, WAKFIL, SOFR);

	btl_config(0,2,7,2,2,1,1,0,0); /* 24Mhz Crystal 2TQ; 11-meter max bus len */
	//btl_config(0,1,8,1,2,1,1,0,0); /* 24Mhz Crystal 1TQ; 19-meter max bus len */

	get_btl_config(BRP, SJW, PRSEG, PHSEG1, PHSEG2, SAM, BTLMODE, WAKFIL, SOFR);

	/* Configure all masks/filters to be extremely restrictive. */

	CAN_ID default_mask;
	CAN_pack_extended_id(0x1FFFFFFF, &default_mask);
	mask_config(RXB0, &default_mask);
	mask_config(RXB1, &default_mask);

	CAN_ID default_filter;
	CAN_pack_extended_id(0x1FFFFFFF, &default_filter);
	filter_config(RXF_0, &default_filter);
	filter_config(RXF_1, &default_filter);
	filter_config(RXF_2, &default_filter);
	filter_config(RXF_3, &default_filter);
	filter_config(RXF_4, &default_filter);
	filter_config(RXF_5, &default_filter);

	if (set_mode(default_mode))
		goto error_cleanup;

	enable_CAN_interrupts();
	return MCP25625_OK;

error_cleanup:
	enable_CAN_interrupts();
	return MCP25625_CTL_ERR;
}

void CANInterface::enable_controller_interrupts() {
	disable_CAN_interrupts();
	// Enable interrupts from MCP25625 controller
	ie_ctl.err = true;
	ie_ctl.mask = (MCP25625_INT_CTL_MASK) 0xFF;
	ie_ctl.merre = true;
	ie_ctl.rx0 = true;
	ie_ctl.rx1 = true;
	ie_ctl.tx0 = true;
	ie_ctl.tx1 = true;
	ie_ctl.tx2 = true;
	ie_ctl.wak = false;
	ie_ctl.reg = REG_INT_CTL;
	HAL_MCP25625_HW_Ctl_Set((void*) &ie_ctl);
	enable_CAN_interrupts();
}

void CANInterface::disable_controller_interrupts() {
	disable_CAN_interrupts();
	// Enable interrupts from MCP25625 controller
	ie_ctl.err = false;
	ie_ctl.mask = (MCP25625_INT_CTL_MASK) 0xFF;
	ie_ctl.merre = false;
	ie_ctl.rx0 = false;
	ie_ctl.rx1 = false;
	ie_ctl.tx0 = false;
	ie_ctl.tx1 = false;
	ie_ctl.tx2 = false;
	ie_ctl.wak = false;
	ie_ctl.reg = REG_INT_CTL;
	HAL_MCP25625_HW_Ctl_Set((void*) &ie_ctl);
	enable_CAN_interrupts();
}

CAN_INTERFACE_STATUS CANInterface::get_interrupt_enables(MCP25625_INT_CTL& ctl) {
	ctl.reg = REG_INT_CTL;
	CAN_INTERFACE_STATUS s = HAL_MCP25625_HW_Ctl_Get((void*) &ctl);
	return s;
}

CAN_INTERFACE_STATUS CANInterface::clear_error_interrupt_flags()
{
	disable_CAN_interrupts();
	// Clear only error interrupt flags - and do not clear any others.
	uint8_t clear_all = 0;
	ie_ctl.err = true;
	ie_ctl.merre = true;
	ie_ctl.rx0 = false;
	ie_ctl.rx1 = false;
	ie_ctl.tx0 = false;
	ie_ctl.tx1 = false;
	ie_ctl.tx2 = false;
	ie_ctl.wak = false;
	uint8_t mask = *((uint8_t *)&ie_ctl);
	ie_ctl.reg = REG_INT_FLG;

	HAL_MCP25625_BitModify(ie_ctl.reg, mask, &clear_all);
	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::clear_all_interrupt_flags()
{
	disable_CAN_interrupts();
	// Clear all interrupt flags
	ie_ctl.err = false;
	ie_ctl.merre = false;
	ie_ctl.rx0 = false;
	ie_ctl.rx1 = false;
	ie_ctl.tx0 = false;
	ie_ctl.tx1 = false;
	ie_ctl.tx2 = false;
	ie_ctl.wak = false;
	ie_ctl.reg = REG_INT_FLG;
	if (HAL_MCP25625_HW_Ctl_Set((void*) &ie_ctl)) {
		enable_CAN_interrupts();
		return MCP25625_CTL_ERR;
	}

	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::set_mode(CAN_MODE mode, bool disable_interrupts) {
	if (current_mode == mode) return MCP25625_OK;

	CAN_INTERFACE_STATUS result = MCP25625_OK;
	if(disable_interrupts) {
		disable_CAN_interrupts();
	}

	can_ctl.reqop = mode;
	can_ctl.mask = CAN_REQOP;
	if (mode == CAN_MODE_CONFIG) {
		can_ctl.abat = true; /* Clear any transmission in progress */
		can_ctl.mask = (MCP25625_CAN_CTL_MASK)(CAN_REQOP | CAN_ABAT);
	}

	HAL_MCP25625_HW_Ctl_Update((void*) &can_ctl);

	/* Read the CANSTAT register, verify OPMODE has been changed */
	/* (This can be delayed while transmit is in progress. */
	result = MCP25625_OK;
	for (size_t i = 0; i < 300; i++) {
		if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==HAL_OK) {
			can_stat = status_reg.value;
			if (can_stat.opmod != mode) {
				result = MCP25625_CTL_ERR;
			} else {
				current_mode = mode;
				result = MCP25625_OK;
				break;
			}
		}
	}

	/* If device failed to respond within the timeout period,
	 * revert the internal mode state variable to match
	 * the opmode state the hardware is currently in.
	 */
	if (result == MCP25625_CTL_ERR) {
		current_mode = can_stat.opmod;
	}

	if (current_mode != CAN_MODE_CONFIG) {
		enable_controller_interrupts();
	} else {
		/* Only ever clear interrupts if the transition to
		 * CONFIG mode has been verified.
		 */
		if (result == MCP25625_OK) {
			disable_controller_interrupts();
		}
	}

	if(disable_interrupts) {
		enable_CAN_interrupts();
	}
	return result;
}

CAN_INTERFACE_STATUS CANInterface::com_error(bool clear) {
	ie_ctl.reg = REG_INT_FLG;

	HAL_MCP25625_HW_Ctl_Get(&ie_ctl);

	if (ie_ctl.merre) {
		ie_ctl.merre = 0;
		ie_ctl.mask = CAN_MERRE;

		if (clear)
			HAL_MCP25625_HW_Ctl_Update(&ie_ctl);

		return MCP25625_COM_ERR;
	}

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::get_btl_config(uint8_t& BRP, uint8_t& SJW,
		uint8_t& PRSEG, uint8_t& PHSEG1, uint8_t& PHSEG2, bool& SAM, bool& BTLMODE,
		bool& WAKFIL, bool& SOFR) {

	if (HAL_MCP25625_HW_Ctl_Get((void*) &cnf_1)
			|| HAL_MCP25625_HW_Ctl_Get((void*) &cnf_2)
			|| HAL_MCP25625_HW_Ctl_Get((void*) &cnf_3))
		return MCP25625_CTL_ERR;

	BRP = cnf_1.brp;
	SJW = cnf_1.sjw + 1;
	BTLMODE = cnf_2.btlmode;
	SAM = cnf_2.sam;
	PRSEG = cnf_2.prseg + 1;
	PHSEG1 = cnf_2.phseg1 + 1;
	PHSEG2 = cnf_3.phseg2 + 1;
	SOFR = cnf_3.sof;
	WAKFIL = cnf_3.wakfil;

	return MCP25625_OK;
}


CAN_INTERFACE_STATUS CANInterface::btl_config(uint8_t BRP, uint8_t SJW,
		uint8_t PRSEG, uint8_t PHSEG1, uint8_t PHSEG2, bool SAM, bool BTLMODE,
		bool WAKFIL, bool SOFR) {
	if (set_mode(CAN_MODE_CONFIG))
		return MCP25625_MODE_FAULT;

	cnf_1.brp = BRP;
	cnf_1.sjw = SJW - 1;
	cnf_2.btlmode = BTLMODE;
	cnf_2.sam = SAM;
	cnf_2.prseg = PRSEG - 1;
	cnf_2.phseg1 = PHSEG1 - 1;
	cnf_3.phseg2 = PHSEG2 - 1;
	cnf_3.sof = SOFR;
	cnf_3.wakfil = WAKFIL;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &cnf_1)
			|| HAL_MCP25625_HW_Ctl_Set((void*) &cnf_2)
			|| HAL_MCP25625_HW_Ctl_Set((void*) &cnf_3))
		return MCP25625_CTL_ERR;

	if (set_mode(default_mode))
		return MCP25625_MODE_FAULT;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::rx_config(MCP25625_RX_BUFFER_INDEX rx_buff,
		MCP25625_RX_MODE mode, bool BUKT) {
	bool RX_PIN = false;
	disable_CAN_interrupts();
	switch (rx_buff) {
	case RXB0:

		rx0_ctl.bukt = BUKT;
		rx0_ctl.rxm = mode;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx0_ctl)) {
			enable_CAN_interrupts();
			return MCP25625_CTL_ERR;
		}

		rx_pins.p0_enab = RX_PIN;
		rx_pins.p0_mode = true;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins)) {
			enable_CAN_interrupts();
			return MCP25625_CTL_ERR;
		}

		break;
	case RXB1:

		rx1_ctl.rxm = mode;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx1_ctl)) {
			enable_CAN_interrupts();
			return MCP25625_CTL_ERR;
		}

		rx_pins.p1_enab = RX_PIN;
		rx_pins.p1_mode = true;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins)) {
			enable_CAN_interrupts();
			return MCP25625_CTL_ERR;
		}

		break;
	}

	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::filter_config(
		MCP25625_RX_FILTER_INDEX rx_filter, CAN_ID *p_id) {
	CAN_MODE previous_mode = current_mode;
	disable_CAN_interrupts();
	if (set_mode(CAN_MODE_CONFIG, false)) {
		enable_CAN_interrupts();
		return MCP25625_MODE_FAULT;
	}

	if (HAL_MCP25625_HW_Filter_Set(rx_filter, p_id)) {
		enable_CAN_interrupts();
		return MCP25625_CTL_ERR;
	}

	if (set_mode(previous_mode, false)) {
		enable_CAN_interrupts();
		return MCP25625_MODE_FAULT;
	}

	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::mask_config(MCP25625_RX_BUFFER_INDEX rx_mask,
		CAN_ID *p_id) {
	CAN_MODE previous_mode = current_mode;
	disable_CAN_interrupts();
	if (set_mode(CAN_MODE_CONFIG, false)) {
		enable_CAN_interrupts();
		return MCP25625_MODE_FAULT;
	}

	if (HAL_MCP25625_HW_Mask_Set(rx_mask, p_id)) {
		enable_CAN_interrupts();
		return MCP25625_CTL_ERR;
	}

	if (set_mode(previous_mode, false)) {
		enable_CAN_interrupts();
		return MCP25625_MODE_FAULT;
	}

	enable_CAN_interrupts();
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::msg_load(MCP25625_TX_BUFFER_INDEX tx_buff,
		CAN_TRANSFER_PADDED *p_tx) {
#if 0
	tx_ctl.buffer = tx_buff;
	tx_ctl.txreq = false;
	tx_ctl.mask = TXB_TXREQ;

	if (HAL_MCP25625_HW_Ctl_Update(&tx_ctl))
		return MCP25625_CTL_ERR;
#endif
	if (HAL_MCP25625_HW_Data_Set(tx_buff, p_tx))
		return MCP25625_TX_ERR;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::msg_send(MCP25625_TX_BUFFER_INDEX tx_buff) {
	tx_ctl.buffer = tx_buff;
	if (tx_ctl.buffer == TXB0) {
		tx_ctl.txp = 2;
	} else if (tx_ctl.buffer == TXB1) {
		tx_ctl.txp = 1;
	} else {
		tx_ctl.txp = 0;
	}
	tx_ctl.txreq = true;
	tx_ctl.mask = TXB_TXREQ;

	if (HAL_MCP25625_HW_Ctl_Update((void*) &tx_ctl))
		return MCP25625_CTL_ERR;

	return MCP25625_OK;
}

CANInterface::~CANInterface() {
}

