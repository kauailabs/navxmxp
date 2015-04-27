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

#ifndef __STM32_SHIM_H
#define __STM32_SHIM_H

#define i2c_write(a,b,c,d)	stm32_i2c_write(a,b,c,d)
#define i2c_read(a,b,c,d)	stm32_i2c_read(a,b,c,d)
#define fabs(x)     		(((x)>0)?(x):-(x))
#define min(a,b)			((a<b)?a:b)
#define get_ms				stm32_get_ms
#define delay_ms			stm32_delay_ms

//#define SERIAL_OUTPUT_DEVICE SerialUSB // comment out if no debugging required

#ifdef SERIAL_OUTPUT_DEVICE

extern USBSerial SerialUSB;
#define log_i       SERIAL_OUTPUT_DEVICE.println
#define log_e		SERIAL_OUTPUT_DEVICE.println

#else // No Debug Output

#define log_i(...) do { } while (0)
#define log_e(...) do { } while (0)

#endif

#define __no_operation() asm volatile("nop"); // emit STM32 no-op

#ifdef __cplusplus
#define _EXTERN_ATTRIB extern "C"
#else
#define _EXTERN_ATTRIB
#endif

_EXTERN_ATTRIB int stm32_i2c_write(unsigned char slave_addr, unsigned char reg_addr,unsigned char length, unsigned char const *data);
_EXTERN_ATTRIB int stm32_i2c_read(unsigned char slave_addr, unsigned char reg_addr,unsigned char length, unsigned char *data);
_EXTERN_ATTRIB void stm32_get_ms(unsigned long *count);
_EXTERN_ATTRIB int reg_int_cb(struct int_param_s *int_param);
_EXTERN_ATTRIB void stm32_delay_ms(unsigned long ms);

#endif // __STM32_SHIM_H
