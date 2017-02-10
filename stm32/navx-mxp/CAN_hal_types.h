/*
 * CAN_hal_types.h
 *
 *  Created on: Jan 20, 2017
 *      Author: Scott
 */

#ifndef CAN_HAL_TYPES_H_
#define CAN_HAL_TYPES_H_

#include <stdint.h>

#include "CANRegisters.h"

/* Typedefs */

typedef int MCP25625_BOOLEAN;
#define MCP25625_TRUE 1
#define MCP25625_FALSE 0

typedef enum {
	MCP25625_RESET_CMD 		= 0xC0,
	MCP25625_READ_CMD		= 0x03,
	MCP25625_READ_RX_CMD	= 0x90,
	MCP25625_WRITE_CMD		= 0x02,
	MCP25625_LOAD_TX_CMD	= 0x40,
	MCP25625_RTS_CMD		= 0x80,
	MCP25625_READ_STAT_CMD	= 0xA0,
	MCP25625_RX_STATUS_CMD	= 0xB0,
	MCP25625_BIT_MODIFY_CMD	= 0x05,
} MCP25625_COMMAND;

typedef enum {
	RXB0,
	RXB1
} MCP25625_RX_BUFFER_INDEX;

typedef enum {
	TXB0,
	TXB1,
	TXB2,
	TXB_INVALID
} MCP25625_TX_BUFFER_INDEX;

typedef enum {
	RXF_0,
	RXF_1,
	RXF_2,
	RXF_3,
	RXF_4,
	RXF_5
} MCP25625_RX_FILTER_INDEX;

typedef enum {
	REG_CTL_RXP		= 0x0C,
	REG_CTL_RTS     = 0x0D,
	REG_CTL_CAN     = 0x0F,
	REG_CTL_CNF3    = 0x28,
	REG_CTL_CNF2    = 0x29,
	REG_CTL_CNF1    = 0x2A,
	REG_CTL_EFLG    = 0x2D,
	REG_CTL_TXB     = 0x30,
	REG_CTL_RXB0    = 0x60,
	REG_CTL_RXB1    = 0x70,
	REG_INT_CTL     = 0x2B,
	REG_INT_FLG     = 0x2C,
	REG_STAT_CAN	= 0x0E,
	REG_STAT_TEC	= 0x1C,
	REG_STAT_REC	= 0x1D
} MCP25625_REGISTERS;

typedef enum {
	MCP25625_RX_MODE_ALL,
	MCP25625_RX_MODE_SID,
	MCP25625_RX_MODE_EID,
	MCP25625_RX_MODE_OFF
} MCP25625_RX_MODE;

typedef enum {
    INT_CTL                     = 0x2B,
    INT_FLG                     = 0x2C
} MCP25625_INT;

typedef enum {
	INT_NO,
	INT_ERR,
	INT_WAKE,
	INT_TXB0,
	INT_TXB1,
	INT_TXB2,
	INT_RXB0,
	INT_RXB1
} MCP25625_INTERRUPT_STATUS;

typedef enum {
    RTS_P0_MODE                 = 0x01,
    RTS_P1_MODE                 = 0x02,
    RTS_P2_MODE                 = 0x04,
    RTS_P0_STAT                 = 0x08,
    RTS_P1_STAT                 = 0x10,
    RTS_P2_STAT                 = 0x20
} MCP25625_RTS_CTL_MASK;

typedef struct {
	MCP25625_BOOLEAN    p0_mode : 1;
	MCP25625_BOOLEAN    p1_mode : 1;
	MCP25625_BOOLEAN    p2_mode : 1;
	MCP25625_BOOLEAN    p0_stat : 1;
	MCP25625_BOOLEAN    p1_stat : 1;
	MCP25625_BOOLEAN    p2_stat : 1;
    uint8_t                     : 2;
    MCP25625_REGISTERS        	reg;
    MCP25625_RTS_CTL_MASK       mask;
} MCP25625_RTS_CTL;

typedef struct {
	MCP25625_RX_FILTER_INDEX filter	: 3;
	MCP25625_BOOLEAN exide  		: 1;
	MCP25625_BOOLEAN rtr    		: 1;
	uint8_t							: 1;
	MCP25625_BOOLEAN rxb_0  		: 1;
	MCP25625_BOOLEAN rxb_1  		: 1;
} MCP25625_RX_STATUS_INFO;

typedef enum {
	RXP_P0_MODE	= 0x01,
	RXP_P1_MODE = 0x02,
	RXP_P0_ENAB = 0x04,
	RXP_P1_ENAB = 0x08,
	RXP_P0_STAT = 0x10,
	RXP_P1_STAT = 0x20
} MCP25625_RX_PIN_CTL_AND_STATUS_MASK;

typedef enum {
	QSTAT_RX0IF 	= 0x01,
	QSTAT_RX1IF 	= 0x02,
	QSTAT_TX0REQ 	= 0x04,
	QSTAT_TX0IF 	= 0x08,
	QSTAT_TX1REQ 	= 0x10,
	QSTAT_TX1IF 	= 0x20,
	QSTAT_TX2REQ 	= 0x40,
	QSTAT_TX2IF 	= 0x80
} MCP25625_CAN_QUICK_STATUS;

typedef struct {

	MCP25625_BOOLEAN					p0_mode : 1;
	MCP25625_BOOLEAN    				p1_mode : 1;
	MCP25625_BOOLEAN    				p0_enab : 1;
	MCP25625_BOOLEAN    				p1_enab : 1;
	MCP25625_BOOLEAN    				p0_stat : 1;
	MCP25625_BOOLEAN    				p1_stat	: 1;
	uint8_t             						: 2;

	MCP25625_REGISTERS        			reg;
	MCP25625_RX_PIN_CTL_AND_STATUS_MASK	mask;

} MCP25625_RXP_CTL;

typedef enum {
	CAN_CLKPRE                  = 0x03,
	CAN_CLKOUT                  = 0x04,
	CAN_OSM                     = 0x08,
	CAN_ABAT                    = 0x10,
	CAN_REQOP                   = 0xE0
} MCP25625_CAN_CTL_MASK;

typedef struct {
	uint8_t     			clkpre  : 2;
	MCP25625_BOOLEAN        clken   : 1;
	MCP25625_BOOLEAN        osm     : 1;
	MCP25625_BOOLEAN        abat    : 1;
	CAN_MODE    			reqop	: 3;

	MCP25625_REGISTERS      reg;
	MCP25625_CAN_CTL_MASK   mask;
} MCP25625_CAN_CTL;

typedef enum {
	CNF3_PHSEG2	= 0x07,
	CNF3_WAKFIL	= 0x40,
	CNF3_SOFR	= 0x80
} MCP25625_CONFIG3_CTL_MASK;

typedef struct {
	uint8_t     				phseg2  : 3;
	uint8_t                     		: 3;
	MCP25625_BOOLEAN        	wakfil  : 1;
	MCP25625_BOOLEAN        	sof     : 1;

	MCP25625_REGISTERS        	reg;
	MCP25625_CONFIG3_CTL_MASK	mask;
} MCP25625_CONFIG3_CTL;

typedef enum {
	CNF2_PRSEG                   = 0x07,
	CNF2_PHSEG1                  = 0x38,
	CNF2_SAM                     = 0x40,
	CNF2_BTLMODE                 = 0x80
} MCP25625_CONFIG2_CTL_MASK;

typedef struct {
	uint8_t     				prseg   : 3;
	uint8_t     				phseg1  : 3;
	MCP25625_BOOLEAN        	sam     : 1;
	MCP25625_BOOLEAN        	btlmode : 1;

	MCP25625_REGISTERS        	reg;
	MCP25625_CONFIG2_CTL_MASK   mask;
} MCP25625_CONFIG2_CTL;

typedef enum {
	CNF1_BRP                    = 0x3F,
	CNF1_SJW                    = 0xC0
} MCP25625_CONFIG1_CTL_MASK;

typedef struct {
	uint8_t     				brp     : 6;
	uint8_t     				sjw     : 2;

	MCP25625_REGISTERS       	reg;
	MCP25625_CONFIG1_CTL_MASK   mask;
} MCP25625_CONFIG1_CTL;

typedef enum {
	CAN_RX0IE                   = 0x01,
	CAN_RX1IE                   = 0x02,
	CAN_TX0IE                   = 0x04,
	CAN_TX1IE                   = 0x08,
	CAN_TX2IE                   = 0x10,
	CAN_ERRIE                   = 0x20,
	CAN_WAKIE                   = 0x40,
	CAN_MERRE                   = 0x80
} MCP25625_INT_CTL_MASK;

typedef struct {
	uint8_t     rx0             : 1;
	uint8_t     rx1             : 1;
	uint8_t     tx0             : 1;
	uint8_t     tx1             : 1;
	uint8_t     tx2             : 1;
	uint8_t     err             : 1;
	uint8_t     wak             : 1;
	uint8_t     merre           : 1;

	MCP25625_REGISTERS          reg;
	MCP25625_INT_CTL_MASK		mask;
} MCP25625_INT_CTL;

typedef enum {
	FLG_EWARN                   = 0x01,
	FLG_RXWAR                   = 0x02,
	FLG_TXWAR                   = 0x04,
	FLG_RXEP                    = 0x08,
	FLG_TXEP                    = 0x10,
	FLG_TXBO                    = 0x20,
	FLG_RX0OVR                  = 0x40,
	FLG_RX1OVR                  = 0x80
} MCP25625_ERRORFLAG_CTL_MASK;

typedef struct {
	MCP25625_BOOLEAN        ewarn           : 1;
	MCP25625_BOOLEAN        rxwar           : 1;
	MCP25625_BOOLEAN        txwar           : 1;
	MCP25625_BOOLEAN        rxerr           : 1;
	MCP25625_BOOLEAN        txerr           : 1;
	MCP25625_BOOLEAN        txbo            : 1;
	MCP25625_BOOLEAN        rx0ovr          : 1;
	MCP25625_BOOLEAN        rx1ovr          : 1;

	MCP25625_REGISTERS        	reg;
	MCP25625_ERRORFLAG_CTL_MASK mask;
} MCP25625_ERRORFLAG_CTL;

typedef enum {
	TXB_TXP                     = 0x03,
	TXB_TXREQ                   = 0x08,
	TXB_TXERR                   = 0x10,
	TXB_MLOA                    = 0x20,
	TXB_ABTF                    = 0x40
} MCP25625_TXBUFF_CTL_MASK;

typedef struct {
	uint8_t     	 txp        : 2;
	uint8_t                     : 1;
	MCP25625_BOOLEAN txreq      : 1;
	MCP25625_BOOLEAN txerr      : 1;
	MCP25625_BOOLEAN mloa       : 1;
	uint8_t     	 abtf       : 1;
	uint8_t                     : 1;

	MCP25625_REGISTERS        	reg;
	MCP25625_TXBUFF_CTL_MASK    mask;
	MCP25625_TX_BUFFER_INDEX	buffer;
} MCP25625_TXBUFF_CTL;

typedef enum {
    RX0_FILHIT0                 = 0x01,
    RX0_BUKT1                   = 0x02,
    RX0_BUKT                    = 0x04,
    RX0_RTR                     = 0x08,
    RX0_RXM                     = 0X60
} MCP25625_RXBUFF0_CTL_MASK;

typedef struct {
	MCP25625_BOOLEAN   filhit0  : 1;
    MCP25625_BOOLEAN   bukt_1   : 1;
    MCP25625_BOOLEAN   bukt     : 1;
    MCP25625_BOOLEAN   rxrtr    : 1;
    uint8_t                     : 1;
    MCP25625_RX_MODE   rxm      : 3;

    MCP25625_REGISTERS        	reg;
    MCP25625_RXBUFF0_CTL_MASK   mask;
} MCP25625_RXBUFF0_CTL;

typedef enum {
    RX1_FILHIT                  = 0x07,
    RX1_RTR                     = 0x08,
    RX1_RXM                     = 0x60
} MCP25625_RXBUFF1_CTL_MASK;

typedef struct {
	uint8_t				filhit 	:3;
	MCP25625_BOOLEAN 	rxrtr 	:1;
	uint8_t 					:1;
	MCP25625_RX_MODE 	rxm 	:3;

	MCP25625_REGISTERS 			reg;
	MCP25625_RXBUFF1_CTL_MASK 	mask;
} MCP25625_RXBUFF1_CTL;

typedef struct {
    uint8_t                     		: 1;
    MCP25625_INTERRUPT_STATUS	icod	: 3;
    uint8_t                     		: 1;
    CAN_MODE    		  		opmod   : 3;
} MCP25625_CAN_STATUS;

typedef struct {
	MCP25625_CAN_STATUS			value;
	MCP25625_REGISTERS			reg;
	uint8_t						mask;
} MCP25625_CAN_STATUS_REG;

#endif /* CAN_HAL_TYPES_H_ */
