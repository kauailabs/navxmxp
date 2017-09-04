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

#ifndef VMXERRORS_H_
#define VMXERRORS_H_

typedef int VMXErrorCode;

#define VMXERR_IO_INVALID_RESOURCE_HANDLE 				-20000
#define VMXERR_IO_INVALID_CHANNEL_INDEX   				-20001
#define VMXERR_IO_CHANNEL_ALREADY_ROUTED  				-20002
#define VMXERR_IO_CHANNEL_RESOURCE_INCOMPATIBILITY		-20003
#define VMXERR_IO_NO_AVAILABLE_RESOURCE_PORTS			-20004
#define VMXERR_IO_LOGICAL_ROUTING_ERROR					-20005
#define VMXERR_IO_PHYSICAL_ROUTING_ERROR				-20006
#define VMXERR_IO_RESOURCE_NOT_ALLOCATED				-20007
#define VMXERR_IO_RESOURCE_ALREADY_ALLOCATED			-20008
#define VMXERR_IO_SHARED_RESOURCE_ALREADY_ALLOCATED		-20009
#define VMXERR_IO_RESOURCE_ALLOCATION_ERROR				-20010
#define VMXERR_IO_RESOURCE_ROUTING_INCOMPLETE			-20011
#define VMXERR_IO_RESOURCE_TYPE_INVALID					-20012
#define VMXERR_IO_ERROR_CONFIGURING_PHYSICAL_RESOURCE	-20013
#define VMXERR_IO_SHARED_RESOURCE_UNROUTABLE			-20014
#define VMXERR_IO_INVALID_RESOURCE_PORT_INDEX			-20015
#define VMXERR_IO_INVALID_RESOURCE_TYPE					-20016
#define VMXERR_IO_INVALID_RESOURCE_INDEX				-20017
#define VMXERR_IO_BOARD_COMM_ERROR						-20018
#define VMXERR_IO_RESOURCE_INACTIVE						-20019
#define VMXERR_IO_CHANNEL_NOT_ROUTED_TO_RESOURCE		-20020
#define VMXERR_IO_UNITARY_CHANNEL_CAP_REQUIRED			-20021
#define VMXERR_IO_NO_COMPATIBLE_RESOURCES				-20022
#define VMXERR_IO_NO_UNALLOCATED_COMPATIBLE_RESOURCES	-20023
#define VMXERR_IO_INVALID_NULL_PARAMETER				-20024
#define VMXERR_IO_INVALID_CHANNEL_TYPE					-20025
#define VMXERR_IO_INVALID_RESOURCE_PROVIDER_TYPE		-20026
#define VMXERR_IO_ERROR_ACQUIRING_RESOURCE_CONFIG		-20027
#define VMXERR_IO_INTERNAL_LOGIC_ERROR					-20028
#define VMXERR_CAN_NO_AVAILABLE_MASKFILTER_SLOTS		-20029
#define VMXERR_CAN_INVALID_RECEIVE_STREAM_HANDLE		-20030
#define VMXERR_IO_UART_WRITE_ERROR						-20031
#define VMXERR_IO_UART_READ_ERROR						-20032
#define VMXERR_IO_SPI_XFER_ERROR						-20033
#define VMXERR_IO_I2C_XFER_ERROR						-20034

#ifdef __cplusplus
extern "C" {
#endif

const char *GetVMXErrorString(VMXErrorCode);

#ifdef __cplusplus
}
#endif

#define SET_VMXERROR(vmx_errcode_ptr, vmx_errcode) \
	do { \
		if(vmx_errcode_ptr) { \
			*vmx_errcode_ptr = vmx_errcode; \
		} \
	} while(0)

#endif /* VMXERRORS_H_ */
