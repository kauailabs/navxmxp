/*
 * CAN.h
 *
 *  Created on: Jan 21, 2017
 *      Author: Scott
 */

#ifndef CAN_H_
#define CAN_H_


#ifdef __cplusplus
#define _EXTERN_ATTRIB extern "C"
#else
#define _EXTERN_ATTRIB
#endif

#include <stdint.h>

_EXTERN_ATTRIB void CAN_init();
_EXTERN_ATTRIB void CAN_loop();
_EXTERN_ATTRIB uint8_t *CAN_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint8_t requested_count, uint16_t* size );
_EXTERN_ATTRIB void CAN_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values );

#endif /* CAN_H_ */
