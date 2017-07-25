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

/* VMX HAL Serial Port Interface */

#pragma once

#include "types.h"

typedef enum {
  VMXHAL_SerialPort_Rpi = 0,
} VMXHAL_SerialPort;

#ifdef __cplusplus
extern "C" {
#endif

void VMXHAL_InitializeSerialPort(VMXHAL_SerialPort port, int32_t* status);
void VMXHAL_SetSerialBaudRate(VMXHAL_SerialPort port, int32_t baud, int32_t* status);
void VMXHAL_SetSerialDataBits(VMXHAL_SerialPort port, int32_t bits, int32_t* status);
void VMXHAL_SetSerialParity(VMXHAL_SerialPort port, int32_t parity, int32_t* status);
void VMXHAL_SetSerialStopBits(VMXHAL_SerialPort port, int32_t stopBits,
                           int32_t* status);
void VMXHAL_SetSerialWriteMode(VMXHAL_SerialPort port, int32_t mode, int32_t* status);
void VMXHAL_SetSerialFlowControl(VMXHAL_SerialPort port, int32_t flow,
                              int32_t* status);
void VMXHAL_SetSerialTimeout(VMXHAL_SerialPort port, double timeout, int32_t* status);
void VMXHAL_EnableSerialTermination(VMXHAL_SerialPort port, char terminator,
                                 int32_t* status);
void VMXHAL_DisableSerialTermination(VMXHAL_SerialPort port, int32_t* status);
void VMXHAL_SetSerialReadBufferSize(VMXHAL_SerialPort port, int32_t size,
                                 int32_t* status);
void VMXHAL_SetSerialWriteBufferSize(VMXHAL_SerialPort port, int32_t size,
                                  int32_t* status);
int32_t VMXHAL_GetSerialBytesReceived(VMXHAL_SerialPort port, int32_t* status);
int32_t VMXHAL_ReadSerial(VMXHAL_SerialPort port, char* buffer, int32_t count,
                       int32_t* status);
int32_t VMXHAL_WriteSerial(VMXHAL_SerialPort port, const char* buffer, int32_t count,
                        int32_t* status);
void VMXHAL_FlushSerial(VMXHAL_SerialPort port, int32_t* status);
void VMXHAL_ClearSerial(VMXHAL_SerialPort port, int32_t* status);
void VMXHAL_CloseSerial(VMXHAL_SerialPort port, int32_t* status);

#ifdef __cplusplus
}
#endif

