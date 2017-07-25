/*
 * VMXResourceConfig.h
 *
 *  Created on: 21 Jul 2017
 *      Author: pi
 */

#ifndef VMXRESOURCECONFIG_H_
#define VMXRESOURCECONFIG_H_

#include "VMXResource.h"
#include "Handlers.h"

struct InterruptConfig : public VMXResourceConfig {

	typedef enum { RISING, FALLING, BOTH } InterruptEdge;

	InterruptEdge edge;
	IO_InterruptHandler p_handler;
	void *p_param;
	InterruptConfig() : VMXResourceConfig(VMXResourceType::Interrupt) {
		edge = InterruptEdge::RISING;
		p_handler = 0;
		p_param = 0;
	}
	IO_InterruptHandler GetHandler() { return p_handler; }
	void * GetParam() { return p_param; }
	InterruptEdge GetEdge() { return edge; }

	void SetHandler(IO_InterruptHandler p_handler) { this->p_handler = p_handler; };
	void SetParam(void *p_param) { this->p_param = p_param; }
	void SetEdge(InterruptEdge edge) { this->edge = edge; }

	virtual VMXResourceConfig *GetCopy() const {
		InterruptConfig *p_new = new InterruptConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((InterruptConfig *)p_config);
		return true;
	}

	virtual ~InterruptConfig() {}
};

struct DIOConfig : public VMXResourceConfig {
	typedef enum { PUSHPULL, OPENDRAIN } OutputMode;
	typedef enum { PULLUP, PULLDOWN, NONE } InputMode;

	/* NOTE:  Certain DIOs may be input-only or output-only at the hardware level. */
	/* Therefore it's possible this configuration may fail if an incompatible state is requested. */
	bool input;
	OutputMode outputmode; /* Not all DIO (e.g., rpi) channels can support !pushpull (opendrain) */
	InputMode inputmode;
	DIOConfig() : VMXResourceConfig(VMXResourceType::DigitalIO) {
		input = true;
		outputmode = OutputMode::PUSHPULL;
		inputmode = InputMode::PULLUP;
	}

	bool GetInput() { return input; }
	OutputMode GetOutputMode() { return outputmode; }
	InputMode GetInputMode() { return inputmode; }

	void SetInput(bool input) { this->input = input; }
	void SetInputMode(InputMode inputmode) { this->inputmode = inputmode; }
	void SetOutputMode(OutputMode outputmode) { this->outputmode = outputmode; }

	virtual VMXResourceConfig *GetCopy() const {
		DIOConfig *p_new = new DIOConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((DIOConfig *)p_config);
		return true;
	}
	virtual ~DIOConfig() {}
};

struct PWMGeneratorConfig : public VMXResourceConfig {
	uint32_t frequency_hz;
	PWMGeneratorConfig() : VMXResourceConfig(VMXResourceType::PWMGenerator) {
		frequency_hz = 200;
	}

	uint32_t GetFrequencyHz() { return frequency_hz; }

	void SetFrequencyHz(uint32_t frequency_hz) { this->frequency_hz = frequency_hz; }

	virtual VMXResourceConfig *GetCopy() const {
		PWMGeneratorConfig *p_new = new PWMGeneratorConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((PWMGeneratorConfig *)p_config);
		return true;
	}
	virtual ~PWMGeneratorConfig() {}
};

struct PWMCaptureConfig : public VMXResourceConfig {
	typedef enum { RISING, FALLING } CaptureEdge;
	CaptureEdge edge_type;
	PWMCaptureConfig() : VMXResourceConfig(VMXResourceType::PWMCapture) {
		edge_type = CaptureEdge::RISING;
	}

	CaptureEdge GetCaptureEdge() { return edge_type; }

	void SetCaptureEdge(CaptureEdge edge) { this->edge_type = edge; }

	virtual VMXResourceConfig *GetCopy() const {
		PWMCaptureConfig *p_new = new PWMCaptureConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((PWMCaptureConfig *)p_config);
		return true;
	}
	virtual ~PWMCaptureConfig() {}
};

struct EncoderConfig : public VMXResourceConfig {
	typedef enum { x1, x2, x4 } EncoderEdge;
	EncoderEdge edge_count;
	EncoderConfig() : VMXResourceConfig(VMXResourceType::Encoder) {
		edge_count = EncoderEdge::x4;
	}

	EncoderEdge GetEncoderEdge() { return edge_count; }

	void SetEncoderEdge(EncoderEdge edge) { this->edge_count = edge; }

	virtual VMXResourceConfig *GetCopy() const {
		EncoderConfig *p_new = new EncoderConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((EncoderConfig *)p_config);
		return true;
	}
	virtual ~EncoderConfig() {}
};

struct AccumulatorConfig : public VMXResourceConfig {
	uint8_t num_average_bits;
	uint8_t num_oversample_bits;
	AccumulatorConfig() : VMXResourceConfig(VMXResourceType::Accumulator) {
		num_oversample_bits = 3;
		num_average_bits = 3;
	}

	uint8_t GetNumAverageBits() { return num_average_bits; }
	uint8_t GetNumOversampleBits() { return num_oversample_bits; }

	void SetNumAverageBits(uint8_t num_average_bits) { this->num_average_bits = num_average_bits; }
	void SetNumOversampleBits(uint8_t num_oversample_bits) { this->num_oversample_bits = num_oversample_bits; }

	virtual VMXResourceConfig *GetCopy() const {
		AccumulatorConfig *p_new = new AccumulatorConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((AccumulatorConfig *)p_config);
		return true;
	}
	virtual ~AccumulatorConfig() {}
};

struct AnalogTriggerConfig : public VMXResourceConfig {
	typedef enum { STATE, RISING_EDGE_PULSE, FALLING_EDGE_PULSE } AnalogTriggerMode;

	uint16_t threshold_high; /* 12-bit value (0-4095) */
	uint16_t threshold_low;  /* 12-bit value (0-4095) */
	AnalogTriggerMode mode;

	AnalogTriggerConfig() : VMXResourceConfig(VMXResourceType::AnalogTrigger) {
		threshold_high = 992; /* .8V on a 3.3V scale */
		threshold_low = 2482; /* 2V on a 3.3V scale */
		mode = AnalogTriggerMode::STATE;
	}
	uint16_t GetThresholdHigh() { return threshold_high; }
	uint16_t GetThresholdLow() { return threshold_low; }
	AnalogTriggerMode GetMode() { return mode; }

	void SetThresholdHigh(uint16_t threshold_high) { this->threshold_high = threshold_high; }
	void SetThresholdLow(uint16_t threshold_low) { this->threshold_low = threshold_low; }
	void SetMode(AnalogTriggerMode mode) { this->mode = mode; }

	virtual VMXResourceConfig *GetCopy() const {
		AnalogTriggerConfig *p_new = new AnalogTriggerConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((AnalogTriggerConfig *)p_config);
		return true;
	}
	virtual ~AnalogTriggerConfig() {}
};

struct UARTConfig : public VMXResourceConfig {
	uint32_t baudrate_bps;
	UARTConfig() : VMXResourceConfig(VMXResourceType::UART) {
		baudrate_bps = 57600;
	}

	uint32_t GetBaudrate() { return baudrate_bps; }

	void SetBaudrate(uint32_t baudrate_bps) { this->baudrate_bps = baudrate_bps; }

	virtual VMXResourceConfig *GetCopy() const {
		UARTConfig *p_new = new UARTConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((UARTConfig *)p_config);
		return true;
	}
	virtual ~UARTConfig() {}
};

struct SPIConfig : public VMXResourceConfig {
	uint32_t bitrate_bps;
	uint8_t mode; /* Range:  0-3 */
	bool cs_active_low;
	bool msbfirst;
	SPIConfig() : VMXResourceConfig(VMXResourceType::SPI) {
		bitrate_bps = 1000000;
		mode = 3;
		cs_active_low = true;
		msbfirst = true;
	}

	uint32_t GetBitrate() { return bitrate_bps; }
	uint8_t GetMode() { return mode; }
	bool GetCSActiveLow() { return cs_active_low; }
	bool GetMSBFirst() { return msbfirst; }

	void SetBitrate(uint32_t bitrate) { this->bitrate_bps = bitrate; }
	void SetMode(uint8_t mode) { this->mode = mode % 4; }
	void SetCSActiveLow(bool cs_active_low) { this->cs_active_low = cs_active_low; }
	void SetMSBFirst(bool msbfirst) { this->msbfirst = msbfirst; }

	virtual VMXResourceConfig *GetCopy() const {
		SPIConfig *p_new = new SPIConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((SPIConfig *)p_config);
		return true;
	}
	virtual ~SPIConfig() {}
};

struct I2CConfig : public VMXResourceConfig {
	I2CConfig() : VMXResourceConfig(VMXResourceType::I2C) {}

	virtual VMXResourceConfig *GetCopy() const {
		I2CConfig *p_new = new I2CConfig();
		*p_new = *this;
		return p_new;
	}
	virtual bool Copy(const VMXResourceConfig *p_config) {
		if (p_config->GetResourceType() != this->GetResourceType()) return false;
		*this = *((I2CConfig *)p_config);
		return true;
	}
	virtual ~I2CConfig() {}
};

#endif /* VMXRESOURCECONFIG_H_ */
