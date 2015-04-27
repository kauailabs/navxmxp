/* ============================================
navX MXP source code is placed under the MIT license
Copyright (c) 2015 Kauai Labs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "inv_mpu.h" /* Invensense Motion Driver Header */

#include "stm32_shim.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_it.h"
extern I2C_HandleTypeDef hi2c2; /* TODO:  Clean up this external reference. */
#include "ext_interrupts.h"

#define I2C_SHIM_TIMEOUT_MS 	100

_EXTERN_ATTRIB int stm32_i2c_write(unsigned char slave_addr, unsigned char reg_addr,unsigned char length, unsigned char const *data)
{
	int result = ( ( HAL_I2C_Mem_Write(&hi2c2,slave_addr<<1,reg_addr,I2C_MEMADD_SIZE_8BIT,(uint8_t *)data,length,I2C_SHIM_TIMEOUT_MS) == HAL_OK ) ? 0 : -1);
	if (result == -1 ) {
		result--;
	}
	return result;
}

_EXTERN_ATTRIB int stm32_i2c_read(unsigned char slave_addr, unsigned char reg_addr,unsigned char length, unsigned char *data)
{
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(&hi2c2,slave_addr<<1,reg_addr,I2C_MEMADD_SIZE_8BIT,(uint8_t *)data,length,I2C_SHIM_TIMEOUT_MS);
	if ( status != HAL_OK ) {
		status = HAL_TIMEOUT;
	}
	return ((status == HAL_OK ) ? 0 : -1);
}

_EXTERN_ATTRIB void stm32_get_ms(unsigned long *count)
{
	*count =  HAL_GetTick();
}

_EXTERN_ATTRIB int reg_int_cb(struct int_param_s *int_param)
{
	attachInterrupt(int_param->pin, int_param->cb,RISING);
	return 0;
}

_EXTERN_ATTRIB void stm32_delay_ms(unsigned long ms)
{
	HAL_Delay(ms);
}

