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

#ifndef VMXPOWER_H_
#define VMXPOWER_H_

#include "VMXErrors.h"

class MISCClient;

class VMXPower {
	friend class VMXPi;

	MISCClient& misc;

	VMXPower(MISCClient& misc);
	void ReleaseResources();
	virtual ~VMXPower();

public:
	bool GetOvercurrent(bool& overcurrent, VMXErrorCode *errcode = 0);
	bool GetSystemVoltage(float& ext_power_volts, VMXErrorCode *errcode = 0);
	bool GetOvercurrentLimitEnabled(bool& enabled, VMXErrorCode *errcode = 0);
	bool SetOvercurrentLimitEnabled(bool enabled, VMXErrorCode *errcode = 0);
};

#endif /* VMXPOWER_H_ */
