/*
 * CANRegisters.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef CANREGISTERS_H_
#define CANREGISTERS_H_

#define CAN_REGISTER_BANK 2

#define NUM_HW_RX_BUFFERS 			 	 2 /* Note: RXB0/RXB1 on the MCP25625 */
#define NUM_ACCEPT_FILTERS_PER_RX_BUFFER 3

typedef struct {
	uint8_t     rx_fifo_nonempty: 1; /* Received data iavailable in rx FIFO   */
	uint8_t     tx_fifo_empty   : 1; /* Successful transmission from tx FIFO  */
	uint8_t     error           : 1; /* Error flags/counts provide details    */
	uint8_t     message_error   : 1; /* Used for Baud Rate determination
										(in LISTEN-only Mode)                 */
	uint8_t     wake			: 1; /* Set in SLEEP mode if CAN bus activity
										occurs (auto-enters LISTEN-only Mode) */
	uint8_t		rx_overflow		: 1; /* Receive overflow.  MUST BE CLEARED in
										order to receive more data.           */
	uint8_t     unused          : 3;
} CAN_IFX_INT_FLAGS;

typedef struct {
	uint8_t		rx_overflow		: 1; /* Receive overflow.  MUST be CLEARED in
										order to receive more data.  See
										CAN_IFX_INT_FLAGS register.           */
	/* NOTE:  The following errors are auto-cleared by the CAN hardware.      */
	uint8_t		can_bus_warn	: 1; /* Rx or Tx error count > 96			  */
	uint8_t     can_bus_err_pasv: 1; /* Rx or Tx error count > 128            */
	uint8_t     can_bux_tx_off  : 1; /* Tx error count == 255.  Note:  Will   */
									 /* auto-recover.  RESET to force clear. */
} CAN_ERROR_FLAGS;

typedef enum {
	CAN_MODE_NORMAL,
	CAN_MODE_SLEEP,
	CAN_MODE_LOOP,
	CAN_MODE_LISTEN,
	CAN_MODE_CONFIG,
} CAN_MODE;

typedef struct {
    uint8_t     eidt	: 2; /* Lowest 2 bits of Extended Identifier */
    uint8_t     unused  : 1;
    uint8_t     ide     : 1; /* Extended Identifier Flag */
    uint8_t     srr     : 1; /* Frame Remote Transmit Request (Standard only) */
    uint8_t     sidl    : 3; /* Lowest 3 bits of Standard Identifier */
} CAN_ID_FLAGS;

typedef struct {
    uint8_t             sidh;
    CAN_ID_FLAGS        sidl;
    uint8_t             eidh;
    uint8_t             eidl;
} CAN_ID;

typedef struct {
    uint8_t     len             : 4;
    uint8_t                     : 2;
    uint8_t     rtr             : 1;
    uint8_t                     : 1;
} CAN_DLC;

typedef struct {
	CAN_DLC        		dlc;
    uint8_t             buff[ 8 ];
} CAN_DATA;

typedef struct {
	CAN_ID         id;
	CAN_DATA       payload;
} CAN_TRANSFER;

typedef struct {
	uint8_t sam : 1; 	 /* True: one sample point is located between PHSEG1 and
	 	 	 	 	 	 	PHSEG2.  Additionally, two samples are taken at one-
	 	 	 	 	 	 	half TQ intervals prior to the end of PHSEG1, with
			 	 	 	 	bit valueg determined by majority decision. */
	uint8_t btlmode : 1; /* PS2 Bit Time Length bit.  True:  Length of PS2
							determined by PHSEG2<2:0> bits of CNF3; False:
							Length is greater of PS1 & IPT (2 Time Quantums) */
	uint8_t wakfil : 1;  /* True: Wake-up filter enabled */
	uint8_t unused: 5;
} CAN_ADV_CONFIG;

#define MAX_TX_ENTRIES		10 /* See CANRegisters.h */
#define MAX_RX_ENTRIES		10 /* See CANRegisters.h */

/* Interrupt pin:  Active Low, held until interrupt cleared in int flags */

#define BIT_MODIFY_COMMAND_REG 0x7F

/* BitModify SPI command:  allows contents of any single register to be
   modified using a bit mask.  Typically used for interrupt acknowledgement.
   Format: [Bank] [0x80 | 0x7F] [Count=3] [RegAddr] [Mask Byte] [DataByte] */

#define RESET_COMMAND 0x5A

struct __attribute__ ((__packed__)) CAN_REGS {
	/****************************/
	/* Capabilities (Read-only) */
	/****************************/
	uint16_t capability_flags;	/* TBD */

	/*********************************************/
	/* Interrupt Status/Acknowledge (Read/Write) */
	/*********************************************/
	CAN_IFX_INT_FLAGS int_flags; /* Note:  Use BitModify command to modify. */

	/**********************/
	/* Status (Read-only) */
	/**********************/
	uint8_t tx_fifo_entry_count; /* (0 to MAX_TX_ENTRIES) */
	uint8_t rx_fifo_entry_count; /* (0 to MAX_RX_ENTRIES) */
	CAN_ERROR_FLAGS error_flags;
	uint8_t tx_err_count;
	uint8_t rx_err_count;

	/******************************/
	/* Transmit FIFO (Write-only) */
	/******************************/
	uint8_t tx_fifo_head; /* Up to MAX_TX_ENTRIES of CAN_DATA entries */
	/****************************/
	/* Receive FIFO (Read-only) */
	/****************************/
	uint8_t rx_fifo_tail; /* Up to MAX_RX_ENTRIES of CAN_TRANSFER entries */

	/************************************/
	/* Basic Configuration (Read/Write) */
	/************************************/
	CAN_MODE opmode;
	CAN_IFX_INT_FLAGS int_enable;
	CAN_ID tx_id;

	/************************************/
	/* Reset (Write-only) */
	/************************************/
	uint8_t reset;	/* Set to RESET_COMMAND to force reset. */

	/*****************************************************************/
	/* Advanced Configuration (Read/Write)                           */
	/*                                                               */
	/* NOTE:  Must be in CAN_MODE_CONFIG for changes to take effect! */
	/*****************************************************************/
	/*-- Acceptance Filters/Masks - see MCP25625 Datasheet section 3.7.5. --*/
	CAN_ID accept_mask[NUM_HW_RX_BUFFERS];
	CAN_ID accept_filter[NUM_HW_RX_BUFFERS][NUM_ACCEPT_FILTERS_PER_RX_BUFFER];
	/*-- Bit Timing Configuration - see MCP25625 Datasheet section 3.8. --*/
	uint8_t baud_rate_prescaler; /* (0-63) */
	uint8_t sjw; /* Synchronization Jump Width Length (0-3) */
	/* Propagation/Phase Time Quantums (range 0-7 [= Time Quantum +1]) */
	uint8_t prseg; /* Propagation Segment.  7 allows 40m bus length. */
	uint8_t phseg1;	/* Phase Segment 1 compensates for phase shift on edges. */
	uint8_t phseg2; /* Phase Segment 2 compensates for phase shift on edges. */
	/* Advanced Configuration Flags */
	CAN_ADV_CONFIG adv_config;
	uint8_t end_of_bank;
};

inline void CAN_unpack_standard_id(CAN_ID *id, uint32_t *STANDARD_ID) {
	*STANDARD_ID = 0;
	*STANDARD_ID |= id->sidh;
	*STANDARD_ID <<= 3;
	*STANDARD_ID |= id->sidl.sidl;
}

inline void CAN_unpack_extended_id(CAN_ID *id, uint32_t *EXTENDED_ID) {
	*EXTENDED_ID = 0;
	*EXTENDED_ID |= id->sidh;
	*EXTENDED_ID <<= 3;
	*EXTENDED_ID |= id->sidl.sidl;
	*EXTENDED_ID <<= 2;
	*EXTENDED_ID |= id->sidl.eidt;
	*EXTENDED_ID <<= 8;
	*EXTENDED_ID |= id->eidh;
	*EXTENDED_ID <<= 8;
	*EXTENDED_ID |= id->eidl;
}

inline void CAN_pack_standard_id(uint32_t SID, CAN_ID *id) {
	id->sidh = (SID & 0x07F8) >> 3;
	id->sidl.sidl = (SID & 0x0007);
}

inline void CAN_pack_extended_id(uint32_t EID, CAN_ID *id) {
	id->sidh = (EID & 0x1FE00000) >> 21;
	id->sidl.sidl = (EID & 0x001C0000) >> 18;
	id->sidl.eidt = (EID & 0x00030000) >> 16;
	id->eidh = (EID & 0x0000FF00) >> 8;
	id->eidl = (EID & 0x000000FF);
}

#endif /* CANREGISTERS_H_ */
