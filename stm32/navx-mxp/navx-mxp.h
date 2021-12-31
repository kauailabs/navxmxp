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
#ifndef NAVX_MXP_H_
#define NAVX_MXP_H_

#ifdef __cplusplus
#define _EXTERN_ATTRIB extern "C"
#else
#define _EXTERN_ATTRIB
#endif

typedef void (*loop_func)();
typedef uint8_t *(*register_lookup_func)(uint8_t bank, uint8_t register_offset, uint8_t requested_count, uint16_t* size );
typedef void (*register_write_func)(uint8_t bank, uint8_t register_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_data );

_EXTERN_ATTRIB void nav10_init(int init_power_only);
_EXTERN_ATTRIB void nav10_main();
_EXTERN_ATTRIB void nav10_set_loop(uint8_t bank, loop_func);
_EXTERN_ATTRIB void nav10_set_register_lookup_func(uint8_t bank, register_lookup_func);
_EXTERN_ATTRIB void nav10_set_register_write_func(uint8_t bank, register_write_func);

#endif /* NAVX_MXP_H_ */
