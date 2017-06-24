/* ============================================
VMX HAL source code is placed under the MIT license
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

/* VMX HAL I2C Port Interface */

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void VMXHAL_InitializeI2C(VMXHAL_I2CPort port, int32_t* status);

int32_t VMXHAL_TransactionI2C(VMXHAL_I2CPort port, int32_t deviceAddress,
                           uint8_t* dataToSend, int32_t sendSize,
                           uint8_t* dataReceived, int32_t receiveSize);

int32_t VMXHAL_WriteI2C(VMXHAL_I2CPort port, int32_t deviceAddress,
                     uint8_t* dataToSend, int32_t sendSize);

int32_t VMXHAL_ReadI2C(VMXHAL_I2CPort port, int32_t deviceAddress, uint8_t* buffer,
                    int32_t count);

void VMXHAL_CloseI2C(VMXHAL_I2CPort port);

#ifdef __cplusplus
}
#endif

