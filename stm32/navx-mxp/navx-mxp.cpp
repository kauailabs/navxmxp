/* ============================================
navX MXP source code is placed under the MIT license
Copyright (c) 2015 Kauai Labs

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
extern "C" {
#include "navx-mxp_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
}

#include "navx-mxp.h"
#include "version.h"
#include "revision.h"
#include "usb_serial.h"
#include "HardwareSerial.h"
#include "ext_interrupts.h"

#include "VMXPiTestJigProtocol.h"
#include "VMXPiTestJigRegisters.h"

#include "CANInterface.h"
#include "CAN.h"

extern I2C_HandleTypeDef hi2c3; /* External I2C Interface */
extern SPI_HandleTypeDef hspi1; /* External SPI Interface */

TIM_HandleTypeDef TimHandle; /* Internal Timer */

void SpiTxTimeoutCheck();
void Reset_SPI_And_PrepareToReceive();
void i2c_hard_reset();

int prepare_i2c_receive_timeout_count = 0;
unsigned long prepare_i2c_receive_last_attempt_timestamp = 0;
bool i2c_rx_in_progress = false;
uint32_t last_i2c_rx_start_timestamp = 0;
int i2c_glitch_count = 0;
HAL_StatusTypeDef prepare_next_i2c_receive();

#define RXBUFFERSIZE 64
uint8_t i2c3_RxBuffer[RXBUFFERSIZE];
uint8_t spi1_RxBuffer[RXBUFFERSIZE];

#define NUM_SPI_BANKS 5

loop_func p_loop_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL, NULL };
register_lookup_func p_reg_lookup_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL, NULL };
register_write_func p_reg_write_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL, NULL };

_EXTERN_ATTRIB void nav10_set_loop(uint8_t bank, loop_func p) { p_loop_func[bank] = p; }
_EXTERN_ATTRIB void nav10_set_register_lookup_func(uint8_t bank, register_lookup_func p) { p_reg_lookup_func[bank] = p; }
_EXTERN_ATTRIB void nav10_set_register_write_func(uint8_t bank, register_write_func p) { p_reg_write_func[bank] = p; }

#define UART_RX_PACKET_TIMEOUT_MS 	30 /* Max wait after start of packet */
#define MIN_UART_MESSAGE_LENGTH		TEST_JIG_CONFIG_MESSAGE_LENGTH

#define		SPI_RECV_LENGTH 3	/* SPI Requests:  [0x80 | RegAddr] [Count (Readd) or Data (Write)] [CRC] (Bank 0 is implicitly used) */
#define     MAX_VALID_BANK  0

static bool ext_spi_init_complete = false;
bool spi_transmitting = false;                  // TRUE:  SPI Slave Transmit in progress (awaiting Master to complete Read)

#define NUM_STREAMING_INTERFACES 1

char update_type[NUM_STREAMING_INTERFACES] = { MSGID_TEST_JIG_UPDATE };
char update_buffer[NUM_STREAMING_INTERFACES][TEST_JIG_PROTOCOL_MAX_MESSAGE_LENGTH * 3];	/* Buffer for outbound serial update messages. */
char inbound_data[TEST_JIG_PROTOCOL_MAX_MESSAGE_LENGTH];			/* Buffer for inbound serial messages.  */
char response_buffer[NUM_STREAMING_INTERFACES][TEST_JIG_PROTOCOL_MAX_MESSAGE_LENGTH * 2];  /* Buffer for building serial response. */

unique_id chipid;

/* Protocol Registers are externally visible via I2C and SPI protocols. */
/* NOTE:  Each structure member MUST match the definitions in           */
/* IMUProtocol_Registers.h                                              */

struct __attribute__ ((__packed__)) nav10_protocol_registers {
    /* Constants */
    uint8_t                 identifier;
    uint8_t                 hw_rev;
    uint8_t                 fw_major;
    uint8_t                 fw_minor;
    uint8_t					commdio_op_mode;		// CommDIOInput, CommDIOOutput, CommDIOEcho
    uint8_t					commdio_input_values;
    uint8_t					commdio_output_values;
} registers, shadow_registers;


uint8_t *flashdata = 0;

static const uint8_t flash_fast = 0b10101010;
static const uint8_t flash_slow = 0b11111110;
static const uint8_t flash_on   = 0b11111111;
static const uint8_t flash_off  = 0b00000000;

volatile int8_t cal_led_cycle_index = 7;
volatile uint8_t cal_led_cycle = flash_on;
bool led_update_override = false;
uint8_t old_cal_led_cycle = flash_off;

void override_current_led_cycle( uint8_t new_cal_led_cycle )
{
    led_update_override = true;
    old_cal_led_cycle = cal_led_cycle;
    cal_led_cycle = new_cal_led_cycle;
    cal_led_cycle_index = 7;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    HAL_CAL_LED_On((cal_led_cycle & (1 << cal_led_cycle_index)) ? 1 : 0);
    cal_led_cycle_index--;
    if (cal_led_cycle_index < 0) {
        cal_led_cycle_index = 7;
        /* If override occurred, revert to previous */
        if ( led_update_override ) {
            led_update_override = false;
            cal_led_cycle = old_cal_led_cycle;
        }
    }
}

uint8_t crc_lookup_table[256];

// Uppermost 3 bits are flags  [31-29]
#define CAN_MSG_DEVICE_TYPE_ID			10 // Miscellaneous (Next 5 bits [28-24])
#define CAN_MSG_DEVICE_MFR_ID    		9  // Kauai Labs (Next 8 bits) [23-16]
#define CAN_MSG_VMX_PI_TEST_JIG_API		0  // VMX-pi Test Jig API Index (Next 6 bits) [15-10]
#define CAN_MSG_VMX_PI_TEST_JIG_CONFIG	0  // "Config" message (Next 4 bits) [9-6]
#define CAN_MSG_VMX_PI_TEST_JIG_UPDATE	1  // "Update" message
#define CAN_MSG_DEVICE_NUMBER			0  // Currently only one VMX-pi Test Jig/bus is supported [5-0]

uint32_t CAN_VMX_PI_TEST_JIG_MSG_FILTER = 	(CAN_MSG_DEVICE_TYPE_ID << 24) |
											(CAN_MSG_DEVICE_MFR_ID << 16) |
											(CAN_MSG_VMX_PI_TEST_JIG_API << 10);

uint32_t CAN_VMX_PI_TEST_JIG_MSG_MASK   =	(0x1F << 24) |
											(0xFF << 16) |
											(0x3F << 10);

uint32_t CAN_VMX_PI_TEST_JIG_CONFIG_MSG	=	(CAN_MSG_DEVICE_TYPE_ID << 24) |
											(CAN_MSG_DEVICE_MFR_ID << 16) |
											(CAN_MSG_VMX_PI_TEST_JIG_API << 10) |
											(CAN_MSG_VMX_PI_TEST_JIG_CONFIG << 6) |
											(CAN_MSG_DEVICE_NUMBER << 0);

uint32_t CAN_VMX_PI_TEST_JIG_UPDATE_MSG	=	(CAN_MSG_DEVICE_TYPE_ID << 24) |
											(CAN_MSG_DEVICE_MFR_ID << 16) |
											(CAN_MSG_VMX_PI_TEST_JIG_API << 10) |
											(CAN_MSG_VMX_PI_TEST_JIG_UPDATE << 6) |
											(CAN_MSG_DEVICE_NUMBER << 0);

static void reset_can_prepare_to_receive() {
    CAN_command(CAN_CMD_RESET);
    CAN_loop(); // TODO:  Can this be removed?

    CAN_ID rx_vmx_pi_test_jig_msg_filter;
    CAN_ID rx_vmx_pi_test_jig_msg_msg;
    CAN_pack_extended_id(CAN_VMX_PI_TEST_JIG_MSG_FILTER, &rx_vmx_pi_test_jig_msg_filter);
    CAN_pack_extended_id(CAN_VMX_PI_TEST_JIG_MSG_MASK, &rx_vmx_pi_test_jig_msg_msg);
    uint8_t rx_buffer_index = 0;
    uint8_t rx_filter_index = 0;
    CAN_set_buffer_filter_mode(rx_buffer_index, CAN_RX_FILTER_EID_ONLY);
    CAN_set_filter(rx_buffer_index, rx_filter_index, rx_vmx_pi_test_jig_msg_filter);
    CAN_set_mask(rx_buffer_index, rx_vmx_pi_test_jig_msg_msg);
    CAN_loop(); // TODO:  Can this be removed?

    CAN_command(CAN_CMD_FLUSH_RXFIFO);
    CAN_command(CAN_CMD_FLUSH_TXFIFO);
    CAN_loop(); // TODO:  Can this be removed?

    CAN_set_opmode(CAN_MODE_NORMAL);
    CAN_loop();
}


_EXTERN_ATTRIB void nav10_init()
{
    VMXPiTestJigRegisters::buildCRCLookupTable(crc_lookup_table, sizeof(crc_lookup_table));

    registers.hw_rev		= HAL_GetBoardRev();
    registers.identifier	= NAVX_MODEL_NAVX_MXP;
    registers.fw_major		= NAVX_MXP_FIRMWARE_VERSION_MAJOR;
    registers.fw_minor		= NAVX_MXP_FIRMWARE_VERSION_MINOR;
    read_unique_id(&chipid);

    __TIM11_CLK_ENABLE();
    /* Set Interrupt Group Priority; this is a low-priority interrupt */
    HAL_NVIC_SetPriority((IRQn_Type)TIM1_TRG_COM_TIM11_IRQn, 6, 0);
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ((IRQn_Type)TIM1_TRG_COM_TIM11_IRQn);
    /* Initialize low-priority, 125ms timer */
    TimHandle.Instance = TIM11;
    /* Compute the prescaler value to have TIM11 counter clock equal to 10 KHz */
    uint32_t uwPrescalerValue = (uint32_t) (SystemCoreClock / 10000) - 1;
    /* Initialize TIM11 peripheral for 250ms Interrupt */
    TimHandle.Init.Period = 2500 - 1;
    TimHandle.Init.Prescaler = uwPrescalerValue;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TimHandle);
    HAL_TIM_Base_Start_IT(&TimHandle); /* Enable timer interrupt */

    HAL_LED_Init();

    HAL_LED1_On(0);
    HAL_LED2_On(0);

    /* Set default register state */
    registers.commdio_op_mode = TEST_JIG_OPMODE_INPUT;
    registers.commdio_input_values = 0;

    HAL_CommDIOPins_ConfigForInput();

    /* Configure device and external communication interrupts */

    GPIO_InitTypeDef GPIO_InitStruct;

    /* Configure GPIO pin : PC9 (CAL Button) for dual-edge interrupts */
    GPIO_InitStruct.Pin = CAL_BTN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CAL_BTN_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt initialization */
    /* This set of interrupts includes the CAN interrupt */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4/*NVIC_PRIORITYGROUP_0*/);
    HAL_NVIC_SetPriority((IRQn_Type)EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);

    /* Enable EXTI on pins 10-15, if necessary */
    if(CAL_BTN_Pin > GPIO_PIN_9) {
        HAL_NVIC_SetPriority((IRQn_Type)EXTI15_10_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ((IRQn_Type)EXTI15_10_IRQn);
    }

    HAL_IOCX_HIGHRESTIMER_Init();  // Enable hi-res timer, which is used by the CAN interface.

    /* Initialize the CAN interface */
    CAN_init();

    reset_can_prepare_to_receive();
}

unsigned long last_scan = 0;
bool calibration_active = false;
unsigned long cal_button_pressed_timestamp = 0;
unsigned long reset_imu_cal_buttonpress_period_ms = 2000;

#define BUTTON_DEBOUNCE_SAMPLES 10

static void cal_button_isr(uint8_t gpio_pin)
{
	// TODO:  Determine what (if anything) should be done when the CAL buton is pressed
}

unsigned long start_timestamp = 0;	/* The time the "main" started. */

bool new_opmode_request = false;
char new_opmode_request_value = 0;
bool new_output_request = false;
uint8_t new_output_request_value = 0;

PrintSerialReceiver* serial_interfaces[NUM_STREAMING_INTERFACES] = {&SerialUSB};
uint32_t last_packet_start_rx_timestamp[NUM_STREAMING_INTERFACES] = {0};
int num_serial_interfaces = 1;
uint32_t i2c_bus_reset_count = 0;
uint32_t i2c_interrupt_reset_count = 0;

uint8_t curr_opmode = TEST_JIG_OPMODE_INPUT;
uint8_t curr_output = 0;

_EXTERN_ATTRIB void nav10_main()
{
    attachInterrupt(CAL_BTN_Pin,&cal_button_isr,CHANGE);

    if ( start_timestamp == 0 ) {
        start_timestamp = HAL_GetTick();
    }

    CAN_TRANSFER test_jig_update_msg;

    while (1)
    {
        //bool send_stream_response[1] = { false };
        int num_update_bytes[1] = { 0 };

        SpiTxTimeoutCheck();

        if (CAN_get_tx_fifo_entry_count() == 255) {
        	reset_can_prepare_to_receive();
        }

        // Process all recently received CAN messages
        if (CAN_get_rx_fifo_entry_count() > 0) {
        	TIMESTAMPED_CAN_TRANSFER rx_transfer;
        	while(!CAN_get_next_rx_transfer(&rx_transfer)) {
        		uint32_t received_eid;
        		CAN_unpack_extended_id(&rx_transfer.transfer.id, &received_eid);
        		// TODO:  Do any bits (top 3, lowest 6) need to be masked off here?
        		if (received_eid == CAN_VMX_PI_TEST_JIG_CONFIG_MSG) {
        			if (rx_transfer.transfer.payload.dlc.len >= 2) {
						new_opmode_request_value = (char)rx_transfer.transfer.payload.buff[0];
						new_opmode_request = true;
						new_output_request_value = rx_transfer.transfer.payload.buff[1];
						new_output_request = true;
        			}
        		}
        	}
        }

		// Process inbound USB requests
		for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
			/* Scan for start of message, discarding any bytes that precede it. */
			bool found_start_of_message = false;
			while ( serial_interfaces[ifx]->available() > 0 ) {
				char rcv_byte = serial_interfaces[ifx]->peek();
				if ( rcv_byte != PACKET_START_CHAR ) {
					serial_interfaces[ifx]->read();
				}
				else {
					if ( last_packet_start_rx_timestamp[ifx] == 0 ) {
						last_packet_start_rx_timestamp[ifx] = HAL_GetTick();
					}
					found_start_of_message = true;
					break;
				}
			}

			/* If packet start recently found, but insufficient bytes have been */
			/* received after waiting the maximum packet rx time, flush the     */
			/* packet start Indicator.                                          */
			if ( found_start_of_message &&
					serial_interfaces[ifx]->available() < MIN_UART_MESSAGE_LENGTH &&
					HAL_GetTick() - last_packet_start_rx_timestamp[ifx] > (uint32_t)UART_RX_PACKET_TIMEOUT_MS ) {
				/* Flush the start of message indicator. */
				serial_interfaces[ifx]->read();
				last_packet_start_rx_timestamp[ifx] = 0;
				break;
			}

			/* If sufficient bytes have been received, process the data and */
			/* if a valid message is received, handle it.                   */

			if( found_start_of_message && ( serial_interfaces[ifx]->available() >= MIN_UART_MESSAGE_LENGTH ) ) {
				size_t bytes_read = 0;
				last_packet_start_rx_timestamp[ifx] = 0;
				while ( serial_interfaces[ifx]->available() ) {
					if ( bytes_read >= sizeof(inbound_data) ) {
						break;
					}
					inbound_data[bytes_read++] = serial_interfaces[ifx]->read();
				}

				/* If this is a binary message, the packet length is encoded. */
				/* In this case, wait for additional bytes to arrive, with a  */
				/* timeout in case of error.                                  */

				if ( inbound_data[1] == BINARY_PACKET_INDICATOR_CHAR ) {
					char expected_len = inbound_data[2] + 2;
					if ( bytes_read < expected_len ) {
						uint32_t start_wait_timestamp = HAL_GetTick();
						while ( bytes_read < expected_len ) {
							if ( serial_interfaces[ifx]->available() ) {
								inbound_data[bytes_read++] = serial_interfaces[ifx]->read();
							} else {
								if ( HAL_GetTick() - start_wait_timestamp > (uint32_t)UART_RX_PACKET_TIMEOUT_MS ) {
									break;
								}
							}
						}
					}
				}

				size_t i = 0;
				/* Scan the buffer looking for valid packets */
				while ( i < bytes_read )
				{
					int bytes_remaining = bytes_read - i;
					int packet_length = 0;
					char requested_opmode;
					uint8_t requested_output;
					if ( (packet_length = VMXPiTestJigProtocol::decodeTestJigConfig(
							&inbound_data[i], bytes_remaining,
							requested_opmode, requested_output ) )  )
					{
						if ((requested_opmode == TEST_JIG_OPMODE_INPUT) ||
							(requested_opmode == TEST_JIG_OPMODE_OUTPUT) ||
							(requested_opmode == TEST_JIG_OPMODE_ECHO)) {
							new_opmode_request = true;
							new_opmode_request_value = requested_opmode;
						}

				        new_output_request = true;
				        new_output_request_value = requested_output;
					}
					if ( packet_length > 0 ) {
						i += packet_length;
					} else {
						i++;
					}
				}
			}
		}

        // Process any new configuration requests
        if (new_opmode_request) {
        	if (new_opmode_request_value != curr_opmode) {
    		    i2c_rx_in_progress = false;
    		    spi_transmitting = false;
        		if (new_opmode_request_value == TEST_JIG_OPMODE_INPUT) {
        		    ext_spi_init_complete = false;
        	        Serial6.end();
        			HAL_CommDIOPins_ConfigForInput();
                	curr_output = 0;
        		} else if (new_opmode_request_value == TEST_JIG_OPMODE_OUTPUT) {
        		    ext_spi_init_complete = false;
        	        Serial6.end();
        			HAL_CommDIOPins_ConfigForOutput();
        		} else if (new_opmode_request_value == TEST_JIG_OPMODE_ECHO) {
        			//HAL_CommDIOPins_ConfigForCommunication();
        			// Prepare to receive I2C/SPI interrupt-driven data
        			i2c_hard_reset();
        			prepare_next_i2c_receive();
        		    ext_spi_init_complete = true;
        		    Reset_SPI_And_PrepareToReceive();
        		    Serial6.ResetUart();
         	        Serial6.begin(57600);
                	curr_output = 0;
        		}
        		curr_opmode = new_opmode_request_value;
        	}
        	new_opmode_request = false;
        }

        uint8_t curr_input = 0;

        // Update outputs, if in OUTPUT mode
        if (new_output_request) {
        	if (curr_opmode == TEST_JIG_OPMODE_OUTPUT) {
        		HAL_CommDIOPins_Set(new_output_request_value);
        		curr_output = new_output_request_value;
        	}
        	new_output_request = false;
        }

        // Read inputs, if in INPUT mode
        if (curr_opmode == TEST_JIG_OPMODE_INPUT) {
        	// update curr_input with current values
        	HAL_CommDIOPins_Get(&curr_input);
        }

        // Send CAN update
        memset(&test_jig_update_msg, 0, sizeof(CAN_TRANSFER));
        CAN_pack_extended_id(CAN_VMX_PI_TEST_JIG_UPDATE_MSG, &test_jig_update_msg.id);
        test_jig_update_msg.payload.dlc.len = 3;
        test_jig_update_msg.payload.buff[0] = curr_opmode;
        test_jig_update_msg.payload.buff[1] = curr_input;
        test_jig_update_msg.payload.buff[2] = curr_output;
        CAN_add_tx_transfer(&test_jig_update_msg);
        CAN_loop(); // Trigger transmission to CAN bus of any queued messages.

        // Update registers with latest state
        bool register_update = false;

        if (registers.commdio_op_mode != (uint8_t)curr_opmode) {
        	registers.commdio_op_mode = (uint8_t)curr_opmode;
        	register_update = true;
        }
        if (registers.commdio_input_values != curr_input) {
        	registers.commdio_input_values = curr_input;
        	register_update = true;
        }
        if (registers.commdio_output_values != curr_output) {
        	registers.commdio_output_values = curr_output;
        	register_update = true;
        }

        /* Update shadow registers; disable i2c/spi interrupts around this access. */
        if ( register_update ) {
            NVIC_DisableIRQ((IRQn_Type)I2C3_EV_IRQn);
            NVIC_DisableIRQ((IRQn_Type)SPI1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)DMA2_Stream0_IRQn);
            // TODO:  Should UART interrupt be disabled as well?
            memcpy(&shadow_registers, &registers, sizeof(registers));
            NVIC_EnableIRQ((IRQn_Type)DMA2_Stream0_IRQn);
            NVIC_EnableIRQ((IRQn_Type)SPI1_IRQn);
            if ( prepare_i2c_receive_timeout_count > 0 ) {
                if ( (HAL_GetTick() - prepare_i2c_receive_last_attempt_timestamp) > (unsigned long)5000) {
                    if ( HAL_OK == prepare_next_i2c_receive() ) {
                        prepare_i2c_receive_timeout_count = 0;
                    }
                }
            }
            NVIC_EnableIRQ((IRQn_Type)I2C3_EV_IRQn);
           	HAL_AHRS_Int_Assert();
        }

		if (curr_opmode == TEST_JIG_OPMODE_INPUT) {
		    HAL_LED1_On(0);
		    HAL_LED2_On(1);
		} else if (curr_opmode == TEST_JIG_OPMODE_OUTPUT) {
		    HAL_LED1_On(1);
		    HAL_LED2_On(0);
		} else if (curr_opmode == TEST_JIG_OPMODE_ECHO) {
		    HAL_LED1_On(1);
		    HAL_LED2_On(1);
		}

        last_scan = HAL_GetTick();

		/* Send Streaming Updates */

		for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {

			if ( update_type[ifx] == MSGID_TEST_JIG_UPDATE ) {

				num_update_bytes[ifx] = VMXPiTestJigProtocol::encodeTestJigUpdate( update_buffer[ifx],
						curr_opmode,
						curr_input);
			}
            if ( num_update_bytes[ifx] > 0 ) {
                serial_interfaces[ifx]->write(update_buffer[ifx],num_update_bytes[ifx]);
            }
		}

        /* Perform External I2C Interace Glitch Detection/Correction */
        if ( i2c_rx_in_progress ) {
            if ( HAL_GetTick() - last_i2c_rx_start_timestamp > (uint32_t)40 ) {
                if ( ( hi2c3.XferCount != 0 ) &&
                        ( hi2c3.XferSize > hi2c3.XferCount ) ) {
                    if ( ( hi2c3.Instance->CR2 & (I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR) ) == 0 ) {
                        i2c_glitch_count++;
                        __HAL_I2C_ENABLE_IT(&hi2c3, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);
                    }
                }
            }
        }

        const uint8_t num_ms_to_delay = 20;
        if (curr_opmode == TEST_JIG_OPMODE_ECHO) {
			for (uint8_t i = 0; i < num_ms_to_delay; i++) {
				// echo bytes received on UART back to sender
				while (Serial6.available() > 0) {
					uint8_t rcv_buffer[1024];
				    uint32_t num_bytes_read = Serial6.read(rcv_buffer, 512);
				    if (num_bytes_read > 0) {
				    	Serial6.write(rcv_buffer, num_bytes_read);
				    }
				}
				HAL_Delay(1);
			}

        } else {
        	HAL_Delay(num_ms_to_delay);
        }
    }
}

__STATIC_INLINE uint8_t *GetRegisterAddressAndMaximumSize( uint8_t register_address, uint16_t& size )
{
    if ( register_address > NAVX_REG_LAST ) {
        size = 0;
        return 0;
    }
    uint8_t *register_base = (uint8_t *)&shadow_registers;
    size = (NAVX_REG_LAST + 1) - register_address;
    return register_base + register_address;
}

uint8_t i2c_tx_buffer[NAVX_REG_LAST + 1];
volatile unsigned long last_i2c_tx_start_timestamp = 0;
volatile bool i2c_tx_in_progress = false;
#define I2C_TX_TIMEOUT_MS ((unsigned long)100)
#define I2C_PREPARE_RECEIVE_TIMEOUT_MS  ((unsigned long)100)
#define I2C_PREPARE_TRANSMIT_TIMEOUT_MS  ((unsigned long)100)
int i2c_error_count = 0;
HAL_I2C_ErrorTypeDef last_i2c_err_code = HAL_I2C_ERROR_NONE;

void i2c_hard_reset()
{
    HAL_I2C_DeInit(&hi2c3);
    /* Perform software reset */
    hi2c3.Instance->CR1 |= 0x8000;
    /* Clear all registers */
    hi2c3.Instance->CR1 = 0;
    hi2c3.Instance->CR2 = 0;
    hi2c3.Instance->CCR = 0;
    hi2c3.Instance->TRISE = 0;
    hi2c3.Instance->SR1 = 0;
    hi2c3.Instance->SR2 = 0;
    hi2c3.Instance->OAR1 = 0;
    hi2c3.Instance->OAR2 = 0;
    /* Release software reset */
    hi2c3.Instance->CR1 &= ~0x8000;
    HAL_I2C_Init(&hi2c3);
}

/* Prepare for next reception on the I2C3 (external control) port */
/* This function includes a retry mechanism, and also will reset  */
/* the port in case of a timeout.                                 */

HAL_StatusTypeDef prepare_next_i2c_receive()
{
    HAL_StatusTypeDef status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
    if ( status == HAL_BUSY ) {
        unsigned long prepare_i2c_receive_last_attempt_timestamp = HAL_GetTick();
        while ( status == HAL_BUSY )
        {
            status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
            if ( (HAL_GetTick() - prepare_i2c_receive_last_attempt_timestamp) > I2C_PREPARE_RECEIVE_TIMEOUT_MS) {
                HAL_I2C_DeInit(&hi2c3);
                HAL_I2C_Init(&hi2c3);
                status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
                if ( status != HAL_OK ) {
                    i2c_hard_reset();
                    status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
                    if ( status != HAL_OK ) {
                        prepare_i2c_receive_timeout_count++;
                    }
                }
                break;
            }
        }
    }
    if ( status == HAL_OK ) {
        i2c_rx_in_progress = true;
        last_i2c_rx_start_timestamp = HAL_GetTick();
    }
    return status;
}

void process_writable_register_update( uint8_t requested_address, uint8_t *reg_addr, uint8_t value )
{
    if ( requested_address == VMX_PI_TEST_JIG_COMMDIO_OPMODE ) {
        char requested_opmode = (char)value;
        if ((requested_opmode == TEST_JIG_OPMODE_INPUT) ||
        	(requested_opmode == TEST_JIG_OPMODE_OUTPUT) ||
        	(requested_opmode == TEST_JIG_OPMODE_ECHO)) {
        	*reg_addr = value;
        	new_opmode_request = true;
        	new_opmode_request_value = requested_opmode;
        }
    } else if ( requested_address == VMX_PI_TEST_JIG_COMMDIO_OUTPUT) {
    	*reg_addr = value;
        new_output_request = true;
        new_output_request_value = value;
    }
}

int unexpected_receive_size_count = 0;
int i2c_short_xfer_request_count = 0;
int i2c_long_xfer_request_count = 0;
int i2c_too_long_xfer_request_count = 0;
int i2c_tx_complete_count = 0;
int i2c_last_requested_read_address = 0;

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    HAL_StatusTypeDef status;
    uint8_t *reg_addr;
    uint16_t max_size;
    uint16_t num_bytes_received = I2cHandle->XferSize - I2cHandle->XferCount;
    uint8_t *rx_bytes_start = I2cHandle->pBuffPtr - num_bytes_received;
    bool i2c_transmitting = false;
    bool write = false;
    i2c_rx_in_progress = false;
    if ( I2cHandle->Instance == I2C3 ) {
        if ( I2cHandle->Instance->SR2 & I2C_SR2_TRA ) {
        } else {
            /* Master Receive bit set:  request from master received. */
            bool valid_request;
            uint8_t requested_address;
            uint8_t num_bytes;
            if ( num_bytes_received == 1 ) {
                /* If 1 byte is received, the byte specifies the beginning address    */
                /* Setup for a burst read of the maximum possible length.  If the     */
                /* Master desires less, they will send a NAK (see I2C error handler). */
                requested_address = rx_bytes_start[0];
                i2c_last_requested_read_address = requested_address;
                num_bytes = (NAVX_REG_LAST + 1) - requested_address;
                valid_request = true;
                i2c_short_xfer_request_count++;
            }
            else if ( num_bytes_received == 2 ) {
                /* If two bytes are received, the first indicates the beginning address */
                /* Setup for a burst read with a length specifed in the second byte.    */
                requested_address = rx_bytes_start[0];
                i2c_last_requested_read_address = requested_address;
                num_bytes = rx_bytes_start[1];
                valid_request = true;
                i2c_long_xfer_request_count++;
            } else {
                /* Invalid request. */
                valid_request = false;
                i2c_too_long_xfer_request_count++;
            }
            if ( valid_request ) {
                if ( requested_address & 0x80 ) {
                    write = true;
                    requested_address &= ~0x80;
                }
                reg_addr = GetRegisterAddressAndMaximumSize(requested_address, max_size);
                if ( reg_addr ) {
                    if ( write ) {
                        process_writable_register_update( requested_address, reg_addr, num_bytes /* value */ );
                    } else {
                        uint8_t i2c_bytes_to_be_transmitted = (num_bytes > max_size) ? max_size : num_bytes;
                        memcpy(i2c_tx_buffer,reg_addr,i2c_bytes_to_be_transmitted);
                        status = HAL_BUSY;
                        unsigned long prepare_i2c_tx_last_attempt_timestamp = 0;
                        while ( status == HAL_BUSY )
                        {
                            status = HAL_I2C_Slave_Transmit_DMA(&hi2c3, i2c_tx_buffer, i2c_bytes_to_be_transmitted);
                            if ( status == HAL_OK ) {
                                i2c_transmitting = true;
                                i2c_tx_in_progress = true;
                                last_i2c_tx_start_timestamp = HAL_GetTick();
                                break;
                            } else {
                                /* I2C Transmit not ready to request start.     */
                                /* If timeout waiting occurs, the I2C interface */
                                /* likely needs to be reset.                    */
                                if ( prepare_i2c_tx_last_attempt_timestamp == 0 ) {
                                    prepare_i2c_tx_last_attempt_timestamp = HAL_GetTick();
                                } else if ( HAL_GetTick() - prepare_i2c_tx_last_attempt_timestamp >
                                            I2C_PREPARE_TRANSMIT_TIMEOUT_MS) {
                                    /* Transmit aborted, client will need to */
                                    /* timeout.                              */
                                    i2c_hard_reset();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else {
                unexpected_receive_size_count++;
            }
        }
        if ( !i2c_transmitting ) {
            i2c_tx_in_progress = false;
            prepare_next_i2c_receive();
        }
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    HAL_StatusTypeDef status;
    if ( I2cHandle->Instance == I2C3 )
    {
        i2c_tx_in_progress = false;
        status = prepare_next_i2c_receive();
    }
}

int i2c_error_af_ignored_count = 0;
int i2c_error_af_count = 0;
int i2c_error_timeout_count = 0;
int i2c_error_dma_count = 0;
int i2c_error_arlo_count = 0;
int i2c_error_berr_count = 0;
int i2c_error_ovr_count = 0;
int i2c_error_other_count = 0;
int i2c_error_while_rx_in_progress_count = 0;
int i2c_error_while_tx_in_progress_count = 0;
int i2c_num_bytes_transferred_before_last_nack = 0;

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    /* The "AF" (Ack fail) error will occur in the case of a burst read */
    /* if the receiver terminates the read before all bytes are         */
    /* transmitted.  In this case, simply clear the transmit buffer     */
    /* pointer; else, reset the I2C interface.                          */
    if ( I2cHandle->Instance == I2C3 )
    {
        last_i2c_err_code = I2cHandle->ErrorCode;
        if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_AF ) {
            i2c_num_bytes_transferred_before_last_nack = I2cHandle->XferSize - I2cHandle->XferCount;
            I2cHandle->XferCount = 0;
            I2cHandle->XferSize = 0;
            I2cHandle->pBuffPtr = 0;
            i2c_error_af_count++;
        } else {
            if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_TIMEOUT ) {
                i2c_error_timeout_count++;
            }
            else if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_DMA ) {
                i2c_error_dma_count++;
            }
            else if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_ARLO ) {
                i2c_error_arlo_count++;
            }
            else if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_BERR ) {
                i2c_error_berr_count++;
            }
            else if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_OVR ) {
                i2c_error_ovr_count++;
            }
            HAL_I2C_DeInit(I2cHandle);
            HAL_I2C_Init(I2cHandle);
            i2c_error_other_count++;
        }

        if ( i2c_rx_in_progress ) {
            i2c_rx_in_progress = false;
            i2c_error_while_rx_in_progress_count++;
        }
        if ( i2c_tx_in_progress ) {
            i2c_tx_in_progress = false;
            i2c_error_while_tx_in_progress_count++;
        }
        prepare_next_i2c_receive();
        i2c_error_count++;
    }
}

typedef enum  {
    NONE,
    WRITE_REQ,    /* SPI Master sent write request to SPI Slave */
    READ_REQ,     /* SPI Master sent read request to SPI Slave. */
    READ_RESP,    /* SPI Slave is sending reponse to read request from SPI Master. */
    SLAVE_RX_ERR, /* SPI Slave received an invalid request, which was discarded, or rx overflow occurred. */
    SLAVE_TX_ERR  /* Error occurred during SPI Slave transmit [NOTE:  not an error if BUSY error occurs. */
} SPI_MSG_TYPE;

SPI_MSG_TYPE spi_last_valid_msg_type = NONE;    // Previous valid spi transaction type
SPI_MSG_TYPE spi_last_err_msg_type = NONE;      // Previous invalid spi transaction type
uint8_t spi_last_read_req_len = 0;              // # Bytes to be transmitted in curr/previous SPI Slave Transmit
uint8_t spi_last_read_req_reg_addr = 0;         // Starting register address of last READ_REQ
uint8_t spi_last_read_req_bank = 0;             // Register Bank # of last READ_REQ
uint8_t spi_last_write_req_len = 0;             // # Bytes last written in curr/previous WRITE_REQ
uint8_t spi_last_write_req_reg_addr = 0;        // Starting register address of last WRITE_REQ
uint8_t spi_last_write_req_bank = 0;            // Register Bank # of last WRITE_REQ
uint32_t spi_SR_at_beginning_of_last_write = 0;
unsigned long last_spi_tx_start_timestamp = 0;  // Timestamp when last SPI Slave Transmit was initiated
#define SPI_TX_TIMEOUT_MS ((unsigned long)10)   // Max allowed duration of in-progress SPI Slave Transmit

/* SPI Error Counts */
int wrong_size_spi_receive_count = 0;           // SPI Slave Receive w/invalid byte count
int invalid_char_spi_receive_count = 0;         // SPI Slave Receive w/CRC err, bad address or count
int invalid_reg_address_spi_count = 0;          // SPI Slave Receive contained Invalid Register Address
int spi_error_count = 0;                        // SPI Slave Error Callback count
int spi_ovr_error_count = 0;                    // SPI Slave Receive Overflow count
int spi_txne_error_count = 0;                   // SPI Slave Transmit (TXNE) Underflow count [e.g., DMA Underrun;
                                                // (this can also occur during cleanup of a SPI Transmit Timeout)
int spi_rxne_error_count = 0;                   // During SPI Slave Receive, data lost because RX register was full
int spi_busy_error_count = 0;                   // SPI Busy after SPI Slave Transmit completed [caused by Silicon bug]
int spi_rx_while_tx_error_count = 0;            // SPI Data Receive Callback invoked during SPI Slave Transmit ['impossible' condition]
int spi_tx_complete_timeout_count = 0;          // SPI Slave Transmit completion timeout [protocol error]
uint32_t last_spi_tx_complete_timeout_xfer_size = 0;
uint32_t last_spi_tx_complete_timeout_xfer_count = 0;
uint32_t last_spi_tx_complete_timeout_error_code = 0;
uint32_t last_spi_tx_complete_timeout_SR = 0;

void Reset_SPI_And_PrepareToReceive() {
    HAL_SPI_Comm_Ready_Deassert();
    HAL_SPI_DeInit(&hspi1);
    HAL_SPI_Init(&hspi1);
    spi_transmitting = false;
    __HAL_SPI_CLEAR_OVRFLAG(&hspi1);
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
    HAL_SPI_Comm_Ready_Assert();
}

// This function should be polled periodically (at least once every
// SPI_TX_TIMEOUT_MS period).
//
// If the SPI transmitter has been waiting more than the timeout period
// to complete the transmission, disable the transmitter and prepare
// again to receive data.
void SpiTxTimeoutCheck() {
    if (spi_transmitting) {
    	unsigned long last_spi_txt_start = last_spi_tx_start_timestamp;
        unsigned long now = HAL_GetTick();
        if (now > last_spi_txt_start) {
        	unsigned long transmit_period = now - last_spi_txt_start;
        	if (transmit_period > SPI_TX_TIMEOUT_MS) {
        		last_spi_tx_complete_timeout_xfer_size = hspi1.TxXferSize;
        		last_spi_tx_complete_timeout_xfer_count = hspi1.TxXferCount;
        		last_spi_tx_complete_timeout_error_code = hspi1.ErrorCode;
        		last_spi_tx_complete_timeout_SR = hspi1.Instance->SR;
        		Reset_SPI_And_PrepareToReceive();
        		spi_tx_complete_timeout_count++;
        	}
        }
    }
}

#ifdef ENABLE_BANKED_REGISTERS
int spi_rx_variable_message_len = 0;
uint8_t dummy;
#endif

uint8_t spi_tx_buffer[255];

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (!ext_spi_init_complete) return;

    uint8_t *reg_addr = 0;
    uint16_t max_size;
    uint8_t reg_address;
    uint8_t reg_count;
    uint8_t received_crc;
    uint8_t calculated_crc;
    uint8_t bank;
    bool write = false;
    if ( hspi->Instance == SPI1 ) {

        if ( !spi_transmitting ) {
            int num_bytes_received = hspi->RxXferSize - hspi->RxXferCount;
#ifdef ENABLE_BANKED_REGISTERS
            uint8_t *received_data;
            if ((num_bytes_received == SPI_RECV_LENGTH ) ||
            	((spi_rx_variable_message_len > 0) && (num_bytes_received == spi_rx_variable_message_len))) {
            	bank = spi1_RxBuffer[0];
            	reg_address = spi1_RxBuffer[1];
            	reg_count = spi1_RxBuffer[2]; /* Bank 0: This is the byte to write; Bank > 0:  count of bytes to be written */
            	received_data = &spi1_RxBuffer[3];
            	if(spi_rx_variable_message_len > 0) {
            		spi_rx_variable_message_len = 0;
            	}
#else
            if ( num_bytes_received == SPI_RECV_LENGTH ) {
            	bank = 0;
                reg_address = spi1_RxBuffer[0];
                reg_count = spi1_RxBuffer[1];
#endif
                received_crc = spi1_RxBuffer[num_bytes_received-1];
                calculated_crc = VMXPiTestJigRegisters::getCRCWithTable(crc_lookup_table,spi1_RxBuffer,num_bytes_received-1);
                if ( ( reg_address != 0xFF ) && (reg_count > 0 ) && ( received_crc == calculated_crc ) ) {
                    if ( reg_address & 0x80 ) {
                        write = true;
                        reg_address &= ~0x80;
                    }
#ifdef ENABLE_BANKED_REGISTERS
                    if (bank < NUM_SPI_BANKS) {
                    	if (bank == 0) {
                           	reg_addr = GetRegisterAddressAndMaximumSize(reg_address, max_size);
                    	} else if (p_reg_lookup_func[bank] != NULL) {
                    		reg_addr = p_reg_lookup_func[bank](bank, reg_address, reg_count, &max_size);
                    	}
                    } else if (bank == COMM_MODE_BANK) {
                    	reg_addr = &dummy; /* Special mode does not access reg_addr */
                    } else if (bank >= NUM_SPI_BANKS) {
                    	reg_addr = 0; /* Invalid Bank Requested */
                    }
                    else
#endif
                    {
                    	reg_addr = GetRegisterAddressAndMaximumSize(reg_address, max_size);
                    }
                    if ( reg_addr ) {
                    	if ( write ) {
                    	    // Write Request
                    		HAL_SPI_Comm_Ready_Deassert();
                    		uint16_t next_rcv_size = SPI_RECV_LENGTH;
#ifdef ENABLE_BANKED_REGISTERS
                    		if (bank <= NUM_SPI_BANKS) {
                    			if (bank == 0) {
                               		process_writable_register_update( reg_address, reg_addr, reg_count /* value */ );
                    			} else if(p_reg_write_func[bank] != NULL) {
                    				p_reg_write_func[bank](bank, reg_address, reg_addr, reg_count, received_data);
                    			}
                    		} else if (bank == COMM_MODE_BANK) {
                            	if (reg_address == COMM_MODE_REG_VARIABLEWRITE ) {
                            		/* Next transaction is variable len write; len is in defined by count */
                            		next_rcv_size = reg_count;
                            		spi_rx_variable_message_len = next_rcv_size;
                            	}
                            }
#else
                    		process_writable_register_update( reg_address, reg_addr, reg_count /* value */ );
#endif

							spi_last_valid_msg_type = WRITE_REQ;
                            spi_last_write_req_len = (bank == 0) ? 1 : reg_count;
                            spi_last_write_req_reg_addr = reg_address;
                            spi_last_write_req_bank = bank;
                            __HAL_SPI_CLEAR_OVRFLAG(hspi);
                    		HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, next_rcv_size);
                            HAL_SPI_Comm_Ready_Assert();
                        } else {
                            // Read Request
                            if (bank == 0) {
                                HAL_AHRS_Int_Deassert();
                            }
                        	spi_transmitting = true;
                            spi_last_read_req_len = (reg_count > max_size) ? max_size : reg_count;
                            spi_last_read_req_reg_addr = reg_address;
                            spi_last_read_req_bank = bank;
                            memcpy(spi_tx_buffer,reg_addr,spi_last_read_req_len);
                            spi_tx_buffer[spi_last_read_req_len] =
                                    VMXPiTestJigRegisters::getCRCWithTable(crc_lookup_table, spi_tx_buffer,
                                                                  (uint8_t)spi_last_read_req_len);
                            spi_last_valid_msg_type = READ_REQ;
                            spi_SR_at_beginning_of_last_write = hspi->Instance->SR;
                            __HAL_SPI_CLEAR_OVRFLAG(hspi);
                            __HAL_SPI_DISABLE(hspi);
                            HAL_SPI_Transmit_DMA(&hspi1, spi_tx_buffer, spi_last_read_req_len+1);
                            last_spi_tx_start_timestamp = HAL_GetTick();
                        	HAL_SPI_Comm_Ready_Deassert();
                        }
                    } else {
                        invalid_reg_address_spi_count++;
                        spi_last_err_msg_type = SLAVE_RX_ERR;
                        __HAL_SPI_CLEAR_OVRFLAG(hspi);
                        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                        HAL_SPI_Comm_Ready_Assert();
                    }
                } else {
                    invalid_char_spi_receive_count++;
                    spi_last_err_msg_type = SLAVE_RX_ERR;
                    /* If repeated invalid requests are received, and if the */
                    /* SPI interface is still busy, reset the SPI interface. */
                    /* This condition can occurs sometimes after the         */
                    /* host computer is power-cycled.                        */
#if 0
                    if ( ( (invalid_char_spi_receive_count % 5) == 0 ) &&
                         ( __HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_BSY) != RESET ) ) {
                        Reset_SPI_And_PrepareToReceive();
                   } else {
                       Reset_SPI_And_PrepareToReceive();
                       HAL_SPI_Comm_Ready_Assert(); /* This shouldn't be necessary - remove it! */
                   }
#endif
                    if ( ( __HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_BSY) != RESET ) ) {
                        Reset_SPI_And_PrepareToReceive();
                   } else {
#ifdef ENABLE_BANKED_REGISTERS
						if(spi_rx_variable_message_len > 0) {
							spi_rx_variable_message_len = 0;
						}
#endif
						__HAL_SPI_CLEAR_OVRFLAG(hspi);
                       HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                	   HAL_SPI_Comm_Ready_Assert();
                   }
                }
            } else {
                spi_last_err_msg_type = SLAVE_RX_ERR;
                wrong_size_spi_receive_count++;
#ifdef ENABLE_BANKED_REGISTERS
            	if(spi_rx_variable_message_len > 0) {
            		spi_rx_variable_message_len = 0;
            	}
#endif
                __HAL_SPI_CLEAR_OVRFLAG(hspi);
                HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                HAL_SPI_Comm_Ready_Assert();
            }
        } else {
            // A SPI receive event occurred while transmitting.  This should never happen!
            spi_rx_while_tx_error_count++;
        }
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if ( hspi->Instance == SPI1 ) {
        /* HAL_SPI_ERROR_OVR: Master has transmitted bytes, the slave hasn't read the */
        /* previous byte in the receive register.  This case occurs when the          */
        /* Receive ISR hasn't been responded to before the master starts writing.     */

        /* Clear the error, and prepare for next receive.                             */
        spi_error_count++;
        if ((hspi->ErrorCode & HAL_SPI_ERROR_OVR) != 0) {
            __HAL_SPI_CLEAR_OVRFLAG(hspi);
            spi_last_err_msg_type = spi_transmitting ? SLAVE_TX_ERR : SLAVE_RX_ERR;
            spi_ovr_error_count++;
        }

        if ((hspi->ErrorCode == HAL_SPI_ERROR_FLAG) != 0) {
        	/* Either RXNE,TXE, BSY flags are set.  It's also possible the OVR flag is set. */
        	uint32_t status_bits = hspi->Instance->SR;
        	if (status_bits & 0x00000001) {
        	    spi_last_err_msg_type = spi_transmitting ? SLAVE_TX_ERR : SLAVE_RX_ERR;
        	    spi_rxne_error_count++;
        	}
        	if (spi_transmitting) {
        	    // Transmitter NE is only an error during SPI Slave Transmit
        	    if ((status_bits & 0x00000002) == 0) {
        	        spi_last_err_msg_type = SLAVE_TX_ERR;
        	        spi_txne_error_count++;
        	    }
        	}
        	if (status_bits & 0x00000040) {
                __HAL_SPI_CLEAR_OVRFLAG(hspi);
                spi_last_err_msg_type = spi_transmitting ? SLAVE_TX_ERR : SLAVE_RX_ERR;
                spi_ovr_error_count++;
        	}
        	if (status_bits & 0x00000080) {
        	    // NOTE:  This occurs often due to a silicon bug
        	    // Therefore, this is not considered a SLAVE_TX_ERR.
        		spi_busy_error_count++;
                /* Disable the SPI, which clears the busy condition   */
        		/* (SPI will be reenabled upon the next invocation of */
        		/* HAL_SPI_Receive_DMA or HAL_SPI_Transmit_DMA)       */
                __HAL_SPI_DISABLE(hspi);
        	}
        }
        spi_transmitting = false;
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
        HAL_SPI_Comm_Ready_Assert();
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if ( hspi->Instance == SPI1 ) {
        spi_transmitting = false;
        __HAL_SPI_CLEAR_OVRFLAG(hspi);
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
        HAL_SPI_Comm_Ready_Assert();
    }
}
