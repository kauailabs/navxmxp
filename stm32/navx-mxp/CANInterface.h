/*
 * CANInterface.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef CANINTERFACE_H_
#define CANINTERFACE_H_

#include "stm32f4xx_hal.h"
#include "CAN_hal_types.h"
#include "FIFO.h"

typedef int CAN_INTERFACE_STATUS;

#define MCP25625_OK             0
#define MCP25625_MODE_FAULT     1
#define MCP25625_HW_ERR         2
#define MCP25625_TX_ERR         3
#define MCP25625_RX_ERR         4
#define MCP25625_COM_ERR    	6
#define MCP25625_CTL_ERR    	7
#define MCP25625_RTS_ERR    	8
#define MCP25625_RXP_ERR    	9

#define RECEIVE_FIFO_DEPTH	254
#define TRANSMIT_FIFO_DEPTH 64

typedef void (*CAN_interrupt_flag_func)(CAN_IFX_INT_FLAGS mask, CAN_IFX_INT_FLAGS flags);

typedef struct __attribute__ ((__packed__)) {
	uint8_t		   			   cmd;
	TIMESTAMPED_CAN_TRANSFER   transfer;
} TIMESTAMPED_CAN_TRANSFER_PADDED;

class CANInterface {

	MCP25625_CAN_CTL     		can_ctl;
	MCP25625_RTS_CTL     		rts_pins;
	MCP25625_RXP_CTL     		rx_pins;
	MCP25625_INT_CTL     		ie_ctl;
	MCP25625_INT_CTL     		ie_ctl_isr;
	MCP25625_TXBUFF_CTL			tx_ctl;
	MCP25625_RXBUFF0_CTL 		rx0_ctl;
	MCP25625_RXBUFF1_CTL 		rx1_ctl;
	MCP25625_CONFIG1_CTL 		cnf_1;
	MCP25625_CONFIG2_CTL 		cnf_2;
	MCP25625_CONFIG3_CTL 		cnf_3;
	MCP25625_ERRORFLAG_CTL		eflg_ctl;
	MCP25625_ERRORFLAG_CTL		eflg_ctl_isr;
	CAN_MODE      				default_mode;
	CAN_MODE					current_mode;
	CAN_TRANSFER_PADDED			write_transfer;
	TIMESTAMPED_CAN_TRANSFER_PADDED read_transfer;
	MCP25625_CAN_STATUS			can_stat;
	MCP25625_RX_STATUS_INFO  	rx_status;
	MCP25625_CAN_STATUS_REG 	status_reg;
	bool						tx_pending[3];

	uint32_t message_error_interrupt_count;
	uint32_t error_interrupt_count;
	uint32_t wake_interrupt_count;
	uint32_t tx0_complete_count;
	uint32_t tx1_complete_count;
	uint32_t tx2_complete_count;
	uint32_t rx0_complete_count;
	uint32_t rx1_complete_count;
	uint32_t rx_buff_full_count;
	uint32_t more_interrupt_pending_count;
	uint32_t bus_off_count;
	bool     bus_off;
	uint32_t tx0_sent_count;
	uint32_t tx1_sent_count;
	uint32_t tx2_sent_count;
	uint32_t tx_fifo_highwatermark_count;
	uint32_t tx0_err_count;
	uint32_t tx1_err_count;
	uint32_t tx2_err_count;

	FIFO<TIMESTAMPED_CAN_TRANSFER_PADDED, RECEIVE_FIFO_DEPTH> rx_fifo;
	FIFO<CAN_TRANSFER_PADDED, TRANSMIT_FIFO_DEPTH> tx_fifo;

	static void mcp25625_isr(uint8_t);
	static CANInterface *p_singleton;
	void interrupt_handler();

	CAN_interrupt_flag_func p_isr_flag_func;

public:
	CANInterface(uint16_t stm32_gpio_pin = GPIO_PIN_4);
	CAN_INTERFACE_STATUS init(CAN_MODE mode);
	CAN_MODE get_current_can_mode() { return current_mode; }
	CAN_INTERFACE_STATUS clear_all_interrupt_flags();
	CAN_INTERFACE_STATUS clear_error_interrupt_flags();
	CAN_INTERFACE_STATUS flush_rx_fifo();
	CAN_INTERFACE_STATUS flush_tx_fifo();
	CAN_INTERFACE_STATUS set_mode(CAN_MODE mode, bool disable_interrupts = true);
	CAN_INTERFACE_STATUS com_error(bool clear);
	CAN_INTERFACE_STATUS btl_config
	(
		uint8_t BRP, 	/* Baud Rate Prescaler (0-63)                                                    */
		uint8_t SJW, 	/* Synchronization Jump Width Length (0-3)                                       */
		uint8_t PRSEG, 	/* Propagation Segment, 0-7 (+1 Time Quantums); 7 allows 40m bus length.         */
		uint8_t PHSEG1,	/* Phase Segment 1 compensates for phase shift on edges (0-7) [+1 Time Quantums] */
		uint8_t PHSEG2, /* Phase Segment 2 compensates for phase shift on edges (0-7) [+1 Time Quantums] */
		bool SAM, 	  	/* True: one sample point is located between PHSEG1 and PHSEG2.  Additionally,   */
				  	  	/* two samples are taken at one-half TQ intervals prior to the end of PHSEG1,    */
				 	 	/* with the value of the bit being determined by majority decision.              */
		bool BTLMODE,   /* PS2 Bit Time Length bit.  True:  Length of PS2 determined by                  */
					    /* PHSEG2<2:0> bits of CNF3; False:  Length is the greater of PS1 and IPT (2 TQ) */
		bool WAKFIL,  	/* True: Wake-up filter enabled                                                  */
		bool SOFR 	  	/* True: = CLKOUT pin enabled for Start-of-Frame (SOF) signal                    */
	);

	CAN_INTERFACE_STATUS get_btl_config(uint8_t& BRP, uint8_t& SJW,
			uint8_t& PRSEG, uint8_t& PHSEG1, uint8_t& PHSEG2, bool& SAM, bool& BTLMODE,
			bool& WAKFIL, bool& SOFR);

	CAN_INTERFACE_STATUS rx_config
	(
			MCP25625_RX_BUFFER_INDEX rx_buff,
			MCP25625_RX_MODE mode,
	        bool BUKT	/* True:  RXB0 message will rollover and be written to RXB1 if RXB0 is full */
	);

	CAN_INTERFACE_STATUS filter_config
	(
			MCP25625_RX_FILTER_INDEX rx_filter,
	        CAN_ID *p_id,
	        CAN_ID *p_id_out
	);

	CAN_INTERFACE_STATUS mask_config
	(
			MCP25625_RX_BUFFER_INDEX rx_mask,
	        CAN_ID *p_id,
	        CAN_ID *p_id_out
	);
	CAN_INTERFACE_STATUS msg_load(MCP25625_TX_BUFFER_INDEX tx_buff,
		CAN_TRANSFER_PADDED *p_tx);

	CAN_INTERFACE_STATUS msg_send(MCP25625_TX_BUFFER_INDEX tx_buff);
	CAN_INTERFACE_STATUS get_tx_control(MCP25625_TX_BUFFER_INDEX tx_buff,
			MCP25625_TXBUFF_CTL& txbuff_ctl);
	CAN_INTERFACE_STATUS get_quick_status(MCP25625_CAN_QUICK_STATUS& status);

	bool clear_rx_overflow(bool disable_interrupts = false);

	uint32_t get_bus_off_count() { return bus_off_count; }

	/* Returns true if any error conditions (further specified in the
	 * output parameters from this function) exist.
	 */
	bool get_errors(bool& rx_overflow, CAN_ERROR_FLAGS& error_flags,
			uint8_t& tx_err_count, uint8_t& rx_err_count);

	void process_transmit_fifo();

	FIFO<CAN_TRANSFER_PADDED, TRANSMIT_FIFO_DEPTH>& get_tx_fifo() { return tx_fifo; }
	FIFO<TIMESTAMPED_CAN_TRANSFER_PADDED, RECEIVE_FIFO_DEPTH>& get_rx_fifo() { return rx_fifo; }

	void register_interrupt_flag_function(CAN_interrupt_flag_func p_isr_flag_func);

	inline void disable_CAN_interrupts() {
		HAL_NVIC_DisableIRQ((IRQn_Type)EXTI4_IRQn);
	}

	inline void enable_CAN_interrupts() {
		HAL_NVIC_EnableIRQ((IRQn_Type)EXTI4_IRQn);
	}

	void enable_controller_interrupts();
	void disable_controller_interrupts();

	virtual ~CANInterface();
};

#endif /* CANINTERFACE_H_ */
