/*
 * VMXChannelManager.cpp
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#include <stddef.h>

#include "VMXChannelManager.h"
#include "VMXChannel.h"

#define DI  		VMXChannelCapability::DigitalInput
#define DO			VMXChannelCapability::DigitalOutput
#define PWMOUT 		VMXChannelCapability::PWMGeneratorOutput
#define PWMOUT2		VMXChannelCapability::PWMGeneratorOutput2
#define PWMIN  		VMXChannelCapability::PWMCaptureInput
#define INTIN  		VMXChannelCapability::InterruptInput
#define ENCAIN 		VMXChannelCapability::EncoderAInput
#define ENCBIN 		VMXChannelCapability::EncoderBInput
#define UART_TX		VMXChannelCapability::UART_TX
#define UART_RX		VMXChannelCapability::UART_RX
#define SPI_CLK		VMXChannelCapability::SPI_CLK
#define SPI_MISO	VMXChannelCapability::SPI_MISO
#define SPI_MOSI	VMXChannelCapability::SPI_MOSI
#define SPI_CS		VMXChannelCapability::SPI_CS
#define I2C_SDA		VMXChannelCapability::I2C_SDA
#define I2C_SCL		VMXChannelCapability::I2C_SCL
#define ACCUMIN		VMXChannelCapability::AccumulatorInput
#define ANTRIGIN	VMXChannelCapability::AnalogTriggerInput
#define IODIRSEL	VMXChannelHwOpt::IODirectionSelect

#define IOCX_D		VMXChannelType::FlexDIO
#define IOCX_A		VMXChannelType::AnalogIn
#define PIGPIO		VMXChannelType::HiCurrDIO
#define PICMIO		VMXChannelType::CommDIO
#define PI_I2C		VMXChannelType::CommI2C

#define UNUSED		(VMXChannelCapability)0

#define MAX_VMX_CHANNELS 64

#define CHANHWOPT(i)		   ((uint64_t(i) & 0x00000000000000FF) << 56)
#define CHANTYPE(t) 		   ((uint64_t(t) & 0x00000000000000FF) << 48)
#define CHANIDX(i)			   ((uint64_t(i) & 0x000000000000FFFF) << 32)
#define CHANCAPS(c)			   ((uint64_t(i) & 0x00000000FFFFFFFF) <<  0)

#define EXTRACT_CHANHWOPT(desc) (VMXChannelHwOpt)     ((desc & 0xFF00000000000000) >> 56)
#define EXTRACT_CHANTYPE(desc)  (VMXChannelType)      ((desc & 0x00FF000000000000) >> 48)
#define EXTRACT_CHANIDX(desc)   (uint8_t)             ((desc & 0x0000FFFF00000000) >> 32)
#define EXTRACT_CHANCAPS(desc)  (VMXChannelCapability)((desc & 0x00000000FFFFFFFF) >>  0)

#define CHANID(type,idx)	   (CHANTYPE(type) | CHANIDX(idx))

static const VMXChannelDescriptor channel_descriptors[MAX_VMX_CHANNELS] =
{
	/*  0- 1:  VMX-pi 2-pin QE1 Connector (A-B) */
	CHANID(IOCX_D,  0) | DI | DO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  1) | DI | DO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  2- 3:  VMX-pi 2-pin QE2 Connector (A-B) */
	CHANID(IOCX_D,  2) | DI | DO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  3) | DI | DO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  4- 5:  VMX-pi 2-pin QE3 Connector (A-B) */
	CHANID(IOCX_D,  4) | DI | DO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  5) | DI | DO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  6 -7:  VMX-pi 2-pin QE4 Connector (A-B) */
	CHANID(IOCX_D,  6) | DI | DO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  7) | DI | DO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  8- 11:  VMX-pi 4-pin DIO Header (1-4) */
	CHANID(IOCX_D,  8) | DI | DO | PWMOUT  | PWMIN | INTIN,
	CHANID(IOCX_D,  9) | DI | DO | PWMOUT2 | PWMIN | INTIN,
	CHANID(IOCX_D, 10) | DI | DO | PWMOUT  | PWMIN | INTIN,
	CHANID(IOCX_D, 11) | DI | DO | PWMOUT2 | PWMIN | INTIN,
	/* 12-21:  VMX-pi PWM Header (1-10) [RPI GPIOs] */
	/* Note:  as a group, these are Jumper-selected between DO/PWMOut & DI InterruptInput */
	CHANID(PIGPIO,  0) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  1) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  2) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  3) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  4) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  5) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  6) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  7) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  8) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	CHANID(PIGPIO,  9) | DI | DO | PWMOUT  | INTIN | CHANHWOPT(IODIRSEL),
	/* 22-25:  VMX-pi Analog In Header (1-4) */
	CHANID(IOCX_A,  0) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  1) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  2) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  3) | ACCUMIN | ANTRIGIN | INTIN,
	/* 26-27:  VMX-pi UART Connector (TX/RX) [RPI GPIOs] */
	CHANID(PICMIO,  0) | DI | DO | PWMOUT  | INTIN | UART_TX,
	CHANID(PICMIO,  1) | DI | DO | PWMOUT  | INTIN | UART_RX,
	/* 28-31:  VMX-pi SPI Connector (CLK, MOSI, MISO, CS) [RPI GPIOs] */
	CHANID(PICMIO,  2) | DI | DO | PWMOUT  | INTIN | SPI_CLK,
	CHANID(PICMIO,  3) | DI | DO | PWMOUT  | INTIN | SPI_MISO,
	CHANID(PICMIO,  4) | DI | DO | PWMOUT  | INTIN | SPI_MOSI,
	CHANID(PICMIO,  5) | DI | DO | PWMOUT  | INTIN | SPI_CS,
	/* 32-33:  VMX-pi I2C Connector (SDA, SCL) */
	CHANID(PI_I2C,  0) | I2C_SDA,
	CHANID(PI_I2C,  1) | I2C_SCL
	/* 34-63:  Unused */
};

static uint8_t s_num_iocx_d_channels;
static uint8_t s_num_iocx_a_channels;
static uint8_t s_num_pigpio_channels;
static uint8_t s_num_picmio_channels;
static uint8_t s_num_pi_i2c_channels;

static VMXChannelIndex s_first_iocx_d_channel_index;
static VMXChannelIndex s_first_iocx_a_channel_index;
static VMXChannelIndex s_first_pigpio_channel_index;
static VMXChannelIndex s_first_picmio_channel_index;
static VMXChannelIndex s_first_pi_i2c_channel_index;

typedef struct {
	VMXChannelType channel_type;
	uint8_t *p_count;
	VMXChannelIndex *p_first_channel_index;
} VMXChannelTypeToCountMap;

static VMXChannelTypeToCountMap chan_type_to_count_map[] =
{
		{ IOCX_D, &s_num_iocx_d_channels, &s_first_iocx_d_channel_index },
		{ IOCX_A, &s_num_iocx_a_channels, &s_first_iocx_a_channel_index },
		{ PIGPIO, &s_num_pigpio_channels, &s_first_pigpio_channel_index },
		{ PICMIO, &s_num_picmio_channels, &s_first_picmio_channel_index },
		{ PI_I2C, &s_num_pi_i2c_channels, &s_first_pi_i2c_channel_index },
};

static uint8_t s_num_di_channels;
static uint8_t s_num_do_channels;
static uint8_t s_num_pwm_in_channels;
static uint8_t s_num_pwm_out_channels;
static uint8_t s_num_int_in_channels;
static uint8_t s_num_accum_in_channels;
static uint8_t s_num_an_trig_in_channels;
static uint8_t s_num_encoder_a_in_channels;
static uint8_t s_num_encoder_b_in_channels;
static uint8_t s_num_i2c_sda_channels;
static uint8_t s_num_i2c_scl_channels;
static uint8_t s_num_uart_tx_channels;
static uint8_t s_num_uart_rx_channels;
static uint8_t s_num_spi_clk_channels;
static uint8_t s_num_spi_miso_channels;
static uint8_t s_num_spi_mosi_channels;
static uint8_t s_num_spi_cs_channels;

typedef struct {
	VMXChannelCapability capability;
	uint8_t *p_count;
} VMXChannelCapabilityToCountMap;

static VMXChannelCapabilityToCountMap cap_to_count_map[] =
{
		{ DI, &s_num_di_channels },
		{ DO, &s_num_do_channels },
		{ PWMIN, &s_num_pwm_in_channels },
		{ PWMOUT, &s_num_pwm_out_channels },
		{ INTIN, &s_num_int_in_channels },
		{ ACCUMIN, &s_num_accum_in_channels },
		{ ANTRIGIN, &s_num_an_trig_in_channels },
		{ ENCAIN, &s_num_encoder_a_in_channels },
		{ ENCBIN, &s_num_encoder_b_in_channels },
		{ I2C_SDA, &s_num_i2c_sda_channels },
		{ I2C_SCL, &s_num_i2c_scl_channels },
		{ UART_TX, &s_num_uart_tx_channels },
		{ UART_RX, &s_num_uart_rx_channels },
		{ SPI_CLK, &s_num_spi_clk_channels },
		{ SPI_MISO, &s_num_spi_miso_channels },
		{ SPI_MOSI, &s_num_spi_mosi_channels },
		{ SPI_CS, &s_num_spi_cs_channels },
};

void VMXChannelManager::Init()
{
	for (size_t i = 0; i < (sizeof(cap_to_count_map)/sizeof(cap_to_count_map[0])); i++) {
		*cap_to_count_map[i].p_count = 0;
		for (size_t j = 0; j < MAX_VMX_CHANNELS; j++ ) {
			if (VMXChannelCapabilityCheck(EXTRACT_CHANCAPS(channel_descriptors[j]), cap_to_count_map[i].capability)) {
				(*(cap_to_count_map[i].p_count))++;
			}
		}
	}

	for (size_t i = 0; i < (sizeof(chan_type_to_count_map)/sizeof(chan_type_to_count_map[0])); i++) {
		*chan_type_to_count_map[i].p_count = 0;
		*(chan_type_to_count_map[i].p_first_channel_index) = INVALID_VMX_CHANNEL_INDEX;
		for (size_t j = 0; j < MAX_VMX_CHANNELS; j++ ) {
			if (EXTRACT_CHANTYPE(channel_descriptors[j]) == chan_type_to_count_map[i].channel_type) {
				(*(chan_type_to_count_map[i].p_count))++;
				if (*(chan_type_to_count_map[i].p_first_channel_index) == INVALID_VMX_CHANNEL_INDEX) {
					*(chan_type_to_count_map[i].p_first_channel_index) = j;
				}
			}
		}
	}
}

VMXChannelManager::VMXChannelManager()
{

}

VMXChannelManager::~VMXChannelManager()
{

}

uint8_t VMXChannelManager::GetMaxNumChannels()
{
	return uint8_t((sizeof(cap_to_count_map)/sizeof(cap_to_count_map[0])));
}

uint8_t VMXChannelManager::GetNumChannelsByCapability(VMXChannelCapability capability)
{
	for (size_t i = 0; i < (sizeof(cap_to_count_map)/sizeof(cap_to_count_map[0])); i++) {
		if(cap_to_count_map[i].capability == capability) {
			return *(cap_to_count_map[i].p_count);
		}
	}
	return 0;
}

bool VMXChannelManager::GetChannelTypeAndProviderSpecificIndex(VMXChannelIndex channel_index, VMXChannelType& type, uint8_t& provider_specific_index)
{
	if (channel_index >= MAX_VMX_CHANNELS) return false;

	VMXChannelDescriptor channel_desc = channel_descriptors[channel_index];
	type = EXTRACT_CHANTYPE(channel_desc);
	provider_specific_index = EXTRACT_CHANIDX(channel_desc);

	return (type != VMXChannelType::INVALID);
}

VMXChannelCapability VMXChannelManager::GetChannelCapabilityBits(VMXChannelIndex channel_index)
{
	if (channel_index >= MAX_VMX_CHANNELS) return VMXChannelCapability::None;

	return EXTRACT_CHANCAPS(channel_descriptors[channel_index]);
}

VMXChannelHwOpt VMXChannelManager::GetChannelHwOpts(VMXChannelIndex channel_index) {
	if (channel_index >= MAX_VMX_CHANNELS) return VMXChannelHwOpt::NoHwOptions;

	return EXTRACT_CHANHWOPT(channel_descriptors[channel_index]);
}

uint8_t VMXChannelManager::GetNumChannelsByType(VMXChannelType channel_type, VMXChannelIndex& first_channel_index)
{
	for (size_t i = 0; i < (sizeof(chan_type_to_count_map)/sizeof(chan_type_to_count_map[0])); i++) {
		if (chan_type_to_count_map[i].channel_type == channel_type) {
			first_channel_index = *(chan_type_to_count_map[i].p_first_channel_index);
			return *(chan_type_to_count_map[i].p_count);
		}
	}
	return 0;
}

uint8_t VMXChannelManager::GetNumChannelsByTypeAndCapability(VMXChannelType channel_type, VMXChannelCapability capability, VMXChannelIndex& first_channel_index)
{
	uint8_t match_count = 0;
	for (size_t i = 0; i < MAX_VMX_CHANNELS; i++ ) {
		if (EXTRACT_CHANTYPE(channel_descriptors[i]) == channel_type) {
			if (VMXChannelCapabilityCheck(EXTRACT_CHANCAPS(channel_descriptors[i]), capability)) {
				if (match_count == 0) {
					first_channel_index = VMXChannelIndex(i);
				}
				match_count++;
			}
		}
	}
	return match_count;
}
