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

#include "FlashStorage.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include <string.h>
#include <stdlib.h>

struct FlashMemoryPage
{
	char 		signature[4]; 		/* 'KLab' */
	uint16_t 	data_size;			/* Valid memory size */
	uint8_t 	first_data_byte;	/* First valid data byte */
};

FlashStorageClass::FlashStorageClass(void)
{
	ShadowMemorySize = 0;
	ShadowMemory = 0;
	PageBase = 0;
	PageSize = 0;
	SectorNumber = 0;
}

uint16_t FlashStorageClass::init(		uint32_t shadow_memorysize,
										uint8_t  page_sector_number,
										uint32_t page_base,
										uint32_t page_size
										)
{
	SectorNumber = page_sector_number;
	PageBase = page_base;
	PageSize = page_size;
	ShadowMemory = (uint8_t *)malloc(shadow_memorysize);
	if ( ShadowMemory != 0 ) {
		ShadowMemorySize = shadow_memorysize;
	}
	return 0;
}

uint8_t *FlashStorageClass::get_mem( bool& valid, uint16_t& data_size )
{
	// look for the signature in the flash memory page.
	const FlashMemoryPage *pflashpage = (const FlashMemoryPage *)PageBase;
	if ( ( pflashpage->signature[0] == 'K' ) &&
		 ( pflashpage->signature[1] == 'l' ) &&
		 ( pflashpage->signature[2] == 'a' ) &&
		 ( pflashpage->signature[3] == 'b' ) &&
		 ( pflashpage->data_size == ShadowMemorySize ) ) {
		data_size = pflashpage->data_size;
		memcpy(ShadowMemory, &(pflashpage->first_data_byte), data_size);
		valid = true;
	} else {
		valid = false;
		data_size = ShadowMemorySize;
		memset(ShadowMemory,0xFF,data_size);
	}
	return ShadowMemory;
}

bool FlashStorageClass::erase()
{
	FLASH_EraseInitTypeDef erase_spec;
	uint32_t sector_error;
	HAL_StatusTypeDef status;
	HAL_FLASH_Unlock();

   // Clear pending flags (if any)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	erase_spec.TypeErase = TYPEERASE_SECTORS;
	erase_spec.NbSectors = 1;
	erase_spec.Sector = SectorNumber;
	erase_spec.VoltageRange = VOLTAGE_RANGE_3;
	status = HAL_FLASHEx_Erase(&erase_spec,&sector_error);
	HAL_FLASH_Lock();
	return (status == HAL_OK);
}

uint16_t FlashStorageClass::commit()
{
	if ( erase() ) {

		FlashMemoryPage page;
		page.signature[0] = 'K';
		page.signature[1] = 'l';
		page.signature[2] = 'a';
		page.signature[3] = 'b';
		page.data_size = ShadowMemorySize;

		HAL_FLASH_Unlock();

		/* Signature */
		uint8_t *flashmem = (uint8_t *)PageBase;
		for ( size_t i = 0; i < sizeof(page.signature); i++ ) {
			HAL_FLASH_Program(TYPEPROGRAM_BYTE,(uint32_t)flashmem++,page.signature[i]);
		}
		/* Data (written after size halfword in address space, but before size halfword in time) */
		uint16_t *flashmem_uint16 = (uint16_t *)(PageBase + sizeof(page.signature) + sizeof(uint16_t));
		uint16_t *src_data = (uint16_t *)ShadowMemory;
		for ( int i = 0; i < page.data_size / sizeof(uint16_t); i++ ) {
			HAL_FLASH_Program(TYPEPROGRAM_HALFWORD,(uint32_t)flashmem_uint16++,*src_data++);
		}
		/* Size (this is written last in order, to enable "torn write" (e.g. reset during write) cases. */
		flashmem_uint16 = (uint16_t *)(PageBase + sizeof(page.signature));
		HAL_FLASH_Program(TYPEPROGRAM_HALFWORD,(uint32_t)flashmem_uint16,page.data_size);

		HAL_FLASH_Lock();
		return HAL_OK;
	} else {
		return HAL_ERROR;
	}
}

uint16_t FlashStorageClass::otp_data_present()
{
	uint16_t bitfield = 0;
	uint16_t *p_otp_data_bytes =
			(uint16_t *)0x1FFF7800;
	for ( int i = 0; i < MAX_OTP_BLOCK_NUMBER; i++ ) {

		uint16_t all_ones = 0;
		for ( int j = 0; j < 16; j++ ) {
			uint16_t data_byte = *p_otp_data_bytes;
			if ( data_byte == EEPROM_ERASED) {
				all_ones |= (1 << j);
			}
			p_otp_data_bytes++;
		}
		if ( all_ones != 0xFFFF ) {
			bitfield |= (1 << i);
		}
	}
	return bitfield;
}

#define LOCK_BYTE_UNLOCKED	((uint8_t)0xFF)
#define LOCK_BYTE_LOCKED    ((uint8_t)0x00)

/* Returns a bitmask, 1 = otp data block is locked.                */
uint16_t FlashStorageClass::otp_data_locked()
{
	uint16_t bitfield = 0;
	uint8_t *p_otp_lock_bytes =
			(uint8_t *)0x1FFF7A00;
	for ( int i = 0; i < MAX_OTP_BLOCK_NUMBER; i++ ) {
		uint8_t lock_byte = *p_otp_lock_bytes;
		if ( lock_byte == LOCK_BYTE_LOCKED) {
			bitfield |= (1 << i);
		}
		p_otp_lock_bytes++;
	}
	return bitfield;
}
/* Returns a constant pointer to the specified otp memory block.   */
/* This is a pointer to constant memory.  If needing to edit the   */
/* memory, use the set_otp_memory() function below.                */
const uint8_t *FlashStorageClass::get_otp_mem( uint8_t block_num, size_t& size, bool& locked)
{
	const uint8_t *otp_mem = 0;
	if (( block_num >= MIN_OTP_BLOCK_NUMBER) && ( block_num <= MAX_OTP_BLOCK_NUMBER))
	{
		otp_mem = (const uint8_t *)(0x1FFF7800 + (block_num * 32));
		size = 32;
		uint32_t otp_locked = otp_data_locked();
		if ( otp_locked & (1 << block_num)) {
			locked = true;
		} else {
			locked = false;
		}
	}
	return otp_mem;
}
/* Edits the otp memory.  Please note that only "1" bits can be    */
/* modified, once set to 0, any bit cannot be changed back to a 1. */
/* Note that this function does NOT lock the otp                   */
/* Note that this function will fail if the otp block is locked.   */
bool     FlashStorageClass::set_otp_mem( uint8_t block_num, uint8_t *data, size_t size )
{
	const uint32_t *otp_mem;
	if ( (size <= 32) && ( block_num >= MIN_OTP_BLOCK_NUMBER) && ( block_num <= MAX_OTP_BLOCK_NUMBER))
	{
		otp_mem = (const uint32_t *)(0x1FFF7800 + (block_num * 32));
		uint32_t otp_locked = otp_data_locked();
		if ( otp_locked & (1 << block_num)) {
			return false;
		}
		HAL_FLASH_Unlock();
		uint32_t *src_data_mem = (uint32_t *)data;
		for ( size_t i = 0; i < size / sizeof(uint32_t); i++) {
			HAL_FLASH_Program(TYPEPROGRAM_WORD,(uint32_t)otp_mem,*src_data_mem);
			src_data_mem++;
			otp_mem++;
		}
		HAL_FLASH_Lock();
		return true;
	}
	return false;
}

/* Locks the specified otp block.  After the block is locked, the  */
/* set_otp_mem() function will fail on this block.                 */
/* Note that this funciton will fail if the otp block has already  */
/* been locked previously.                                         */
bool FlashStorageClass::lock_otp( uint8_t block_num )
{
	if (( block_num >= MIN_OTP_BLOCK_NUMBER) && ( block_num <= MAX_OTP_BLOCK_NUMBER))
	{
		uint32_t otp_locked = otp_data_locked();
		if ( otp_locked & (1 << block_num)) {
			return false;
		} else {
			uint8_t *p_otp_lock_byte =
					(uint8_t *)(0x1FFF7A00 + block_num);
			*p_otp_lock_byte = LOCK_BYTE_LOCKED;
			return true;
		}
	}
	return false;
}
