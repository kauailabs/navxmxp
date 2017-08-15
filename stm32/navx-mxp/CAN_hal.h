/*
 * can_hal.h
 *
 *  Created on: Jan 20, 2017
 *      Author: Scott
 */

#ifndef CAN_HAL_H_
#define CAN_HAL_H_

#include "stm32f4xx_hal.h"
#include "CAN_hal_types.h"

/*check if the compiler is of C++*/
#ifdef __cplusplus
extern "C" {
#endif

/* MCP25625 CAN Interface Access */

/* Pin-level control */
void HAL_MCP25625_PinReset();
void HAL_MCP25625_PinWake();
void HAL_MCP25625_PinSleep();

/* SPI communication */
HAL_StatusTypeDef HAL_MCP25625_HWReset();
HAL_StatusTypeDef HAL_MCP25625_RTS(MCP25625_TX_BUFFER_INDEX tx_index);
HAL_StatusTypeDef HAL_MCP25625_Read_Status(MCP25625_CAN_QUICK_STATUS* p_status);
HAL_StatusTypeDef HAL_MCP25625_Get_Read_Rx_Status(
		MCP25625_RX_STATUS_INFO* p_status);
HAL_StatusTypeDef HAL_MCP25625_Read(uint8_t reg, uint8_t *buffer,
		uint8_t count);
HAL_StatusTypeDef HAL_MCP25625_Write(uint8_t start_reg, uint8_t *p_data,
		uint8_t count);
HAL_StatusTypeDef HAL_MCP25625_BitModify(uint8_t reg, uint8_t mask,
		uint8_t *value);
HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Set(void *value);
HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Update(void *value);
HAL_StatusTypeDef HAL_MCP25625_HW_Ctl_Get(void *result);
HAL_StatusTypeDef HAL_MCP25625_HW_Data_Set(MCP25625_TX_BUFFER_INDEX num,
		CAN_TRANSFER_PADDED *tx_data);
HAL_StatusTypeDef HAL_MCP25625_HW_Data_Get(MCP25625_RX_BUFFER_INDEX num,
		CAN_TRANSFER_PADDED *rx_data);
HAL_StatusTypeDef HAL_MCP25625_HW_Filter_Set(MCP25625_RX_FILTER_INDEX reg,
		CAN_ID *filter);
HAL_StatusTypeDef HAL_MCP25625_HW_Mask_Set(MCP25625_RX_BUFFER_INDEX reg,
		CAN_ID *mask);
HAL_StatusTypeDef HAL_MCP25625_HW_Filter_Get(MCP25625_RX_FILTER_INDEX reg,
		CAN_ID *filter);
HAL_StatusTypeDef HAL_MCP25625_HW_Mask_Get(MCP25625_RX_BUFFER_INDEX reg,
		CAN_ID *mask);
#ifdef __cplusplus
}
#endif

#endif /* CAN_HAL_H_ */
