/*
 * can_hal.c
 *
 *  Created on: Jan 20, 2017
 *      Author: Scott
 */

#ifndef CAN_HAL_C_
#define CAN_HAL_C_

#include <string.h>
#include "CAN_hal.h"
#include "gpiomap_navx-pi.h"

/* MCP25625 CAN Interface Access */
/* The CAN is an SPI slave, connected to the
 * STM32 via SPI2, using Mode 0.
 */

extern SPI_HandleTypeDef hspi2; /* TODO:  Relocate this */

#define MCP25625_SPI_TIMEOUT_MS  100

typedef struct {
	uint8_t value;
	uint8_t address;
	uint8_t mask;
	uint8_t buffer;
} reg_t;

static reg_t reg;
static uint8_t mcp25625_read_buffer[255 + 2];
static uint8_t mcp25625_write_buffer[255 + 2];

#define MCP25625_TX( tx_buff_index )            ( (tx_buff_index) * 2 )
#define MCP25625_RX( rx_buff_index )            ( (rx_buff_index) * 4 )
#define MCP25625_RTS( tx_buff_index )           ( 1 << (tx_buff_index) )
#define CTL_TXB( tx_buffer_index, address )     ( ((tx_buffer_index) * 16) + (address) )
#define MCP25625_MASK( rx_mask_index )          ( (rx_mask_index) * 4 + 0x20 )
#define MCP25625_FILTER( rx_filter_index )      ( (rx_filter_index) < 3 ? (rx_filter_index) * 4 : ((rx_filter_index - 3) * 4) + 0x10 )

/* NOTE:  After power on, the MCP25625 automatically enters configuration mode. */
void HAL_MCP25625_PinReset() {
	HAL_GPIO_WritePin(_CAN_RESET_GPIO_Port, _CAN_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(_CAN_RESET_GPIO_Port, _CAN_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(_CAN_RESET_GPIO_Port, _CAN_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
}

void HAL_MCP25625_PinWake() {
	/* To enter the normal mode of operation, apply a low level to the STBY pin. */
	/* After this, the MCP25625 should be reachable on the SPI2 bus. */
	HAL_GPIO_WritePin(CAN_STANDBY_GPIO_Port, CAN_STANDBY_Pin, GPIO_PIN_RESET);
}

void HAL_MCP25625_PinSleep() {
	HAL_GPIO_WritePin(CAN_STANDBY_GPIO_Port, CAN_STANDBY_Pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef HAL_MCP25625_HWReset() {
	uint8_t cmd = MCP25625_RESET_CMD;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, &cmd, sizeof(cmd), MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef HAL_MCP25625_RTS(MCP25625_TX_BUFFER_INDEX tx_index) {
	uint8_t cmd = MCP25625_RTS_CMD | MCP25625_RTS(tx_index);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, &cmd, sizeof(cmd), MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef HAL_MCP25625_Read_Status(MCP25625_CAN_QUICK_STATUS* p_status) {
	uint8_t faststatus_cmd[3];
	uint8_t faststatus_resp[3];
	faststatus_cmd[0] = MCP25625_READ_STAT_CMD;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2,
			faststatus_cmd, faststatus_resp, sizeof(faststatus_cmd),
			MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	if (spi_status == HAL_OK) {
		*p_status = faststatus_resp[1];
	}
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_Get_Read_Rx_Status(
		MCP25625_RX_STATUS_INFO* p_status) {
	uint8_t faststatus_cmd[3] = {0,0,0};
	uint8_t faststatus_resp[3] = {0,0,0};
	faststatus_cmd[0] = MCP25625_RX_STATUS_CMD;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2,
			faststatus_cmd, faststatus_resp, sizeof(faststatus_cmd),
			MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	if (spi_status == HAL_OK) {
		*((uint8_t *) p_status) = faststatus_resp[1];
	}
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_Read(uint8_t reg, uint8_t *buffer, uint8_t count) {
	mcp25625_write_buffer[0] = MCP25625_READ_CMD;
	mcp25625_write_buffer[1] = reg;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2,
			mcp25625_write_buffer, mcp25625_read_buffer, count + 2,
			MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	if (spi_status == HAL_OK) {
		memcpy(buffer, &mcp25625_read_buffer[2], count);
	}
	return spi_status;
}

static uint8_t int_flag_read_cmd[3] = {MCP25625_READ_CMD, REG_INT_FLG, 0};

HAL_StatusTypeDef HAL_MCP25625_IntFlagRead(uint8_t* int_flag) {
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2,
			int_flag_read_cmd, mcp25625_read_buffer, 3,
			MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	if (spi_status == HAL_OK) {
		*int_flag = mcp25625_read_buffer[2];
	}
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_Write(uint8_t start_reg, uint8_t *p_data,
		uint8_t count) {
	mcp25625_write_buffer[0] = MCP25625_WRITE_CMD;
	mcp25625_write_buffer[1] = start_reg;
	memcpy(&mcp25625_write_buffer[2], p_data, count);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(&hspi2,
			mcp25625_write_buffer, count + 2, MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_BitModify(uint8_t reg, uint8_t mask,
		uint8_t *value) {
	uint8_t cmd[4];
	cmd[0] = MCP25625_BIT_MODIFY_CMD;
	cmd[1] = reg;
	cmd[2] = mask;
	cmd[3] = *value;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(&hspi2, cmd, sizeof(cmd), MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Set(void *value) {
	if (*((uint8_t*) value + 1) != REG_CTL_TXB) {
		memcpy((void*) &reg, value, sizeof(reg_t));
		return HAL_MCP25625_Write(reg.address, &reg.value, 1);
	} else {
		memcpy((void*) &reg, value, sizeof(reg_t));
		return HAL_MCP25625_Write(CTL_TXB( reg.buffer, reg.address ),
				&reg.value, 1);
	}
}

HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Update(void *value) {
	if (*((uint8_t*) value + 1) != REG_CTL_TXB) {
		memcpy((void*) &reg, value, sizeof(reg_t));
		return HAL_MCP25625_BitModify(reg.address, reg.mask, &reg.value);
	} else {
		memcpy((void*) &reg, value, sizeof(reg_t));
		return HAL_MCP25625_BitModify(CTL_TXB( reg.buffer, reg.address ),
				reg.mask, &reg.value);
	}
	return 0;
}

HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Get(void *result) {
	HAL_StatusTypeDef spi_status;
	if (*((uint8_t*) result + 1) != REG_CTL_TXB) {
		memcpy((void*) &reg, result, sizeof(reg_t));
		spi_status = HAL_MCP25625_Read(reg.address, &reg.value, 1);
	} else {

		memcpy((void*) &reg, result, sizeof(reg_t));
		spi_status = HAL_MCP25625_Read(CTL_TXB( reg.buffer, reg.address ),
				&reg.value, 1);
	}

	if (spi_status == HAL_OK) {
		memcpy(result, &reg.value, 1);
	}
	return spi_status;
}

HAL_StatusTypeDef HAL_MCP25625_HW_Data_Set(MCP25625_TX_BUFFER_INDEX num,
		CAN_TRANSFER_PADDED *tx_data) {
	tx_data->cmd = MCP25625_LOAD_TX_CMD + MCP25625_TX( num );
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status =  HAL_SPI_Transmit(&hspi2, (uint8_t *)tx_data, 14,
			MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef HAL_MCP25625_HW_Data_Get(MCP25625_RX_BUFFER_INDEX num,
		CAN_TRANSFER_PADDED *rx_data) {
#if 1
	uint8_t cmd = MCP25625_READ_RX_CMD + MCP25625_RX( num );
	rx_data->cmd = cmd;
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)rx_data, (uint8_t *)rx_data, 14, MCP25625_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin( CAN_CS_Port, CAN_CS_Pin, GPIO_PIN_SET);
	rx_data->cmd = cmd; /* DEBUG:  to help w/record-keeping, update cmd. */
	return spi_status;
#else
	uint8_t reg_addr;
	if ( num == RXB0) {
		reg_addr = 0x61;
	} else {
		reg_addr = 0x71;
	}
	return HAL_MCP25625_Read(reg_addr, ((uint8_t *)rx_data)+1, 13);
#endif
}

HAL_StatusTypeDef HAL_MCP25625_HW_Filter_Set(MCP25625_RX_FILTER_INDEX reg,
		CAN_ID *filter) {
	return HAL_MCP25625_Write(MCP25625_FILTER( reg ), (uint8_t*) filter, 4);
}

HAL_StatusTypeDef HAL_MCP25625_HW_Mask_Set(MCP25625_RX_BUFFER_INDEX reg,
		CAN_ID *mask) {
	return HAL_MCP25625_Write(MCP25625_MASK( reg ), (uint8_t*) mask, 4);
}

HAL_StatusTypeDef HAL_MCP25625_HW_Filter_Get(MCP25625_RX_FILTER_INDEX reg,
		CAN_ID *filter) {
	return HAL_MCP25625_Read(MCP25625_FILTER( reg ), (uint8_t*) filter, 4);
}

HAL_StatusTypeDef HAL_MCP25625_HW_Mask_Get(MCP25625_RX_BUFFER_INDEX reg,
		CAN_ID *mask) {
	return HAL_MCP25625_Read(MCP25625_MASK( reg ), (uint8_t*) mask, 4);
}

#endif /* CAN_HAL_C_ */
