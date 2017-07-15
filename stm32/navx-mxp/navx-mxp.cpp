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
#include "mpucontroller.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
}

#include "navx-mxp.h"
#include "version.h"
#include "revision.h"
#include <math.h>
#include "AHRSProtocol.h"
#include "IMURegisters.h"
#include "usb_serial.h"
#include "HardwareSerial.h"
#include "FlashStorage.h"
#include "ext_interrupts.h"

extern I2C_HandleTypeDef hi2c3; /* External I2C Interface */
extern I2C_HandleTypeDef hi2c2; /* Internal I2C Interface (to MPU, etc.) */
extern SPI_HandleTypeDef hspi1; /* External SPI Interface */

TIM_HandleTypeDef TimHandle; /* Internal Timer */
FlashStorageClass FlashStorage; /* Class managing persistent configuration data */

int prepare_i2c_receive_timeout_count = 0;
unsigned long prepare_i2c_receive_last_attempt_timestamp = 0;
bool i2c_rx_in_progress = false;
uint32_t last_i2c_rx_start_timestamp = 0;
int i2c_glitch_count = 0;
HAL_StatusTypeDef prepare_next_i2c_receive();

#define RXBUFFERSIZE 64
uint8_t i2c3_RxBuffer[RXBUFFERSIZE];
uint8_t spi1_RxBuffer[RXBUFFERSIZE];

#define NUM_SPI_BANKS 4

loop_func p_loop_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL };
register_lookup_func p_reg_lookup_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL };
register_write_func p_reg_write_func[NUM_SPI_BANKS] = { NULL, NULL, NULL, NULL };

_EXTERN_ATTRIB void nav10_set_loop(uint8_t bank, loop_func p) { p_loop_func[bank] = p; }
_EXTERN_ATTRIB void nav10_set_register_lookup_func(uint8_t bank, register_lookup_func p) { p_reg_lookup_func[bank] = p; }
_EXTERN_ATTRIB void nav10_set_register_write_func(uint8_t bank, register_write_func p) { p_reg_write_func[bank] = p; }

#define MIN_SAMPLE_RATE_HZ 4
#define MAX_SAMPLE_RATE_HZ 200

#define UART_RX_PACKET_TIMEOUT_MS 	30 /* Max wait after start of packet */
#define MIN_UART_MESSAGE_LENGTH		STREAM_CMD_MESSAGE_LENGTH

#ifdef ENABLE_BANKED_REGISTERS
#include "SPICommCtrl.h"
#define		SPI_RECV_LENGTH STD_SPI_MSG_LEN
			/* SPI Requests:  Write:  [Bank] [0x80 | RegAddr] [Count (1-4)] [4 bytes of write data] [CRC] */
			/*                *If bank = 0, 1-byte write is used, and count is the byte to be written.    */
			/*                Read:   [Bank] [RegAddr] [Count] [CRC] [4 bytes are ignored]                */
#define		MAX_VALID_BANK  NUM_SPI_BANKS
#else
#define		SPI_RECV_LENGTH 3	/* SPI Requests:  [0x80 | RegAddr] [Count (Readd) or Data (Write)] [CRC] (Bank 0 is implicitly used) */
#define     MAX_VALID_BANK  0
#endif

uint8_t clip_sample_rate(uint8_t update_rate_hz)
{
    if ( update_rate_hz < MIN_SAMPLE_RATE_HZ ) {
        update_rate_hz = MIN_SAMPLE_RATE_HZ;
    }
    if ( update_rate_hz > MAX_SAMPLE_RATE_HZ ) {
        update_rate_hz = MAX_SAMPLE_RATE_HZ;
    }
    return update_rate_hz;
}

#define MAX_SAMPLE_PERIOD (1000 / MIN_SAMPLE_RATE_HZ)
#define MIN_SAMPLE_PERIOD (1000 / MAX_SAMPLE_RATE_HZ)

#define NUM_STREAMING_INTERFACES 2

char update_type[NUM_STREAMING_INTERFACES] = { MSGID_YPR_UPDATE, MSGID_YPR_UPDATE };
char update_buffer[NUM_STREAMING_INTERFACES][AHRS_PROTOCOL_MAX_MESSAGE_LENGTH * 3];	/* Buffer for outbound serial update messages. */
char inbound_data[AHRS_PROTOCOL_MAX_MESSAGE_LENGTH];			/* Buffer for inbound serial messages.  */
char response_buffer[NUM_STREAMING_INTERFACES][AHRS_PROTOCOL_MAX_MESSAGE_LENGTH * 2];  /* Buffer for building serial response. */

struct fusion_settings {
    float motion_detect_thresh_g;
    float yaw_change_detect_thresh_deg;
    float sealevel_pressure_millibars;
};

struct board_orientation_settings {
    uint8_t yaw_axis;
    bool yaw_axis_up;
};

/* nav10_cal_data is persisted to flash */
struct flash_cal_data {
    uint8_t                                 selfteststatus;
    struct mpu_selftest_calibration_data    mpucaldata;
    struct mpu_dmp_calibration_data         dmpcaldata;
    bool                                    dmpcaldatavalid;
    struct mag_calibration_data             magcaldata;
    struct fusion_settings                  fusiondata;
    struct board_orientation_settings       orientationdata;
};

const float default_motion_detect_thresh_g = 0.02f;
const float default_yaw_change_detect_thresh_deg = 0.05f;
const float default_sealevel_pressure_millibars = 1013.25f;

void init_fusion_settings(struct fusion_settings *settings) {
    settings->motion_detect_thresh_g = default_motion_detect_thresh_g;
    settings->yaw_change_detect_thresh_deg = default_yaw_change_detect_thresh_deg;
    settings->sealevel_pressure_millibars = default_sealevel_pressure_millibars;
}

struct mpu_dmp_calibration_interpolation_coefficients {
    float gyro_m[3];
    float gyro_b[3];
};

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
    /* Read/Write registers */
    uint8_t                 update_rate_hz;
    /* Read-only Registers */
    uint8_t                 accel_fsr_g;
    uint16_t                gyro_fsr_dps;
    uint8_t                 op_status;
    uint8_t                 cal_status;
    uint8_t                 selftest_status;
    uint16_t                capability_flags;
    /* Reserved */
    uint8_t                 reserved[3];
    /* Processed Data */
    uint16_t                sensor_status;
    uint32_t                timestamp;
    s_short_hundred_float   yaw;
    s_short_hundred_float   roll;
    s_short_hundred_float   pitch;
    u_short_hundred_float   heading;
    u_short_hundred_float   fused_heading;
    s_1616_float            altitude;
    s_short_thousand_float  world_linear_accel[3];
    s_short_ratio_float     quat[4];
    /* Raw Data */
    s_short_hundred_float   mpu_temp_c;
    int16_t                 gyro_raw[3];
    int16_t                 accel_raw[3];
    int16_t                 mag_raw[3];
    s_1616_float            barometric_pressure;
    s_short_hundred_float   pressure_sensor_temp_c;
    s_short_hundred_float   yaw_offset;
    s_short_ratio_float     quat_offset[4];
    /* Integrated Values */
    /* - Read/Write Registers */
uint16_t                    integration_control;
    /* - Read-only Registers */
    s_1616_float            velocity[3];
    s_1616_float            displacement[3];
} registers, shadow_registers;

/* Statistical Averages */

#define YAW_HISTORY_PERIOD_MS                       2000
#define WORLD_ACCEL_NORM_HISTORY_PERIOD_MS          250
#define WORLD_ACCEL_NORM_HISTORY_SIZE               WORLD_ACCEL_NORM_HISTORY_PERIOD_MS / MIN_SAMPLE_PERIOD
#define YAW_HISTORY_SIZE                            YAW_HISTORY_PERIOD_MS / MIN_SAMPLE_PERIOD

float world_accel_norm_history[WORLD_ACCEL_NORM_HISTORY_SIZE];
float yaw_history[YAW_HISTORY_SIZE];
int world_acceleration_history_index = 0;
int yaw_history_index = 0;
float world_acceleration_norm_current_avg = 0.0f;
float yaw_current_average = 0.0f;

#define DEFAULT_MPU_SAMPLE_PERIOD_MS                (1000 / DEFAULT_MPU_HZ)

int current_yaw_history_size = YAW_HISTORY_PERIOD_MS
        / DEFAULT_MPU_SAMPLE_PERIOD_MS;
int current_world_accel_norm_history_size = WORLD_ACCEL_NORM_HISTORY_PERIOD_MS
        / DEFAULT_MPU_SAMPLE_PERIOD_MS;

void calculate_current_history_sizes()
{
    int sample_period_ms = 1000 / registers.update_rate_hz;
    current_yaw_history_size = YAW_HISTORY_PERIOD_MS / sample_period_ms;
    current_world_accel_norm_history_size = WORLD_ACCEL_NORM_HISTORY_PERIOD_MS / sample_period_ms;
}

/******************* TUNING VARIBLES ********************/

void init_history()
{
    for (int i = 0; i < WORLD_ACCEL_NORM_HISTORY_SIZE; i++) {
        world_accel_norm_history[i] = world_acceleration_norm_current_avg;
    }
    for (int i = 0; i < YAW_HISTORY_SIZE; i++) {
        yaw_history[i] = yaw_current_average;
    }
    calculate_current_history_sizes();
}

uint8_t *flashdata = 0;

static const uint8_t flash_fast = 0b10101010; /* Self-test Fail */
static const uint8_t flash_slow = 0b11111110; /* IMU Cal */
static const uint8_t flash_on   = 0b11111111; /* Startup, Mag Cal */
static const uint8_t flash_off  = 0b00000000;

volatile int8_t cal_led_cycle_index = 7;
volatile uint8_t cal_led_cycle = flash_on;
float last_gyro_bias_load_temperature = 0.0;

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

void calc_linear_temp_correction_coefficients( struct mpu_dmp_calibration_data *first,
        struct mpu_dmp_calibration_data *second,
        struct mpu_dmp_calibration_interpolation_coefficients *result )
{
    float delta_temp = fabs(first->mpu_temp_c - second->mpu_temp_c);
    float offset_first, offset_second, delta_offset;
    for ( int i = 0; i < 3; i++ ) {
        offset_first = ((float)first->gyro_bias_q16[i]) / 65536.0f;
        offset_second = ((float)second->gyro_bias_q16[i]) / 65536.0f;
        delta_offset = fabs(offset_second - offset_first);
        result->gyro_m[i] = delta_offset / delta_temp;
        result->gyro_b[i] = offset_first - (first->mpu_temp_c * result->gyro_m[i]);
    }
}

void interpolate_temp_correction_offsets( struct mpu_dmp_calibration_interpolation_coefficients *input,
        float temp_c,
        struct mpu_dmp_calibration_data *output )
{
    output->mpu_temp_c = temp_c;
    for ( int i = 0; i < 3; i++ ) {
        float gyro_output_float 	= (temp_c * input->gyro_m[i]) + input->gyro_b[i];
        output->gyro_bias_q16[i] 	= (long)(gyro_output_float * 65536.0f);
    }
}

uint16_t get_capability_flags()
{
    uint16_t capability_flags = 0;
    capability_flags |= (NAVX_CAPABILITY_FLAG_OMNIMOUNT +
            NAVX_CAPABILITY_FLAG_VEL_AND_DISP +
            NAVX_CAPABILITY_FLAG_YAW_RESET +
            NAVX_CAPABILITY_FLAG_AHRSPOS_TS);
    uint16_t yaw_axis_info =
            (((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis << 1) +
            ((((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis_up) ? 1 : 0);
    yaw_axis_info <<= 3;
    capability_flags |= yaw_axis_info;
    return capability_flags;
}

void update_capability_flags()
{
    registers.capability_flags = get_capability_flags();
}

int sense_current_yaw_orientation() {
    uint8_t mpu_yaw_axis;
    bool yaw_axis_up;
    if ( !sense_current_mpu_yaw_orientation( &mpu_yaw_axis, &yaw_axis_up) ) {
        ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis = mpu_yaw_axis;
        ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis_up = yaw_axis_up;
        return 0;
    }
    return -1;
}

uint8_t crc_lookup_table[256];

_EXTERN_ATTRIB void nav10_init()
{
    IMURegisters::buildCRCLookupTable(crc_lookup_table, sizeof(crc_lookup_table));

    registers.op_status		= NAVX_OP_STATUS_INITIALIZING;
    registers.hw_rev		= NAVX_HARDWARE_REV;
    registers.identifier	= NAVX_MODEL_NAVX_MXP;
    registers.fw_major		= NAVX_MXP_FIRMWARE_VERSION_MAJOR;
    registers.fw_minor		= NAVX_MXP_FIRMWARE_VERSION_MINOR;
    read_unique_id(&chipid);
    update_capability_flags();

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
    HAL_LED1_On(1);
    HAL_LED2_On(1);
    HAL_I2C_Power_Init();

    /* Ensure I2C is powered off, to handle case of a "soft" reset */
    HAL_I2C_Power_Off();
    HAL_Delay(50);

    if ( HAL_UART_Slave_Enabled() ) {
        Serial6.begin(57600);
    }

    /* Power on the on-board I2C devices */
    HAL_I2C_Power_On();
    HAL_Delay(1000);

    FlashStorage.init(sizeof(flash_cal_data));

    mpu_initialize(MPU9250_INT_Pin);
    enable_dmp();

    HAL_LED1_On(0);
    HAL_LED2_On(0);
    uint16_t flashdatasize;
    bool flashdatavalid;
    bool mpu_cal_complete = false;
    bool mag_cal_complete = false;
    registers.cal_status = NAVX_CAL_STATUS_IMU_CAL_INPROGRESS;
    registers.selftest_status = 0;
    flashdata = FlashStorage.get_mem(flashdatavalid,flashdatasize);
    if ( ( flashdatavalid ) &&
            ( flashdatasize == sizeof(struct flash_cal_data) ) ) {
        if ( ((struct flash_cal_data *)flashdata)->dmpcaldatavalid ) {
            mpu_apply_calibration_data( &((struct flash_cal_data *)flashdata)->mpucaldata );
            mpu_apply_dmp_gyro_biases(&((struct flash_cal_data *)flashdata)->dmpcaldata);
            set_current_mpu_to_board_xform(((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis,
                    ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis_up);
            update_capability_flags();
            last_gyro_bias_load_temperature = ((struct flash_cal_data *)flashdata)->dmpcaldata.mpu_temp_c;
            cal_led_cycle = flash_off;
            registers.op_status = NAVX_OP_STATUS_NORMAL;
            registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_ACCUMULATE;
            mpu_cal_complete = true;
        }
        if ( !is_mag_cal_data_default(&((struct flash_cal_data *)flashdata)->magcaldata) ) {
            mpu_apply_mag_cal_data(&((struct flash_cal_data *)flashdata)->magcaldata);
            registers.cal_status |= NAVX_CAL_STATUS_MAG_CAL_COMPLETE;
            mag_cal_complete = true;
        }
        registers.selftest_status = ((struct flash_cal_data *)flashdata)->selfteststatus | NAVX_SELFTEST_STATUS_COMPLETE;
    }

    /* If no MPU calibration data exists, detect yaw orientation, and run self tests which */
    /* will acquire MPU calibration data.  Note that the self-tests require correct yaw    */
    /* orientation in order to return correct results.                                     */

    if ( !mpu_cal_complete ) {
        cal_led_cycle = flash_slow;
        registers.op_status = NAVX_OP_STATUS_SELFTEST_IN_PROGRESS;
        uint8_t selftest_status = 0;
        while ( selftest_status != 7 ) {
            if ( !sense_current_yaw_orientation() ) {
                set_current_mpu_to_board_xform(((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis,
                        ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis_up);
                selftest_status = run_mpu_self_test(&((struct flash_cal_data *)flashdata)->mpucaldata,
                        ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis,
                        ((struct flash_cal_data *)flashdata)->orientationdata.yaw_axis_up);
                if ( selftest_status == 7 ) {
                    /* Self-test passed */
                    update_capability_flags();
                    mpu_apply_calibration_data(&((struct flash_cal_data *)flashdata)->mpucaldata);
                    /* Retrieve default magnetometer calibration data */
                    mpu_get_mag_cal_data(&((struct flash_cal_data *)flashdata)->magcaldata);
                    init_fusion_settings(&((struct flash_cal_data *)flashdata)->fusiondata);
                    ((struct flash_cal_data *)flashdata)->selfteststatus = selftest_status;
                    registers.selftest_status = selftest_status | NAVX_SELFTEST_STATUS_COMPLETE;
                    registers.op_status = NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS;
                    registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_INPROGRESS;
                    cal_led_cycle = flash_slow;
                } else {
                    /* Self-test failed, and we have no calibration data. */
                    /* Indicate error, and continue retrying.             */
                    cal_led_cycle = flash_fast;
                    registers.op_status = NAVX_OP_STATUS_ERROR;
                    registers.selftest_status = selftest_status;
                    /* Re-detect MPU on I2C bus */
                    HAL_LED2_On( mpu_detect() ? 1 : 0);
                }
            } else {
                /* Couldn't sense yaw orientation axis.  The board needs to be held still and */
                /* with one of the axes perpendicular to the earth.  Continue retrying...     */
            }
        }
    }

    /* Configure device and external communication interrupts */

    GPIO_InitTypeDef GPIO_InitStruct;

    /*Configure GPIO pin : PC8 (MPU) for rising-edge interrupts */
    GPIO_InitStruct.Pin = MPU9250_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MPU9250_INT_GPIO_Port, &GPIO_InitStruct);

    /* Configure GPIO pin : PC9 (CAL Button) for dual-edge interrupts */
    GPIO_InitStruct.Pin = CAL_BTN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CAL_BTN_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt initialization */
    /* TODO:  Review this priority post-integration of navX-PI HAL. */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4/*NVIC_PRIORITYGROUP_0*/);
    HAL_NVIC_SetPriority((IRQn_Type)EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);

    /* NavX-PI:  Enable EXTI on pins 10-15 */
    if((MPU9250_INT_Pin > GPIO_PIN_9) ||(CAL_BTN_Pin > GPIO_PIN_9)) {
    	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI15_10_IRQn);
    }

    /* Initiate Data Reception on slave SPI Interface, if it is enabled. */
    if ( HAL_SPI_Slave_Enabled() ) {
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
        HAL_SPI_Comm_Ready_Assert();
    }

    /* Initiate Data Reception on slave I2C Interface */
#ifndef DISABLE_EXTERNAL_I2C_INTERFACE
    prepare_next_i2c_receive();
#endif
}

struct mpu_data mpudata;
struct mpu_config mpuconfig;
unsigned long last_scan = 0;
bool calibration_active = false;
unsigned long cal_button_pressed_timestamp = 0;
unsigned long reset_imu_cal_buttonpress_period_ms = 2000;

bool schedule_caldata_clear = false;    /* Request by user to clear cal data */
int caldata_clear_count = 0;            /* Protection against flash wear */

#define BUTTON_DEBOUNCE_SAMPLES 10

static void cal_button_isr(void)
{
    /* If CAL button held down for sufficient duration, clear accel/gyro cal data */
    bool button_pressed = false;

    /* De-bounce the CAL button */

    int button_pressed_count = 0;
    for ( int i = 0; i < BUTTON_DEBOUNCE_SAMPLES; i++) {
        if ( HAL_CAL_Button_Pressed() ) {
            button_pressed_count++;
        }
    }
    if ( button_pressed_count > (BUTTON_DEBOUNCE_SAMPLES/2) ) {
        button_pressed = true;
    }

    if ( button_pressed ) {
        cal_button_pressed_timestamp = HAL_GetTick();
    } else {
        if ( (HAL_GetTick() - cal_button_pressed_timestamp) >= reset_imu_cal_buttonpress_period_ms) {
            if ( registers.op_status == NAVX_OP_STATUS_NORMAL ) {
                schedule_caldata_clear = true;
            }
        }
    }
}

int set_tuning_var( AHRS_DATA_ACTION action, AHRS_TUNING_VAR_ID tuning_var_id, float tuning_value, bool write )
{
    int status;
    if ( action == DATA_SET_TO_DEFAULT ) {
        if ( tuning_var_id == UNSPECIFIED ) {
            for ( int i = MIN_TUNING_VAR_ID; i <= MAX_TUNING_VAR_ID; i++ ) {
                status = set_tuning_var( action, (AHRS_TUNING_VAR_ID)i, 0.0f, false);
            }
        } else {
            struct fusion_settings default_fusion_settings;
            struct mag_calibration_data default_magcaldata;
            init_fusion_settings(&default_fusion_settings);

            switch ( tuning_var_id ) {
            case MOTION_THRESHOLD:
                status = set_tuning_var( DATA_SET, MOTION_THRESHOLD, default_fusion_settings.motion_detect_thresh_g, false);
                break;
            case YAW_STABLE_THRESHOLD:
                status = set_tuning_var( DATA_SET, YAW_STABLE_THRESHOLD, default_fusion_settings.yaw_change_detect_thresh_deg, false);
                break;
            case SEA_LEVEL_PRESSURE:
                status = set_tuning_var( DATA_SET, SEA_LEVEL_PRESSURE, default_fusion_settings.sealevel_pressure_millibars, false);
                break;
            case MAG_DISTURBANCE_THRESHOLD:
                get_default_mag_cal_data(&default_magcaldata);
                status = set_tuning_var( DATA_SET, MAG_DISTURBANCE_THRESHOLD, default_magcaldata.mag_disturbance_ratio, false);
                break;
            case UNSPECIFIED:
            default:
                status = DATA_GETSET_ERROR;
                break;
            }
        }
    } else if ( action == DATA_SET ) {
        switch ( tuning_var_id ) {
        case MOTION_THRESHOLD:
            (((struct flash_cal_data *)flashdata)->fusiondata).motion_detect_thresh_g = tuning_value;
            status = DATA_GETSET_SUCCESS;
            break;
        case YAW_STABLE_THRESHOLD:
            (((struct flash_cal_data *)flashdata)->fusiondata).yaw_change_detect_thresh_deg = tuning_value;
            status = DATA_GETSET_SUCCESS;
            break;
        case SEA_LEVEL_PRESSURE:
            (((struct flash_cal_data *)flashdata)->fusiondata).sealevel_pressure_millibars = tuning_value;
            status = DATA_GETSET_SUCCESS;
            break;
        case MAG_DISTURBANCE_THRESHOLD:
            (((struct flash_cal_data *)flashdata)->magcaldata).mag_disturbance_ratio = tuning_value;
            mpu_apply_mag_cal_data(&((struct flash_cal_data *)flashdata)->magcaldata);
            status = DATA_GETSET_SUCCESS;
            break;
        default:
            status = DATA_GETSET_ERROR;
            break;
        }
    } else {
        status = DATA_GETSET_ERROR; /* Get Response (not implemented on navX MXP) */
    }
    if ( write && ( status == DATA_GETSET_SUCCESS ) ) {
        status = (FlashStorage.commit() == HAL_OK) ? 0 : 1;
    }
    return status;
}

unsigned long start_timestamp = 0;	/* The time the "main" started. */
/* The time period (after the start timestamp) after which the yaw  */
/* offset calculation process will timeout.                         */
unsigned long yaw_offset_startup_timeout_ms = 20000;

/*****************************************
 * MPU Calibration
 *****************************************/

int yaw_offset_accumulator_count = 0;
float yaw_accumulator = 0.0;
float calibrated_yaw_offset = 0.0;
int imu_cal_bias_change_count = 0;
bool yaw_offset_calibration_complete = false;
unsigned long last_temp_compensation_check_timestamp = 0;

void update_config() {
    get_mpu_config(&mpuconfig);
    registers.accel_fsr_g = mpuconfig.accel_fsr;
    registers.gyro_fsr_dps = mpuconfig.gyro_fsr;
    registers.update_rate_hz = (uint8_t)mpuconfig.mpu_update_rate;
}

struct mpu_dmp_calibration_data curr_dmpcaldata;
struct mpu_dmp_calibration_data test_dmpcaldata;
struct mpu_dmp_calibration_interpolation_coefficients temp_correction_coefficients;
bool temp_correction_coefficients_valid = false;

PrintSerialReceiver* serial_interfaces[NUM_STREAMING_INTERFACES] = {&SerialUSB,&Serial6};
uint32_t last_packet_start_rx_timestamp[NUM_STREAMING_INTERFACES] = {0,0};
int num_serial_interfaces = 1;
/* Signal that sample rate has been updated by remote user */
bool sample_rate_update = true;
uint8_t new_sample_rate = 0;
/* Signal that integration control has been updated by remote user */
bool integration_control_update = true;
volatile uint8_t new_integration_control = 0;
uint32_t i2c_bus_reset_count = 0;
uint32_t i2c_interrupt_reset_count = 0;

_EXTERN_ATTRIB void nav10_main()
{
    unsigned long timestamp;
    int uart_slave_enabled = HAL_UART_Slave_Enabled();
    if (uart_slave_enabled) {
        num_serial_interfaces++;
    }

    attachInterrupt(GPIO_PIN_9,&cal_button_isr,CHANGE);

    if ( start_timestamp == 0 ) {
        start_timestamp = HAL_GetTick();
    }

    while (1)
    {
        bool send_stream_response[2] = { false, false };
        bool send_mag_cal_response[2] = { false, false };
        bool send_tuning_var_set_response[2] = { false, false };
        bool send_data_retrieval_response[2] = {false, false};
        bool send_integration_control_response[2] = {false,false};
        uint8_t tuning_var_status = 0;
        AHRS_TUNING_VAR_ID tuning_var_id = UNSPECIFIED, retrieved_var_id = UNSPECIFIED;
        AHRS_DATA_TYPE retrieved_data_type = BOARD_IDENTITY;
        AHRS_DATA_ACTION data_action = DATA_GET;
        uint8_t integration_control_action = 0;
        int32_t integration_control_parameter = 0;
        int num_update_bytes[2] = { 0, 0 };
        int num_resp_bytes[2] = { 0, 0 };

        periodic_compass_update();

        if ( sample_rate_update ) {
            if ( new_sample_rate != 0 ) {
                mpu_set_new_sample_rate(new_sample_rate);
                new_sample_rate = 0;
            }
            update_config();
            init_history();
            sample_rate_update = false;
            send_stream_response[0] = true;
            send_stream_response[1] = true;
        }

        if ( integration_control_update ) {

        	/* Read/Modify/Write integration control register */
			NVIC_DisableIRQ((IRQn_Type)I2C3_EV_IRQn);
			NVIC_DisableIRQ((IRQn_Type)SPI1_IRQn);
	        NVIC_DisableIRQ((IRQn_Type)DMA2_Stream0_IRQn);
			integration_control_update = false;
			uint8_t curr_vel_disp_int_control = new_integration_control & NAVX_INTEGRATION_CTL_VEL_AND_DISP_MASK;
			uint8_t curr_yaw_int_control = new_integration_control & NAVX_INTEGRATION_CTL_RESET_YAW;
			new_integration_control &= ~(curr_vel_disp_int_control | curr_yaw_int_control);
	        NVIC_EnableIRQ((IRQn_Type)DMA2_Stream0_IRQn);
			NVIC_EnableIRQ((IRQn_Type)SPI1_IRQn);
			NVIC_EnableIRQ((IRQn_Type)I2C3_EV_IRQn);

        	if ( curr_vel_disp_int_control != 0 ) {
        		reset_velocity_and_dispacement_integrator(&mpudata,curr_vel_disp_int_control);
        	}

            if ( curr_yaw_int_control != 0 ) {
                if ( yaw_offset_calibration_complete ) {
                    /* Current yaw angle now becomes zero degrees */
                    float curr_yaw_offset;
                    /* Disable MPU interrupts around the following
                     * pair of calls to get_yaw_offset() and set_yaw_offset(),
                     * while changing the yaw offset (which is used in the
                     * interrupt handler).
                     */
                    HAL_NVIC_DisableIRQ((IRQn_Type)EXTI9_5_IRQn);
                    get_yaw_offset(&curr_yaw_offset);
                    calibrated_yaw_offset = mpudata.yaw + curr_yaw_offset;
                    registers.yaw_offset = IMURegisters::encodeSignedHundredthsFloat(calibrated_yaw_offset);
                    /* Note:  protect against a race condition in which multiple
                     * Zero Yaw commands are received before the next MPU
                     * interrupt occurs.  In this condition, the mpudata.yaw value
                     * will not have been updated by the MPU interrupt.  Therefore,
                     * set the mpudata.yaw value to 0 immediately here. */
                    mpudata.yaw = 0.0f;
                    set_yaw_offset(calibrated_yaw_offset);
                    HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);
                }
            }
        }

        if ( schedule_caldata_clear && ( caldata_clear_count < 3 ) ) {
            /* Ensure that the CAL LED is lit for one second */
            HAL_CAL_LED_On(1);
            override_current_led_cycle( flash_on );
            /* clear the "valid calibration" flag and self-test results, write to flash */
            ((struct flash_cal_data *)flashdata)->dmpcaldatavalid = false;
            ((struct flash_cal_data *)flashdata)->selfteststatus = 0;
            FlashStorage.commit();
            schedule_caldata_clear = false;
            caldata_clear_count++;
        }

        if ( dmp_data_ready() ) {
            HAL_LED1_On(1);
            if ( get_dmp_data(&mpudata) == 0 ) {
                last_scan = HAL_GetTick();
                HAL_LED2_On(1);

                registers.yaw 		= IMURegisters::encodeSignedHundredthsFloat(mpudata.yaw);
                registers.pitch 	= IMURegisters::encodeSignedHundredthsFloat(mpudata.pitch);
                registers.roll 		= IMURegisters::encodeSignedHundredthsFloat(mpudata.roll);
                registers.heading	= IMURegisters::encodeUnsignedHundredthsFloat(mpudata.heading);
                registers.fused_heading = IMURegisters::encodeUnsignedHundredthsFloat(mpudata.fused_heading);
                registers.mpu_temp_c = IMURegisters::encodeSignedHundredthsFloat(mpudata.temp_c);
                registers.altitude = 0; 					/* Todo:  update from pressure sensor */
                registers.barometric_pressure = 0;			/* Todo:  update from pressure sensor */
                registers.pressure_sensor_temp_c = 0;		/* Todo:  update from pressure sensor */
                registers.timestamp = mpudata.timestamp;
                for ( int i = 0; i < 4; i++ ) {
                    registers.quat[i] = IMURegisters::encodeRatioFloat(
                            (float)(mpudata.quaternion[i] >> 16) / 16384.0f);
                }
                for ( int i = 0; i < 3; i++ ) {
                    registers.world_linear_accel[i] = IMURegisters::encodeSignedThousandthsFloat(
                            mpudata.world_linear_accel[i]);
                    registers.mag_raw[i]			= mpudata.calibrated_compass[i];
                    registers.accel_raw[i]			= mpudata.raw_accel[i];
                    registers.gyro_raw[i]			= mpudata.raw_gyro[i];
                    IMURegisters::encodeProtocol1616Float( mpudata.velocity[i], (char *)&registers.velocity[i]);
                    IMURegisters::encodeProtocol1616Float( mpudata.displacement[i], (char *)&registers.displacement[i]);
                }

                /* Peform motion detection, a deviation from the norm of world linear accel (X and Y axes) */
                float world_linear_accel_norm = fabs( sqrt( fabs (mpudata.world_linear_accel[0]*mpudata.world_linear_accel[0]) +
                        fabs (mpudata.world_linear_accel[1]*mpudata.world_linear_accel[1]) ) );
                world_accel_norm_history[world_acceleration_history_index] = world_linear_accel_norm;
                world_acceleration_history_index = (world_acceleration_history_index + 1) % current_world_accel_norm_history_size;
                float world_acceleration_history_sum = 0;
                for ( int i = 0; i < current_world_accel_norm_history_size; i++ ) {
                    world_acceleration_history_sum += world_accel_norm_history[i];
                }
                world_acceleration_norm_current_avg = world_acceleration_history_sum / current_world_accel_norm_history_size;
                if ( world_acceleration_norm_current_avg > (((struct flash_cal_data *)flashdata)->fusiondata).motion_detect_thresh_g ) {
                    registers.sensor_status |= NAVX_SENSOR_STATUS_MOVING;
                } else {
                    registers.sensor_status &= ~NAVX_SENSOR_STATUS_MOVING;
                }
                if ( mpudata.fused_heading_valid ) {
                    registers.sensor_status |= NAVX_SENSOR_STATUS_FUSED_HEADING_VALID;
                } else {
                    registers.sensor_status &= ~NAVX_SENSOR_STATUS_FUSED_HEADING_VALID;
                }

                /* Accumulate Yaw History */
                yaw_history[yaw_history_index] = mpudata.yaw;
                yaw_history_index = (yaw_history_index + 1) % current_yaw_history_size;
                float yaw_history_sum = 0;
                float yaw_history_min = 9999.0;
                float yaw_history_max = -9999.0;
                float yaw_delta;
                float curr_yaw;
                for ( int i = 0; i < current_yaw_history_size; i++ ) {
                    curr_yaw = yaw_history[i];
                    yaw_history_sum += curr_yaw;
                    if ( curr_yaw < yaw_history_min ) yaw_history_min = curr_yaw;
                    if ( curr_yaw > yaw_history_max ) yaw_history_max = curr_yaw;
                }
                yaw_delta = fabs( yaw_history_max - yaw_history_min );
                bool yaw_stable = ( yaw_delta < (((struct flash_cal_data *)flashdata)->fusiondata).yaw_change_detect_thresh_deg );
                if ( yaw_stable ) {
                    registers.sensor_status |= NAVX_SENSOR_STATUS_YAW_STABLE;
                } else {
                    registers.sensor_status &= ~NAVX_SENSOR_STATUS_YAW_STABLE;
                }
                if ( mpudata.magnetic_anomaly_detected ) {
                    registers.sensor_status |= NAVX_SENSOR_STATUS_MAG_DISTURBANCE;
#ifdef DEBUG_MAG_ANOMALY
                    SerialUSB.println("MAG ANOMALY!\r\n");
#endif
                } else {
                    registers.sensor_status &= ~NAVX_SENSOR_STATUS_MAG_DISTURBANCE;
                }

#ifdef DEBUG_MOTION_DETECTION
                char motion_string_buffer[40];
                sprintf(motion_string_buffer,"%s, Yaw:  %s\r\n",registers.motion_detected ? "Moving" : "Still", yaw_stable ? "Stable" : "Unstable");
                SerialUSB.write(motion_string_buffer);
#endif

                /* Send Streaming Updates */

                for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {

                    if ( update_type[ifx] == MSGID_AHRSPOS_TS_UPDATE ) {

                        num_update_bytes[ifx] = AHRSProtocol::encodeAHRSPosTSUpdate( update_buffer[ifx],
                                mpudata.yaw,
                                mpudata.pitch,
                                mpudata.roll,
                                mpudata.heading,
                                0.0, 							/* TODO:  Calc altitude    */
                                mpudata.fused_heading,
                                mpudata.world_linear_accel[0],
                                mpudata.world_linear_accel[1],
                                mpudata.world_linear_accel[2],
                                mpudata.temp_c,
                                (((float)mpudata.quaternion[0])/65536), /* Cvt 16:16 -> Float */
                                (((float)mpudata.quaternion[1])/65536), /* Cvt 16:16 -> Float */
                                (((float)mpudata.quaternion[2])/65536), /* Cvt 16:16 -> Float */
                                (((float)mpudata.quaternion[3])/65536), /* Cvt 16:16 -> Float */
                                mpudata.velocity[0],
                                mpudata.velocity[1],
                                mpudata.velocity[2],
                                mpudata.displacement[0],
                                mpudata.displacement[1],
                                mpudata.displacement[2],
                                registers.op_status,
                                registers.sensor_status,
                                registers.cal_status,
                                registers.selftest_status,
                                registers.timestamp);
                    }
                    else if ( update_type[ifx] == MSGID_AHRSPOS_UPDATE ) {

                        num_update_bytes[ifx] = AHRSProtocol::encodeAHRSPosUpdate( update_buffer[ifx],
                                mpudata.yaw,
                                mpudata.pitch,
                                mpudata.roll,
                                mpudata.heading,
                                0.0, 							/* TODO:  Calc altitude    */
                                mpudata.fused_heading,
                                mpudata.world_linear_accel[0],
                                mpudata.world_linear_accel[1],
                                mpudata.world_linear_accel[2],
                                mpudata.temp_c,
                                mpudata.quaternion[0] >> 16,
                                mpudata.quaternion[1] >> 16,
                                mpudata.quaternion[2] >> 16,
                                mpudata.quaternion[3] >> 16,
                                mpudata.velocity[0],
                                mpudata.velocity[1],
                                mpudata.velocity[2],
                                mpudata.displacement[0],
                                mpudata.displacement[1],
                                mpudata.displacement[2],
                                registers.op_status,
                                registers.sensor_status,
                                registers.cal_status,
                                registers.selftest_status);
                    }
                    else if ( update_type[ifx] == MSGID_AHRS_UPDATE ) {

                        num_update_bytes[ifx] = AHRSProtocol::encodeAHRSUpdate( update_buffer[ifx],
                                mpudata.yaw,
                                mpudata.pitch,
                                mpudata.roll,
                                mpudata.heading,
                                0.0, 							/* TODO:  Calc altitude    */
                                mpudata.fused_heading,
                                mpudata.world_linear_accel[0],
                                mpudata.world_linear_accel[1],
                                mpudata.world_linear_accel[2],
                                mpudata.temp_c,
                                mpudata.raw_compass[0],
                                mpudata.raw_compass[1],
                                mpudata.raw_compass[2],
                                mpudata.calibrated_compass[0],
                                mpudata.calibrated_compass[1],
                                mpudata.calibrated_compass[2],
                                mpudata.ratio_of_mag_field_norm,
                                mpudata.mag_norm_scalar,
                                mpudata.quaternion[0] >> 16,
                                mpudata.quaternion[1] >> 16,
                                mpudata.quaternion[2] >> 16,
                                mpudata.quaternion[3] >> 16,
                                0.0f, 							/* TODO: Calc Baro pressure */
                                0.0f, 							/* TODO: Acquire Baro Temp  */
                                registers.op_status,
                                registers.sensor_status,
                                registers.cal_status,
                                registers.selftest_status);

                    }
                    else if ( update_type[ifx] == MSGID_QUATERNION_UPDATE ) {

                        num_update_bytes[ifx] = IMUProtocol::encodeQuaternionUpdate( update_buffer[ifx],
                                mpudata.quaternion[0] >> 16,
                                mpudata.quaternion[1] >> 16,
                                mpudata.quaternion[2] >> 16,
                                mpudata.quaternion[3] >> 16,
                                mpudata.raw_accel[0],
                                mpudata.raw_accel[1],
                                mpudata.raw_accel[2],
                                mpudata.calibrated_compass[0],
                                mpudata.calibrated_compass[1],
                                mpudata.calibrated_compass[2],
                                mpudata.temp_c );

                    } else if ( update_type[ifx] == MSGID_GYRO_UPDATE ) {

                        num_update_bytes[ifx] = IMUProtocol::encodeGyroUpdate( update_buffer[ifx],
                                mpudata.raw_gyro[0],
                                mpudata.raw_gyro[1],
                                mpudata.raw_gyro[2],
                                mpudata.raw_accel[0],
                                mpudata.raw_accel[1],
                                mpudata.raw_accel[2],
                                mpudata.calibrated_compass[0],
                                mpudata.calibrated_compass[1],
                                mpudata.calibrated_compass[2],
                                mpudata.temp_c );

                    } else {

                        num_update_bytes[ifx] = IMUProtocol::encodeYPRUpdate( update_buffer[ifx],
                                mpudata.yaw,
                                mpudata.pitch,
                                mpudata.roll,
                                mpudata.heading);
                    }
                }

                if ( registers.op_status == NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS ) {

                    if ( mpu_did_dmp_gyro_biases_change(&((struct flash_cal_data *)flashdata)->dmpcaldata) ) {
                        imu_cal_bias_change_count++;
                    }

                    if ( ( ( registers.sensor_status & NAVX_SENSOR_STATUS_MOVING ) == 0 ) &&
                            ( ( registers.sensor_status & NAVX_SENSOR_STATUS_YAW_STABLE ) ) &&
                            ( imu_cal_bias_change_count > 2 ) ) {

                        /* Sensor is still, and DMP calibration constants have been accumulated */
                        /* Store to flash memory and begin accumulating yaw offsets.            */

                        registers.cal_status &= ~NAVX_CAL_STATUS_IMU_CAL_STATE_MASK;
                        registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_ACCUMULATE;
                        cal_led_cycle = flash_off;
                        ((struct flash_cal_data *)flashdata)->dmpcaldatavalid = true;
                        FlashStorage.commit();
                        registers.op_status = NAVX_OP_STATUS_NORMAL;
                        start_timestamp = HAL_GetTick();
                        /* Reset integration of velocity/displacement on all axes */
                        reset_velocity_and_dispacement_integrator(&mpudata,0x3F);
                    }
                }

                if ( registers.op_status == NAVX_OP_STATUS_NORMAL ) {

                    /* Accumulate yaw offset if not yet completely acquired. */

                    if ( !yaw_offset_calibration_complete ) {
                        if ( ( ( registers.sensor_status & NAVX_SENSOR_STATUS_MOVING ) == 0 ) &&
                                ( registers.sensor_status & NAVX_SENSOR_STATUS_YAW_STABLE ) ) {

                            yaw_accumulator += mpudata.yaw;
                            yaw_offset_accumulator_count++;

                            if ( yaw_offset_accumulator_count >= current_yaw_history_size ) {

                                registers.cal_status &= ~NAVX_CAL_STATUS_IMU_CAL_STATE_MASK;
                                registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_COMPLETE;

                                calibrated_yaw_offset = yaw_accumulator / yaw_offset_accumulator_count;
                                registers.yaw_offset = IMURegisters::encodeSignedHundredthsFloat(calibrated_yaw_offset);
                                set_yaw_offset(calibrated_yaw_offset);
                                yaw_offset_calibration_complete = true;
                                send_stream_response[0] = true;
                                send_stream_response[1] = true;
                                last_temp_compensation_check_timestamp = HAL_GetTick();
                                /* Reset integration of velocity/displacement on all axes */
                                reset_velocity_and_dispacement_integrator(&mpudata,0x3F);
                            }
                        } else {

                            if ( ( last_scan - start_timestamp ) > yaw_offset_startup_timeout_ms ) {

                                /* Timed out waiting for inital yaw offset accumulation.  This case */
                                /* can occur if the sensor is not still during the startup timeout  */
                                /* period.  In this case, set yaw/quat offsets to 0.                */

                                registers.cal_status &= ~NAVX_CAL_STATUS_IMU_CAL_STATE_MASK;
                                registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_COMPLETE;
                                calibrated_yaw_offset = 0.0f;
                                registers.yaw_offset = IMURegisters::encodeSignedHundredthsFloat(calibrated_yaw_offset);
                                set_yaw_offset(calibrated_yaw_offset);
                                yaw_offset_calibration_complete = true;
                                send_stream_response[0] = true;
                                send_stream_response[1] = true;
                                last_temp_compensation_check_timestamp = HAL_GetTick();
                                /* Reset integration of velocity/displacement on all axes */
                                reset_velocity_and_dispacement_integrator(&mpudata,0x3F);

                            } else {
                                yaw_offset_accumulator_count = 0;
                                yaw_accumulator = 0.0;
                            }
                        }
                    }

                    /* Periodically check for new dmp gyro biases if the temperature has changed. */

                    if ( ( last_scan - last_temp_compensation_check_timestamp ) > 1000 ) {
                        last_temp_compensation_check_timestamp = last_scan;
                        if ( ( ( registers.sensor_status & NAVX_SENSOR_STATUS_MOVING ) == 0 ) &&
                                ( registers.sensor_status & NAVX_SENSOR_STATUS_YAW_STABLE ) ) {
                            if ( mpu_did_dmp_gyro_biases_change(&curr_dmpcaldata) ) {
                                if ( fabs( curr_dmpcaldata.mpu_temp_c -
                                        ((struct flash_cal_data *)flashdata)->dmpcaldata.mpu_temp_c) >= 2.0 ) {
                                    calc_linear_temp_correction_coefficients( 	&(((struct flash_cal_data *)flashdata)->dmpcaldata),
                                            &curr_dmpcaldata,
                                            &temp_correction_coefficients);
                                    temp_correction_coefficients_valid = true;
                                }
                            }
                        }

                        /* Periodically re-calculate dmp gyro biases based upon current temperature */
                        if ( temp_correction_coefficients_valid ) {
                            interpolate_temp_correction_offsets( 	&temp_correction_coefficients,
                                    mpudata.temp_c,
                                    &test_dmpcaldata );
                            /* If temperature compensation slope is active */
                            /* read temp and apply new biases if temperature shift is detected */
                            if ( fabs( last_gyro_bias_load_temperature - mpudata.temp_c ) > .25 ) {
                                //mpu_apply_dmp_biases(&test_dmpcaldata); NOT WORKING VERY WELL YET
                                //last_gyro_bias_load_temperature = mpudata.temp_c;
                            }
                        }
                    }
                }
            } else {
                /* Error retrieving dmp data - possible I2C error. */
                bool mpu_present = mpu_detect();
                HAL_LED2_On( mpu_present ? 1 : 0);
                if ( !mpu_present ) {
                    timestamp = HAL_GetTick();
                    uint32_t max_wait_ms = 3 * (1000 / registers.update_rate_hz);
                    if ( ( last_scan > 0 ) && ( timestamp - last_scan > max_wait_ms ) ) {
                        bool bus_error = (__HAL_I2C_GET_FLAG(&hi2c2, I2C_FLAG_BERR) == SET);
                        bool bus_busy = (__HAL_I2C_GET_FLAG(&hi2c2, I2C_FLAG_BUSY) == SET);
                        if( bus_error || bus_busy ) {
                            /* Bus Busy, or Bus Error.  Reset the I2C Interface, in an attempt */
                            /* to resolve this condition.                                      */
                            HAL_I2C_Reset(&hi2c2);
                            i2c_bus_reset_count++;
                            //HAL_I2C_Reset(&hi2c2);
                            last_scan = HAL_GetTick();
                        }
                    }
                }
            }
        } else {
            /* No Data Ready Interrupt received */
            timestamp = HAL_GetTick();
            uint32_t max_wait_ms = 6 * (1000 / registers.update_rate_hz);
            if ( ( last_scan > 0 ) && ( timestamp - last_scan > max_wait_ms ) ) {
                HAL_LED1_On(0);
                HAL_LED2_On(0);
                uint32_t temp;
                bool group_enabled = false;
                bool pin_masked_in = false;
                bool rising_edge_trigger = false;
                bool falling_edge_trigger = false;
                bool pull_up_direction = false;
                temp = SYSCFG->EXTICR[2];
                if ( temp & 0x00000003 ) {
                    /* EXTI8 is enabled for GPIOC Group */
                    group_enabled = true;
                }
                temp = EXTI->IMR;
                if ( temp & 0x00000100 ) {
                    /* GPIO 8 is masked in */
                    pin_masked_in = true;
                }
                temp = EXTI->RTSR;
                if ( temp & 0x00000100 ) {
                    /* GPIO 8 is rising edge triggered */
                    rising_edge_trigger = true;
                }
                temp = EXTI->FTSR;
                if ( temp & 0x00000100 ) {
                    /* GPIO 8 is falling edge triggered */
                    falling_edge_trigger = true;
                }
                temp = GPIOC->PUPDR;
                if ( temp & 0x0001000 ) {
                    /* GPIO 8 is pull-up direction */
                    pull_up_direction = true;
                }

                /* HACK:  For some reason, in certain error cases (e.g., when  */
                /* power to I2C/SPI/UART is removed and then re-applied), the  */
                /* PC8 Interrupt Mask is disabled.  For the navX-MXP, this     */
                /* condition causes further MPU interrupts to be missed.       */
                /* Therefore, whenever this condition is detected, re-         */
                /* configure GPIO pin : PC8 (MPU) for rising-edge interrupts   */
                GPIO_InitTypeDef GPIO_InitStruct;

                GPIO_InitStruct.Pin = GPIO_PIN_8;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
                last_scan = timestamp;
                i2c_interrupt_reset_count++;
#ifdef DEBUG_I2C_COMM
                for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
                    serial_interfaces[ifx]->println("Scanning");
                }
#endif
            }
        }

        /* Process inbound configuration requests */
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
                    unsigned char update_rate_hz;
                    int packet_length = 0;
                    struct mag_calibration_data newcal;
                    AHRS_DATA_ACTION action;
                    float tuning_value;
                    char new_stream_type;
                    if ( (packet_length = IMUProtocol::decodeStreamCommand(
                            &inbound_data[i], bytes_remaining,
                            new_stream_type, update_rate_hz ) )  )
                    {
                        update_rate_hz = clip_sample_rate(update_rate_hz);
                        if ( update_rate_hz != mpuconfig.mpu_update_rate ) {
                            /* If update rate changed, send stream responses on all interfaces. */
                            sample_rate_update = true;
                            new_sample_rate = update_rate_hz;
                            mpuconfig.mpu_update_rate = new_sample_rate;
                            send_stream_response[0] = true;
                            send_stream_response[1] = true;
                        } else {
                            /* No update rate change.  Send stream response only on this interface. */
                            send_stream_response[ifx] = true;
                        }
                        if ( ( new_stream_type == MSGID_YPR_UPDATE) ||
                                ( new_stream_type == MSGID_QUATERNION_UPDATE ) ||
                                ( new_stream_type == MSGID_GYRO_UPDATE ) ||
                                ( new_stream_type == MSGID_AHRS_UPDATE ) ||
                                ( new_stream_type == MSGID_AHRSPOS_UPDATE ) ||
                                ( new_stream_type == MSGID_AHRSPOS_TS_UPDATE )) {
                            update_type[ifx] = new_stream_type;
                        }
                    }
                    else if ( ( packet_length = AHRSProtocol::decodeMagCalCommand(
                            &inbound_data[i], bytes_remaining,
                            data_action,
                            &(newcal.bias[0]), &(newcal.xform[0][0]),
                            newcal.earth_mag_field_norm ) ) ) {
                        if ( data_action == DATA_SET ) {
                            if ( newcal.earth_mag_field_norm == 0.0f ) {
                                /* Disallow 0 value, to avoid divide by . */
                                newcal.earth_mag_field_norm = 0.001f;
                            }
                            float previous_mag_disturbance_ratio = ((struct flash_cal_data *)flashdata)->magcaldata.mag_disturbance_ratio;
                            memcpy(&((struct flash_cal_data *)flashdata)->magcaldata,&newcal,sizeof(newcal));
                            ((struct flash_cal_data *)flashdata)->magcaldata.mag_disturbance_ratio = previous_mag_disturbance_ratio;
                            mpu_apply_mag_cal_data(&(((struct flash_cal_data *)flashdata)->magcaldata));
                            registers.cal_status |= NAVX_CAL_STATUS_MAG_CAL_COMPLETE;
                            FlashStorage.commit();
                        }
                        send_mag_cal_response[ifx] = true;
                    }
                    else if ( ( packet_length = AHRSProtocol::decodeTuningVariableCmd(
                            &inbound_data[i], bytes_remaining,
                            action, tuning_var_id, tuning_value ) ) ) {
                        tuning_var_status = set_tuning_var( action, tuning_var_id, tuning_value, true );
                        send_tuning_var_set_response[ifx] = true;
                    }
                    else if ( ( packet_length = AHRSProtocol::decodeDataGetRequest(
                            &inbound_data[i], bytes_remaining,
                            retrieved_data_type, retrieved_var_id ) ) ) {
                        send_data_retrieval_response[ifx] = true;
                    }
                    else if ( ( packet_length = AHRSProtocol::decodeIntegrationControlCmd(
                            &inbound_data[i], bytes_remaining,
                            integration_control_action, integration_control_parameter ) ) ) {
                        send_integration_control_response[ifx] = true;
                        new_integration_control |= integration_control_action;
                        integration_control_update = true;
                    }
                    if ( packet_length > 0 ) {
                        i += packet_length;
                    } else {
                        i++;
                    }
                }
            }

            /* Transmit responses, if necessary.  Only transmit responses */
            /* on the same interface on which they were received.         */

            if ( send_stream_response[ifx] ) {
                uint16_t flags = (registers.op_status == NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS) ?
                        NAV6_CALIBRATION_STATE_WAIT : (yaw_offset_calibration_complete ?
                                NAV6_CALIBRATION_STATE_COMPLETE : NAV6_CALIBRATION_STATE_ACCUMULATE );
                flags |= get_capability_flags();
                num_resp_bytes[ifx] += IMUProtocol::encodeStreamResponse(  response_buffer[ifx] + num_resp_bytes[ifx], update_type[ifx],
                        mpuconfig.gyro_fsr, mpuconfig.accel_fsr, mpuconfig.mpu_update_rate, calibrated_yaw_offset,
                        0,
                        0,
                        0,
                        0,
                        flags );
            }
            if ( send_mag_cal_response[ifx] ) {
                num_resp_bytes[ifx] += AHRSProtocol::encodeDataSetResponse(response_buffer[ifx] + num_resp_bytes[ifx], MAG_CALIBRATION, UNSPECIFIED, DATA_GETSET_SUCCESS);
            }
            if ( send_tuning_var_set_response[ifx] ) {
                num_resp_bytes[ifx] += AHRSProtocol::encodeDataSetResponse(response_buffer[ifx] + num_resp_bytes[ifx], TUNING_VARIABLE, tuning_var_id, tuning_var_status);
            }
            if ( send_data_retrieval_response[ifx] ) {
                float value;
                if ( retrieved_data_type == TUNING_VARIABLE ) {
                    switch ( retrieved_var_id ) {
                    case MOTION_THRESHOLD:
                        value = (((struct flash_cal_data *)flashdata)->fusiondata).motion_detect_thresh_g;
                        tuning_var_status = DATA_GETSET_SUCCESS;
                        break;
                    case YAW_STABLE_THRESHOLD:
                        value = (((struct flash_cal_data *)flashdata)->fusiondata).yaw_change_detect_thresh_deg;
                        tuning_var_status = DATA_GETSET_SUCCESS;
                        break;
                    case SEA_LEVEL_PRESSURE:
                        value = (((struct flash_cal_data *)flashdata)->fusiondata).sealevel_pressure_millibars;
                        tuning_var_status = DATA_GETSET_SUCCESS;
                        break;
                    case MAG_DISTURBANCE_THRESHOLD:
                        value = (((struct flash_cal_data *)flashdata)->magcaldata).mag_disturbance_ratio;
                        tuning_var_status = DATA_GETSET_SUCCESS;
                        break;
                    default:
                        /* Invalid case - return error */
                        value = 0;
                        tuning_var_status = DATA_GETSET_ERROR;
                        break;
                    }
                    num_resp_bytes[ifx] += AHRSProtocol::encodeTuningVariableCmd(response_buffer[ifx] + num_resp_bytes[ifx], DATA_GET, retrieved_var_id, value );
                } else if ( retrieved_data_type == MAG_CALIBRATION ) {
                    num_resp_bytes[ifx] += AHRSProtocol::encodeMagCalCommand(response_buffer[ifx] + num_resp_bytes[ifx], DATA_GET,
                            (int16_t *)&((struct flash_cal_data *)flashdata)->magcaldata.bias,
                            (float *)&((struct flash_cal_data *)flashdata)->magcaldata.xform,
                            ((struct flash_cal_data *)flashdata)->magcaldata.earth_mag_field_norm);
                } else if ( retrieved_data_type == BOARD_IDENTITY ) {
                    num_resp_bytes[ifx] += AHRSProtocol::encodeBoardIdentityResponse(response_buffer[ifx] + num_resp_bytes[ifx],
                            registers.identifier,
                            registers.hw_rev,
                            registers.fw_major,
                            registers.fw_minor,
                            NAVX_MXP_REVISION,
                            (uint8_t *)&chipid );
                }
            }
            if ( send_integration_control_response[ifx] ) {
                num_resp_bytes[ifx] += AHRSProtocol::encodeIntegrationControlResponse(response_buffer[ifx] + num_resp_bytes[ifx],
                        integration_control_action,
                        integration_control_parameter );
            }
        }

        /* Transmit Updates over available serial interface(s)                  */
        /* Transmit responses (if any) over the appropriate serial interface    */
        for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
            if ( num_resp_bytes[ifx] > 0 ) {
                memcpy(update_buffer[ifx] + num_update_bytes[ifx], response_buffer[ifx], num_resp_bytes[ifx]);
            }
            if ( ( num_update_bytes[ifx] + num_resp_bytes[ifx] ) > 0 ) {
                serial_interfaces[ifx]->write(update_buffer[ifx],num_update_bytes[ifx] + num_resp_bytes[ifx]);
            }
        }

        /* Shadow the calibration status into the upper 8-bits of the sensor status. */
        registers.sensor_status &= 0x00FF;
        registers.sensor_status |= ((uint16_t)registers.cal_status << 8);
        /* Update shadow registers; disable i2c/spi interrupts around this access. */
        if ( num_update_bytes[0] > 0 ) {
            NVIC_DisableIRQ((IRQn_Type)I2C3_EV_IRQn);
            NVIC_DisableIRQ((IRQn_Type)SPI1_IRQn);
            NVIC_DisableIRQ((IRQn_Type)DMA2_Stream0_IRQn);
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

        /* Peform I2C Glitch Detection/Correction */
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

        for ( int i = 0; i < NUM_SPI_BANKS; i++) {
        	if ( p_loop_func[i] != NULL ) {
        		p_loop_func[i]();
        	}
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
    if ( requested_address == NAVX_REG_UPDATE_RATE_HZ ) {
        *reg_addr = clip_sample_rate(value);
        new_sample_rate = *reg_addr;
        sample_rate_update = true;
    } else if ( requested_address == NAVX_REG_INTEGRATION_CTL) {
        /* This is a write-only register                        */
    	/* Thus, only setting of bits is allowed here (clearing */
    	/* of bits is performed in the foreground).             */
        new_integration_control |= value;
        integration_control_update = true;
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

bool spi_transmitting = false;
uint8_t spi_bytes_to_be_transmitted = 0;
unsigned long last_spi_tx_start_timestamp = 0;
#define SPI_TX_TIMEOUT_MS ((unsigned long)10)

void Reset_SPI() {
    HAL_SPI_Comm_Ready_Deassert();
    HAL_SPI_DeInit(&hspi1);
    HAL_SPI_Init(&hspi1);
    spi_transmitting = false;
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
    HAL_SPI_Comm_Ready_Assert();
}

int wrong_size_spi_receive_count = 0;
int invalid_char_spi_receive_count = 0;
int invalid_reg_address_spi_count = 0;
int spi_error_count = 0;
int spi_ovr_error_count = 0;
int spi_txe_error_count = 0;
int spi_rxne_error_count = 0;
int spi_busy_error_count = 0;
int spi_tx_complete_timeout_count = 0;

#ifdef ENABLE_BANKED_REGISTERS
int spi_rx_variable_message_len = 0;
uint8_t dummy;
#endif

uint8_t spi_tx_buffer[255];

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t *reg_addr;
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
                calculated_crc = IMURegisters::getCRCWithTable(crc_lookup_table,spi1_RxBuffer,num_bytes_received-1);
                if ( ( reg_address != 0xFF ) && (reg_count > 0 ) && ( received_crc == calculated_crc ) ) {
                    if ( reg_address & 0x80 ) {
                        write = true;
                        reg_address &= ~0x80;
                    }
#ifdef ENABLE_BANKED_REGISTERS
                    if (bank <= NUM_SPI_BANKS) {
                    	if (bank == 0) {
                           	reg_addr = GetRegisterAddressAndMaximumSize(reg_address, max_size);
                    	} else if (p_reg_lookup_func[bank] != NULL) {
                    		reg_addr = p_reg_lookup_func[bank](bank, reg_address, reg_count, &max_size);
                    	}
                    } else if (bank == COMM_MODE_BANK) {
                    	reg_addr = &dummy; /* Special mode does not access reg_addr */
                    }
                    else
#endif
                    {
                    	reg_addr = GetRegisterAddressAndMaximumSize(reg_address, max_size);
                    }
                    if ( reg_addr ) {
                    	if ( write ) {

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
                            HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, next_rcv_size);
                            HAL_SPI_Comm_Ready_Assert();
                        } else {
                        	HAL_AHRS_Int_Deassert();
                            spi_transmitting = true;
                            spi_bytes_to_be_transmitted = (reg_count > max_size) ? max_size : reg_count;
                            memcpy(spi_tx_buffer,reg_addr,spi_bytes_to_be_transmitted);
                            spi_tx_buffer[spi_bytes_to_be_transmitted] =
                                    IMURegisters::getCRCWithTable(crc_lookup_table, spi_tx_buffer,
                                                                  (uint8_t)spi_bytes_to_be_transmitted);
                            HAL_SPI_Transmit_DMA(&hspi1, spi_tx_buffer, spi_bytes_to_be_transmitted+1);
                            last_spi_tx_start_timestamp = HAL_GetTick();
                        	HAL_SPI_Comm_Ready_Deassert();
                        }
                    } else {
                        invalid_reg_address_spi_count++;
                        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                        HAL_SPI_Comm_Ready_Assert();
                    }
                } else {
                    invalid_char_spi_receive_count++;
                    /* If repeated invalid requests are received, and if the */
                    /* SPI interface is still busy, reset the SPI interface. */
                    /* This condition occurs sometimes after the RoboRIO     */
                    /* host computer is power-cycled.                        */
                    if ( ( (invalid_char_spi_receive_count % 5) == 0 ) &&
                         ( __HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_BSY) != RESET ) ) {
                        Reset_SPI();
                   } else {
                        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                        HAL_SPI_Comm_Ready_Assert();
                   }
                }
            } else {
                wrong_size_spi_receive_count++;
                HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
                HAL_SPI_Comm_Ready_Assert();
            }
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
        spi_transmitting = false;
        if ( hspi->ErrorCode == HAL_SPI_ERROR_OVR) {
            __HAL_SPI_CLEAR_OVRFLAG(hspi);
            spi_ovr_error_count++;
        } else if ( hspi->ErrorCode == HAL_SPI_ERROR_FLAG) {
        	/* Either RXNE,TXE, BSY flags are set */
        	uint32_t status_bits = hspi->Instance->SR;
        	if (status_bits & 0x00000001 ) {
        		spi_rxne_error_count++;
        	}
        	if (status_bits & 0x00000002) {
        		spi_txe_error_count++;
        	}
        	if (status_bits & 0x00000080) {
        		spi_busy_error_count++;
        	}
        }
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
        HAL_SPI_Comm_Ready_Assert();
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if ( hspi->Instance == SPI1 ) {
        spi_transmitting = false;
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)spi1_RxBuffer, SPI_RECV_LENGTH);
        HAL_SPI_Comm_Ready_Assert();
    }
}
