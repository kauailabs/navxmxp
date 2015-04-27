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

#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include <stdint.h>
#include <stddef.h>

#define EEPROM_PAGE_SECTOR		1
#define EEPROM_PAGE_SIZE 		(uint16_t)0x4000 	/* Page size = 16kbyte*/
#define EEPROM_START_ADDRESS 	((uint32_t)(0x8004000))

/* Page status definitions */
#define EEPROM_ERASED			((uint16_t)0xFFFF)	/* PAGE is empty */

#define EEPROM_DEFAULT_DATA		0xFFFF

#define MIN_OTP_BLOCK_NUMBER 0
#define MAX_OTP_BLOCK_NUMBER 15

/* FlashStorageClass manages a single Flash memory page, and also a */
/* region of 'shadow' RAM that can be smaller than the Flash memory.*/
/*																	*/
/* The FlashStorageClass can perform the following operations:      */
/* - Determine if valid data exists in the Flash memory page, and   */
/*   the size of that data. [data_present(), get_mem()]             */
/* - Read the valid data from the Flash memory page [init()].       */
/* - (re-)Write data to the Flash memory page.  This is accomplished*/
/*   by erasing the Flash memory page, then copying the contents of */
/*   shadow memory to the flash page.   [commit()]                  */
/*                                                                  */
/* NOTE:  Shadow memory is allocated via malloc(), and the caller   */
/* should not free() this memory.                                   */
/*                                                                  */
/* One-time-programmable flash memory access                        */
/*																	*/
/* Provides read and (one-time write) access to the one-time        */
/* programmable memory region on the STM32.                         */

class FlashStorageClass
{
public:
	FlashStorageClass(void);

	/* Flash memory interface */

	uint16_t init(		uint32_t shadow_memorysize,
						uint8_t  page_sector_number = EEPROM_PAGE_SECTOR,
						uint32_t page_base = EEPROM_START_ADDRESS,
						uint32_t page_size = EEPROM_PAGE_SIZE
						);

	uint8_t *get_mem(   bool& valid, uint16_t& data_size);
	uint16_t commit();
	bool erase();

	/* One-time-programmable (OTP) interface                     */
	/*                                                           */
	/* There are 16 blocks of 32-bytes each.  Each block can NOT */
	/* be erased.  Once "locked", the block cannot be written    */
	/* to.  Additionally, before being locked, only "1" bits can */
	/* be changed to zero.  This basically means the memory can  */
	/* only be written to one time.                              */

	/* Returns a bitmask, 1 = otp data present (at least one zero bit) */
	uint16_t otp_data_present();
	/* Returns a bitmask, 1 = otp data block is locked.                */
	uint16_t otp_data_locked();
	/* Returns a constant pointer to the specified otp memory block.   */
	/* This is a pointer to constant memory.  If needing to edit the   */
	/* memory, use the set_otp_memory() function below.                */
	const uint8_t *get_otp_mem( uint8_t block_num, size_t& size, bool& locked);
	/* Edits the otp memory.  Please note that only "1" bits can be    */
	/* modified, once set to 0, any bit cannot be changed back to a 1. */
	/* Note that this function does NOT lock the otp                   */
	/* Note that this function will fail if the otp block is locked.   */
	bool     set_otp_mem( uint8_t block_num, uint8_t *data, size_t size );
	/* Locks the specified otp block.  After the block is locked, the  */
	/* set_otp_mem() function will fail on this block.                 */
	/* Note that this funciton will fail if the otp block has already  */
	/* been locked previously.                                         */
	bool lock_otp( uint8_t block_num );

private:
	uint8_t  SectorNumber;
	uint32_t PageBase;
	uint32_t PageSize;
	uint8_t *ShadowMemory;
	uint16_t ShadowMemorySize;
};

#endif	/* __EEPROM_H */
