/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 *  @file ext_interrupts.c
 *
 *  @brief Wiring-like interface for external interrupts
 */

#include "ext_interrupts.h"

static void (*func_array[16])(void) = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
};

extern "C" {
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	for ( int i = 0; i < 16; i++ ) {
		if ( GPIO_Pin & (1 << i) ) {
			if ( func_array[i] != 0 ) {
				func_array[i]();
			}
			break;
		}
	}
}
}

/**
 * @brief Attach an interrupt handler to a pin, triggering on the given mode.
 * @param pin     Pin to attach an interrupt handler onto.
 * @param handler Function to call when the external interrupt is triggered.
 * @param mode    Trigger mode for the given interrupt.
 * @see ExtIntTriggerMode
 */
void attachInterrupt(uint16_t pin, void (*func)(void), ExtIntTriggerMode mode) {
	for ( int i = 0; i < 16; i++ ) {
		if ( pin & (1 << i) ) {
			func_array[i] = func;
			break;
		}
	}
}

/**
 * @brief Disable any external interrupt attached to a pin.
 * @param pin Pin number to detach any interrupt from.
 */
void detachInterrupt(uint8_t pin) {
	for ( int i = 0; i < 16; i++ ) {
		if ( pin & (1 << i) ) {
			func_array[i] = 0;
			break;
		}
	}
}
