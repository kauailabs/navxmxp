/*
 * VMXChannelManager.cpp
 *
 *  Created on: 29 Jun 2017
 *      Author: pi
 */

#include <stddef.h>

#include "VMXChannelManager.h"
#include "VMXChannel.h"

#define DIO  		VMXChannelCapability::DIO
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

#define IOCX_D		VMXChannelType::IOCX_D
#define IOCX_A		VMXChannelType::IOCX_A
#define PIGPIO		VMXChannelType::PIGPIO

#define UNUSED		(VMXChannelCapability)0

#define MAX_VMX_CHANNELS 64

#define EXTRACT_CHANCAPS(desc) (VMXChannelCapability)(desc & 0x00000000FFFFFFFF)
#define CHANTYPE(t) 		   (t << 24)
#define EXTRACT_CHANTYPE(desc) (VMXChannelType)((desc & 0xFFFF000000000000) >> 24)
#define CHANIDX(i)			   (i << 16)
#define EXTRACT_CHANIDX(desc)  (uint8_t)((desc & 0x0000FFFF00000000) >> 16)
#define CHANID(type,idx)	   (CHANTYPE(type) | CHANIDX(idx))

static const VMXChannelDescriptor channel_descriptors[MAX_VMX_CHANNELS] =
{
	/*  0- 3:  VMX-pi 4-pin DIO Header (1-4) */
	CHANID(IOCX_D,  0) | DIO | PWMOUT  | PWMIN | INTIN,
	CHANID(IOCX_D,  1) | DIO | PWMOUT2 | PWMIN | INTIN,
	CHANID(IOCX_D,  2) | DIO | PWMOUT  | PWMIN | INTIN,
	CHANID(IOCX_D,  3) | DIO | PWMOUT2 | PWMIN | INTIN,
	/*  4- 5:  VMX-pi 2-pin QE1 Connector (A-B) */
	CHANID(IOCX_D,  4) | DIO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  5) | DIO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  6- 7:  VMX-pi 2-pin QE2 Connector (A-B) */
	CHANID(IOCX_D,  6) | DIO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  7) | DIO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/*  8- 9:  VMX-pi 2-pin QE3 Connector (A-B) */
	CHANID(IOCX_D,  8) | DIO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D,  9) | DIO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/* 10-11:  VMX-pi 2-pin QE4 Connector (A-B) */
	CHANID(IOCX_D, 10) | DIO | PWMOUT  | PWMIN | INTIN | ENCAIN,
	CHANID(IOCX_D, 11) | DIO | PWMOUT2 | PWMIN | INTIN | ENCBIN,
	/* 12-21:  VMX-pi PWM Header (1-10) [RPI GPIOs] */
	/* Note:  as a group, these are Jumper-selected between DIOOUT/PWMOut & DIOIN & InterruptInput */
	CHANID(PIGPIO,  0) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  1) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  2) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  3) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  4) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  5) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  6) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  7) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  8) | DIO | PWMOUT  | INTIN,
	CHANID(PIGPIO,  9) | DIO | PWMOUT  | INTIN,
	/* 22-23:  VMX-pi UART Connector (TX/RX) [RPI GPIOs] */
	CHANID(PIGPIO, 10) | DIO | UART_TX,
	CHANID(PIGPIO, 11) | DIO | UART_RX,
	/* 24-27:  VMX-pi SPI Connector (CLK, MOSI, MISO, CS) [RPI GPIOs] */
	CHANID(PIGPIO, 12) | DIO | SPI_CLK,
	CHANID(PIGPIO, 13) | DIO | SPI_MISO,
	CHANID(PIGPIO, 14) | DIO | SPI_MOSI,
	CHANID(PIGPIO, 15) | DIO | SPI_CS,
	/* 28-31:  VMX-pi Analog In Header (1-4) */
	CHANID(IOCX_A,  0) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  1) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  2) | ACCUMIN | ANTRIGIN | INTIN,
	CHANID(IOCX_A,  3) | ACCUMIN | ANTRIGIN | INTIN,
	/* 32-33:  VMX-pi I2C Connector (SDA, SCL) */
	CHANID(PIGPIO, 16) | I2C_SDA,
	CHANID(PIGPIO, 17) | I2C_SCL
	/* 34-63:  Unused */
};

static uint8_t s_num_dio_channels;
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
		{ DIO, &s_num_dio_channels },
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
			if (channel_descriptors[j] & cap_to_count_map[i].capability) {
				(*(cap_to_count_map[i].p_count))++;
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
