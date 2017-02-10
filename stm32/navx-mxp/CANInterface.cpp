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

CANInterface *CANInterface::p_singleton = (CANInterface *) NULL;

void CANInterface::mcp25625_isr(void) {
	if (p_singleton != NULL) {
		p_singleton->interrupt_handler();
	}
}

CANInterface::CANInterface(uint16_t stm32_gpio_pin) {
	p_singleton = this;
	message_error_interrupt_count =
		error_interrupt_count =
		wake_interrupt_count =
		tx0_complete_count =
		tx1_complete_count =
		tx2_complete_count =
		rx0_complete_count =
		rx1_complete_count = 0;
	attachInterrupt(stm32_gpio_pin, &mcp25625_isr, FALLING);
}

void CANInterface::interrupt_handler() {
	if(HAL_MCP25625_HW_Ctl_Get(&ie_ctl_isr)==HAL_OK){
		if(ie_ctl_isr.rx0) {
			/* Read Receive Buffer 0 Contents */
			CAN_TRANSFER *p_rx = rx_fifo.enqueue_reserve();
			if(p_rx) {
				if(HAL_MCP25625_HW_Data_Get(RXB0, p_rx)==HAL_OK) {
					rx_fifo.enqueue_commit(p_rx);
					ie_ctl_isr.rx0 = false;
					rx0_complete_count++;
				}
			}
		}
		if(ie_ctl_isr.rx1) {
			/* Read Receive Buffer 1 Contents */
			CAN_TRANSFER *p_rx = rx_fifo.enqueue_reserve();
			if(p_rx) {
				if(HAL_MCP25625_HW_Data_Get(RXB1, p_rx)==HAL_OK) {
					rx_fifo.enqueue_commit(p_rx);
					ie_ctl_isr.rx1 = false;
					rx1_complete_count++;
				}
			}
		}
		uint8_t remaining_int_flags = *((uint8_t *)&ie_ctl_isr);
		if (remaining_int_flags != 0){
			if (ie_ctl_isr.merre) {
				message_error_interrupt_count++;
			}
			if (ie_ctl_isr.err) {
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
	}
}

bool CANInterface::clear_rx_overflow() {
	eflg_ctl.rx0ovr = false;
	eflg_ctl.rx1ovr = false;
	eflg_ctl.mask = (MCP25625_ERRORFLAG_CTL_MASK)(FLG_RX0OVR | FLG_RX1OVR);
	HAL_NVIC_DisableIRQ((IRQn_Type)EXTI9_5_IRQn);
	CAN_INTERFACE_STATUS s = HAL_MCP25625_HW_Ctl_Update(&eflg_ctl);
	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
	return (s == MCP25625_OK);
}

bool CANInterface::get_errors(CAN_ERROR_FLAGS& error_flags,
		uint8_t& tx_err_count, uint8_t& rx_err_count) {
	uint8_t err_cnt[2] = {0, 0};
	*((uint8_t *)&error_flags) = 0;

	bool remove_me;
	HAL_NVIC_DisableIRQ((IRQn_Type)EXTI9_5_IRQn);
	CAN_INTERFACE_STATUS s1 = HAL_MCP25625_HW_Ctl_Get((void*) &eflg_ctl);
	CAN_INTERFACE_STATUS s2 = HAL_MCP25625_Read(REG_STAT_TEC, err_cnt,sizeof(err_cnt));
	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
	if(s1 == MCP25625_OK){
		error_flags.rx_overflow = (eflg_ctl.rx0ovr | eflg_ctl.rx1ovr);
		if(eflg_ctl.rx1ovr){
			remove_me = true;
		}
		error_flags.can_bus_warn = eflg_ctl.ewarn;
		error_flags.can_bus_err_pasv = (eflg_ctl.rxerr | eflg_ctl.txerr);
		error_flags.can_bux_tx_off = (eflg_ctl.txbo);
	}
	if(s2 == MCP25625_OK){
		tx_err_count = err_cnt[0];
		rx_err_count = err_cnt[1];
	}
	return (*((uint8_t *)&error_flags) !=0);
}

void CANInterface::process_transmit_fifo() {
	MCP25625_CAN_QUICK_STATUS status;
	HAL_NVIC_DisableIRQ((IRQn_Type)EXTI9_5_IRQn);
	HAL_StatusTypeDef spi_status = HAL_MCP25625_Read_Status(&status);
//	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
	if(spi_status==HAL_OK){
		while(!tx_fifo.is_empty()) {
			MCP25625_TX_BUFFER_INDEX available_tx_index;
			if(!(status & QSTAT_TX0REQ)) {
				available_tx_index = TXB0;
			} else if(!(status & QSTAT_TX1REQ)) {
				available_tx_index = TXB1;
#if 0
			} else if(!(status & QSTAT_TX2REQ)) {
				available_tx_index = TXB2;
#endif
			} else {
				break;
			}
			CAN_DATA *p_data = tx_fifo.dequeue();
			if(p_data) {
				uint32_t ID;
				bool IDE;
				bool RTR;
				IDE = own_id.sidl.ide;
				RTR = own_id.sidl.srr;
				if (IDE) {
					CAN_unpack_extended_id(&own_id, &ID);
				} else {
					CAN_unpack_standard_id(&own_id, &ID);
				}
				//HAL_NVIC_DisableIRQ((IRQn_Type)EXTI9_5_IRQn);
				CAN_INTERFACE_STATUS s = msg_load(available_tx_index, p_data->buff, p_data->dlc.len, ID, IDE, RTR);
				if(s == MCP25625_OK){
					if(this->msg_send(available_tx_index)==MCP25625_OK){
						if(available_tx_index == TXB0) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX0REQ);
						} else if ( available_tx_index == TXB1) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX1REQ);
						} else if ( available_tx_index == TXB2) {
							status = (MCP25625_CAN_QUICK_STATUS)(status | QSTAT_TX2REQ);
						}
					}
				}
				//HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
			}
		}
	}
	/* TODO:  Remove this artificial receiver loading code. */
	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
}

CAN_INTERFACE_STATUS CANInterface::init(CAN_MODE mode) {

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

	/* Verify the CANSTAT register power on reset (POR) values */
	bool is_config_mode = false;
	if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==MCP25625_OK) {
		can_stat = status_reg.value;
		is_config_mode = (can_stat.opmod == CAN_MODE_CONFIG);
	}

	/* Verify the CANCTL register power on reset (POR) values */
	if (HAL_MCP25625_HW_Ctl_Get((void*) &can_ctl))
		return MCP25625_CTL_ERR;

	// Setup Control
	can_ctl.clkpre = 0;
	can_ctl.clken = false;
	can_ctl.abat = true; /* Clear any transmission in progress */
	can_ctl.osm = false;
	can_ctl.reqop = CAN_MODE_CONFIG;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &can_ctl))
		return MCP25625_CTL_ERR;

	HAL_Delay(2);

	/* Clear the abort transmission flag. */

	can_ctl.abat = false;
	if (HAL_MCP25625_HW_Ctl_Set((void*) &can_ctl))
		return MCP25625_CTL_ERR;


	/* Re-verify that configuration mode is successfully entered. */
	if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==MCP25625_OK) {
		can_stat = status_reg.value;
		 is_config_mode = (can_stat.opmod == CAN_MODE_CONFIG);
	}

	// Deactivate RTS pins ( TX0, TX1 )
	rts_pins.p0_mode = false;
	rts_pins.p1_mode = false;
	rts_pins.p2_mode = false;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &rts_pins))
		return MCP25625_RTS_ERR;

	// Deactivate RX pins ( RX0, RX1 )
	rx_pins.p0_mode = false;
	rx_pins.p0_enab = false;
	rx_pins.p1_mode = false;
	rx_pins.p1_enab = false;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins))
		return MCP25625_RXP_ERR;

	// Configure TXB registers( same cofig for all txb regs )
	tx_ctl.txp = 2;
	tx_ctl.txreq = false;
	tx_ctl.buffer = TXB0;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		return MCP25625_CTL_ERR;

	tx_ctl.buffer = TXB1;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		return MCP25625_CTL_ERR;

	tx_ctl.buffer = TXB2;
	if (HAL_MCP25625_HW_Ctl_Set(&tx_ctl))
		return MCP25625_CTL_ERR;

	// Configure RXB registers
	rx0_ctl.bukt = true;				/* Enable receive overflow into RXB1. */
	rx0_ctl.rxm = MCP25625_RX_MODE_OFF; /* Disable all masks and filters (receive all messages) */
	rx1_ctl.rxm = MCP25625_RX_MODE_OFF; /* Disable all masks and filters (receive all messages) */
	if (HAL_MCP25625_HW_Ctl_Set((void*) &rx0_ctl)
			|| HAL_MCP25625_HW_Ctl_Set((void*) &rx1_ctl))
		return MCP25625_CTL_ERR;

	HAL_MCP25625_HW_Ctl_Get(&rx0_ctl);
	HAL_MCP25625_HW_Ctl_Get(&rx1_ctl);

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
	if (HAL_MCP25625_HW_Ctl_Set((void*) &ie_ctl))
		return MCP25625_CTL_ERR;

	/* Verify the current interrupt state is all clear. */
	MCP25625_INT_CTL ie_ctl_copy;
	if (!HAL_MCP25625_HW_Ctl_Get((void*) &ie_ctl))
	{
		 ie_ctl_copy = ie_ctl;
	}

	// Enable interrupts
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
	if (HAL_MCP25625_HW_Ctl_Set((void*) &ie_ctl))
		return MCP25625_CTL_ERR;

	/* Verify the current interrupt state is all clear. */
	if (!HAL_MCP25625_HW_Ctl_Get((void*) &ie_ctl))
	{
		 ie_ctl_copy = ie_ctl;
	}

	if (set_mode(default_mode))
		return MCP25625_MODE_FAULT;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::set_mode(CAN_MODE mode) {
	can_ctl.reqop = mode;
	can_ctl.mask = CAN_REQOP;

	HAL_MCP25625_HW_Ctl_Update((void*) &can_ctl);

	/* Read the CANSTAT register, verify OPMODE has been changed */
	/* (This can be delayed while transmit is in progress. */
	if (HAL_MCP25625_HW_Ctl_Get((void*) &status_reg)==HAL_OK) {
		can_stat = status_reg.value;
		if (can_stat.opmod != mode)
			return MCP25625_CTL_ERR;
	}

	return MCP25625_OK;
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

CAN_INTERFACE_STATUS CANInterface::btl_config(uint8_t BRP, uint8_t SJW,
		uint8_t PRSEG, uint8_t PHSEG1, uint8_t PHSEG2, bool SAM, bool BTLMODE,
		bool WAKFIL, bool SOFR) {
	if (set_mode(CAN_MODE_CONFIG))
		return MCP25625_MODE_FAULT;

	cnf_1.brp = BRP;
	cnf_1.sjw = SJW;
	cnf_2.btlmode = BTLMODE;
	cnf_2.sam = SAM;
	cnf_2.prseg = PRSEG;
	cnf_2.phseg1 = PHSEG1;
	cnf_3.phseg2 = PHSEG2;
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

CAN_INTERFACE_STATUS CANInterface::tx_config(MCP25625_TX_BUFFER_INDEX tx_buff,
		uint8_t TXP) {
	bool use_RTS_pin = false;
	tx_ctl.buffer = tx_buff;
	tx_ctl.txp = TXP & TXB_TXP;

	if (set_mode(CAN_MODE_CONFIG))
		return MCP25625_MODE_FAULT;

	if (HAL_MCP25625_HW_Ctl_Set((void*) &tx_ctl))
		return MCP25625_CTL_ERR;

	switch (tx_buff) {
	case TXB0:

		rts_pins.p0_mode = use_RTS_pin;
		rts_pins.mask = RTS_P0_MODE;

		if (HAL_MCP25625_HW_Ctl_Update((void*) &rts_pins))
			return MCP25625_CTL_ERR;
		break;
	case TXB1:

		rts_pins.p1_mode = use_RTS_pin;
		rts_pins.mask = RTS_P1_MODE;

		if (HAL_MCP25625_HW_Ctl_Update((void*) &rts_pins))
			return MCP25625_CTL_ERR;
		break;
	case TXB2:

		rts_pins.p2_mode = use_RTS_pin;
		rts_pins.mask = RTS_P2_MODE;

		if (HAL_MCP25625_HW_Ctl_Update((void*) &rts_pins))
			return MCP25625_CTL_ERR;
		break;
	case TXB_INVALID:
		break;
	}

	if (set_mode(default_mode))
		return MCP25625_MODE_FAULT;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::rx_config(MCP25625_RX_BUFFER_INDEX rx_buff,
		MCP25625_RX_MODE mode, bool BUKT) {
	bool RX_PIN = false;
	switch (rx_buff) {
	case RXB0:

		rx0_ctl.bukt = BUKT;
		rx0_ctl.rxm = mode;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx0_ctl))
			return MCP25625_CTL_ERR;

		rx_pins.p0_enab = RX_PIN;
		rx_pins.p0_mode = true;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins))
			return MCP25625_CTL_ERR;

		break;
	case RXB1:

		rx1_ctl.rxm = mode;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx1_ctl))
			return MCP25625_CTL_ERR;

		rx_pins.p1_enab = RX_PIN;
		rx_pins.p1_mode = true;

		if (HAL_MCP25625_HW_Ctl_Set((void*) &rx_pins))
			return MCP25625_CTL_ERR;

		break;
	}

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::filter_config(
		MCP25625_RX_FILTER_INDEX rx_filter, uint16_t SID, uint32_t EID,
		bool EXIDE) {
	if (set_mode(CAN_MODE_CONFIG))
		return MCP25625_MODE_FAULT;

	transfer.id.sidl.ide = EXIDE;
	if (EXIDE)
		CAN_pack_extended_id(EID, &transfer.id);
	else
		CAN_pack_standard_id(SID, &transfer.id);

	if (HAL_MCP25625_HW_Filter_Set(rx_filter, &transfer.id))
		return MCP25625_CTL_ERR;

	if (set_mode(default_mode))
		return MCP25625_MODE_FAULT;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::mask_config(MCP25625_RX_BUFFER_INDEX rx_mask,
		uint16_t SID, uint32_t EID) {
	CAN_pack_extended_id(EID, &transfer.id);
	CAN_pack_standard_id(SID, &transfer.id);

	if (HAL_MCP25625_HW_Mask_Set(rx_mask, &transfer.id))
		return MCP25625_CTL_ERR;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::msg_load(MCP25625_TX_BUFFER_INDEX tx_buff,
		uint8_t *msg, uint8_t count, uint32_t ID, bool IDE, bool RTR) {
	tx_ctl.buffer = tx_buff;
	tx_ctl.txreq = false;
	tx_ctl.mask = TXB_TXREQ;

#if 0 /* This is not necessary, per datasheet */
	if (set_mode(OPMODE_CONFIG))
		return MCP25625_MODE_FAULT;
#endif

	if (HAL_MCP25625_HW_Ctl_Update(&tx_ctl))
		return MCP25625_CTL_ERR;

#if 0 /* This is not necessary, per datasheet */
	if (set_mode(default_mode))
		return MCP25625_MODE_FAULT;
#endif

	transfer.id.sidl.ide = IDE;
	if (IDE)
		CAN_pack_extended_id(ID, &transfer.id);
	else
		CAN_pack_standard_id(ID, &transfer.id);
	transfer.id.sidl.unused = 0;
	transfer.id.sidl.srr = 0;

	*(uint8_t *)&(transfer.payload.dlc) = 0;
	transfer.payload.dlc.rtr = RTR;
	transfer.payload.dlc.len = count;
	memcpy((void*) &transfer.payload.buff, (void*) msg, count);

	if (HAL_MCP25625_HW_Data_Set(tx_buff, &transfer))
		return MCP25625_TX_ERR;

	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::msg_send(MCP25625_TX_BUFFER_INDEX tx_buff) {
	tx_ctl.buffer = tx_buff;
	tx_ctl.txreq = true;
	tx_ctl.mask = TXB_TXREQ;

	if (HAL_MCP25625_HW_Ctl_Update((void*) &tx_ctl))
		return MCP25625_CTL_ERR;
#if 0
	while (tx_ctl.txreq)
		if (HAL_MCP25625_HW_Ctl_Get((void*) &tx_ctl))
			return MCP25625_HW_ERR;
#endif
	return MCP25625_OK;
}

CAN_INTERFACE_STATUS CANInterface::msg_ready(bool& rxb0_ready, bool& rxb1_ready) {
	MCP25625_RX_STATUS_INFO rx_status;

	CAN_INTERFACE_STATUS s = HAL_MCP25625_Get_Read_Rx_Status(&rx_status);
	if(s!=HAL_OK) {
		rxb0_ready = rxb1_ready = false;
	} else {
		rxb0_ready = rx_status.rxb_0;
		rxb1_ready = rx_status.rxb_1;
	}
	return s;
}

CAN_INTERFACE_STATUS CANInterface::msg_read(MCP25625_RX_BUFFER_INDEX rx_buffer,
		uint8_t *msg, uint8_t *count, uint32_t *ID, bool *IDE, bool *RTR) {
	if (HAL_MCP25625_HW_Data_Get(rx_buffer, &transfer))
		return MCP25625_RX_ERR;

	*IDE = transfer.id.sidl.ide;
	if (*IDE) {
		*RTR = transfer.payload.dlc.rtr;
		CAN_unpack_extended_id(&transfer.id, ID);

	} else {
		*RTR = transfer.id.sidl.srr;
		CAN_unpack_standard_id(&transfer.id, ID);
	}

	*count = transfer.payload.dlc.len;
	memcpy((void*) msg, (void*) transfer.payload.buff, 8);

	ie_ctl.rx0 = false;
	ie_ctl.rx1 = false;
	ie_ctl.mask = (MCP25625_INT_CTL_MASK) (CAN_RX0IE | CAN_RX1IE);
	ie_ctl.reg = REG_INT_FLG;
	if (HAL_MCP25625_HW_Ctl_Update((void*) &ie_ctl))
		return MCP25625_CTL_ERR;

	return MCP25625_OK;
}

CANInterface::~CANInterface() {
}

