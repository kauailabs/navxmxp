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
#ifndef IOCX_H_
#define IOCX_H_

#ifdef __cplusplus
#define _EXTERN_ATTRIB extern "C"
#else
#define _EXTERN_ATTRIB
#endif

#include <stdint.h>

_EXTERN_ATTRIB void IOCX_init();
_EXTERN_ATTRIB void IOCX_loop();
_EXTERN_ATTRIB uint8_t *IOCX_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint16_t* size );
_EXTERN_ATTRIB void IOCX_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values );

#endif /* IOCX_H_ */
