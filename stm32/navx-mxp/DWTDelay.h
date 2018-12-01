/*
 * Simple microseconds delay routine, utilizingARM's DWT
 * (Data Watchpoint and Trace Unit) and HAL library.
 */

#include <stdint.h>

#ifndef INC_DWT_DELAY_H_
#define INC_DWT_DELAY_H_

void DWT_Init(void);
void DWT_Delay(uint32_t us);

#endif /* INC_DWT_DELAY_DWT_DELAY_H_ */
