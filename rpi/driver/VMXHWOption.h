/*
 * VMXHWOption.h
 *
 *  Created on: 21 Jul 2017
 *      Author: pi
 */

#ifndef VMXHWOPTION_H_
#define VMXHWOPTION_H_

#include "VMXResource.h"

struct DIOHWOption : public VMXHWOption {
	bool input_supported;
	bool output_supported;
	DIOHWOption() : VMXHWOption(VMXResourceType::DigitalIO) {
		input_supported = true;
		output_supported = true;
	}
	bool GetInputSupported() { return input_supported; }
	bool GetOutputSupported() { return output_supported; }
};

struct AccumulatorHWOption : public VMXHWOption {
	float voltage_scale;
	AccumulatorHWOption() : VMXHWOption(VMXResourceType::Accumulator) {
		voltage_scale = 3.3f;
	}
	float GetVoltageScale() { return voltage_scale; }
};

#endif /* VMXHWOPTION_H_ */
