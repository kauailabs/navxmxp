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

#define RECEIVE_FIFO_DEPTH	10
#define TRANSMIT_FIFO_DEPTH 10

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
	CAN_MODE      		default_mode;
	CAN_TRANSFER   		transfer;
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

	FIFO<CAN_TRANSFER, RECEIVE_FIFO_DEPTH> rx_fifo;
	FIFO<CAN_DATA, TRANSMIT_FIFO_DEPTH> tx_fifo;
	CAN_ID own_id;

	static void mcp25625_isr(void);
	static CANInterface *p_singleton;
	void interrupt_handler();

public:
	CANInterface(uint16_t stm32_gpio_pin = GPIO_PIN_7);
	CAN_INTERFACE_STATUS init(CAN_MODE mode);
	CAN_INTERFACE_STATUS set_mode(CAN_MODE mode);
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

	CAN_INTERFACE_STATUS tx_config(
			MCP25625_TX_BUFFER_INDEX tx_buff,
	        uint8_t TXP /* Valid range is 0-3, 3 is highest priority */
	);

	CAN_INTERFACE_STATUS rx_config
	(
			MCP25625_RX_BUFFER_INDEX rx_buff,
			MCP25625_RX_MODE mode,
	        bool BUKT	/* True:  RXB0 message will rollover and be written to RXB1 if RXB0 is full */
	);

	CAN_INTERFACE_STATUS filter_config
	(
			MCP25625_RX_FILTER_INDEX rx_filter,
	        uint16_t SID,
	        uint32_t EID,
	        bool EXIDE /* Extended ID Enable bit (True:  Transmit 29-bit EID, False: Transmit 11-bit SID) */
	);

	CAN_INTERFACE_STATUS mask_config
	(
			MCP25625_RX_BUFFER_INDEX rx_mask,
	        uint16_t SID,
	        uint32_t EID
	);

	CAN_INTERFACE_STATUS msg_load
	(
			MCP25625_TX_BUFFER_INDEX tx_buff,
	        uint8_t *msg,
	        uint8_t count,
	        uint32_t ID,
	        bool IDE, /* True:  Extended (29-bit) ID */
	        bool RTR  /* True:  Transmitted Message will be a Remote Transmit Request */
	        		  /* False: Transmitted Message will be a Data Frame */
	);

	CAN_INTERFACE_STATUS msg_send(MCP25625_TX_BUFFER_INDEX tx_buff);

	CAN_INTERFACE_STATUS msg_ready(bool& rxb0_ready, bool& rxb1_ready);

	CAN_INTERFACE_STATUS msg_read
	(
			MCP25625_RX_BUFFER_INDEX rx_buffer,
	        uint8_t *msg,
	        uint8_t *count,
	        uint32_t *ID,
	        bool *IDE, /* True:  Extended (29-bit) ID */
	        bool *RTR  /*  Extended Frame Remote Transmission Request bit     */
	        		   /* (valid only when IDE bit in RXBnSID register is 1). */
	);

	bool clear_rx_overflow();

	bool get_errors(CAN_ERROR_FLAGS& error_flags,
			uint8_t& tx_err_count, uint8_t& rx_err_count);

	void process_transmit_fifo();

	FIFO<CAN_DATA, TRANSMIT_FIFO_DEPTH>& get_tx_fifo() { return tx_fifo; }
	FIFO<CAN_TRANSFER, RECEIVE_FIFO_DEPTH>& get_rx_fifo() { return rx_fifo; }

	CAN_ID& get_own_id() { return own_id; }

	virtual ~CANInterface();
};

#endif /* CANINTERFACE_H_ */
