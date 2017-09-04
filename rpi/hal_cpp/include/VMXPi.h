/* ============================================
VMX-pi HAL source code is placed under the MIT license
Copyright (c) 2017 Kauai Labs
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

#ifndef VMXPI_H_
#define VMXPI_H_

#include <stdint.h>
#include "AHRS.h"
#include "VMXCAN.h"
#include "VMXIO.h"
#include "VMXPower.h"
#include "VMXTime.h"
#include "VMXVersion.h"
#include "VMXThread.h"

class VMXPiImpl;

class VMXPi {

	VMXPiImpl*	p_impl;

public:

	AHRS 		ahrs;
	VMXTime 	time;
	VMXIO 		io;
	VMXCAN		can;
	VMXPower	power;
	VMXVersion	version;
	VMXThread	thread;

	VMXPi(bool realtime, uint8_t ahrs_update_rate_hz);
	virtual ~VMXPi();

	bool IsOpen();
};

#endif /* VMXPI_H_ */
