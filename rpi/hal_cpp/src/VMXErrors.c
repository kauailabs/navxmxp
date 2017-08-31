/*
 * VMXErrors.c
 *
 *  Created on: 28 Jul 2017
 *      Author: pi
 */

#include "VMXErrors.h"

const char *GetVMXErrorString(VMXErrorCode errcode)
{
	switch(errcode) {
	case VMXERR_IO_INVALID_RESOURCE_HANDLE:
		return "Invalid Resource Handle";
	case VMXERR_IO_INVALID_CHANNEL_INDEX:
		return "Invalid Channel Index";
	case VMXERR_IO_CHANNEL_ALREADY_ROUTED:
		return "Channel already routed";
	case VMXERR_IO_CHANNEL_RESOURCE_INCOMPATIBILITY:
		return "Channel-Resource Incompatibility";
	case VMXERR_IO_NO_AVAILABLE_RESOURCE_PORTS:
		return "No available Resource Ports";
	case VMXERR_IO_LOGICAL_ROUTING_ERROR:
		return "Logical routing error";
	case VMXERR_IO_PHYSICAL_ROUTING_ERROR:
		return "Physical routing error";
	case VMXERR_IO_RESOURCE_NOT_ALLOCATED:
		return "Resource not allocated";
	case VMXERR_IO_RESOURCE_ALREADY_ALLOCATED:
		return "Resource already allocated";
	case VMXERR_IO_SHARED_RESOURCE_ALREADY_ALLOCATED:
		return "Shared Resource already allocated";
	case VMXERR_IO_RESOURCE_ALLOCATION_ERROR:
		return "Resource allocation error";
	case VMXERR_IO_RESOURCE_ROUTING_INCOMPLETE:
		return "Resource routing incomplete";
	case VMXERR_IO_RESOURCE_TYPE_INVALID:
		return "Resource Type invalid";
	case VMXERR_IO_ERROR_CONFIGURING_PHYSICAL_RESOURCE:
		return "Error configuring physical resource";
	case VMXERR_IO_SHARED_RESOURCE_UNROUTABLE:
		return "Resource is shared and cannot be routed to";
	case VMXERR_IO_INVALID_RESOURCE_PORT_INDEX:
		return "Invalid Resource Port Index";
	case VMXERR_IO_BOARD_COMM_ERROR:
		return "Board Communication Error";
	case VMXERR_IO_RESOURCE_INACTIVE:
		return "Resource inactive.";
	case VMXERR_IO_CHANNEL_NOT_ROUTED_TO_RESOURCE:
		return "Channel not routed to resource.";
	case VMXERR_IO_UNITARY_CHANNEL_CAP_REQUIRED:
		return "Only a single Channel Capability is allowed";
	case VMXERR_IO_NO_COMPATIBLE_RESOURCES:
		return "No compatible Resources found";
	case VMXERR_IO_NO_UNALLOCATED_COMPATIBLE_RESOURCES:
		return "No compatible Resources currently available";
	case VMXERR_IO_INVALID_NULL_PARAMETER:
		return "Invalid NULL Parameter";
	case VMXERR_IO_INVALID_CHANNEL_TYPE:
		return "Invalid Channel type";
	case VMXERR_IO_INVALID_RESOURCE_PROVIDER_TYPE:
		return "Invalid Resource provider type";
	case VMXERR_IO_ERROR_ACQUIRING_RESOURCE_CONFIG:
		return "Error acquiring Resource configuration";
	case VMXERR_IO_INTERNAL_LOGIC_ERROR:
		return "Internal Logic Error";
	case VMXERR_CAN_NO_AVAILABLE_MASKFILTER_SLOTS:
		return "No available CAN Mask/Filter slots";
	case VMXERR_CAN_INVALID_RECEIVE_STREAM_HANDLE:
		return "Invalid CAN Receive stream handle";
	default:
		return "Unknown Error";
	}
}



