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
HAL_StatusTypeDef prepare_next_i2c_receive();

#define RXBUFFERSIZE 64
uint8_t i2c3_RxBuffer[RXBUFFERSIZE];
uint8_t spi1_RxBuffer[RXBUFFERSIZE];

#define MIN_SAMPLE_RATE_HZ 4
#define MAX_SAMPLE_RATE_HZ 60

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
char response_buffer[NUM_STREAMING_INTERFACES][AHRS_PROTOCOL_MAX_MESSAGE_LENGTH * 2];  /* Buffer for building serial reponse. */

struct fusion_settings {
	float motion_detect_thresh_g;
	float yaw_change_detect_thresh_deg;
	float sealevel_pressure_millibars;
};

/* nav10_cal_data is persisted to flash */
struct flash_cal_data {
	uint8_t									selfteststatus;
	struct mpu_selftest_calibration_data 	mpucaldata;
	struct mpu_dmp_calibration_data 		dmpcaldata;
	bool   									dmpcaldatavalid;
	struct mag_calibration_data				magcaldata;
	struct fusion_settings					fusiondata;
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
	uint8_t					identifier;
	uint8_t					hw_rev;
	uint8_t					fw_major;
	uint8_t					fw_minor;
	/* Read/Write registers */
	uint8_t					update_rate_hz;
	/* Read-only Registers */
	uint8_t					accel_fsr_g;
	uint16_t				gyro_fsr_dps;
	uint8_t					op_status;
	uint8_t					cal_status;
	uint8_t					selftest_status;
	/* Reserved */
	uint8_t					reserved[5];
	/* Processed Data */
	uint16_t				sensor_status;
	uint32_t				timestamp;
	s_short_hundred_float	yaw;
	s_short_hundred_float	pitch;
	s_short_hundred_float	roll;
	u_short_hundred_float	heading;
	u_short_hundred_float	fused_heading;
	s_1616_float			altitude;
	s_short_thousand_float	world_linear_accel[3];
	s_short_ratio_float 	quat[4];
	/* Raw Data */
	s_short_hundred_float	mpu_temp_c;
	int16_t 				gyro_raw[3];
	int16_t 				accel_raw[3];
	int16_t 				mag_raw[3];
	s_1616_float			barometric_pressure;
	s_short_hundred_float	pressure_sensor_temp_c;
	s_short_hundred_float 	yaw_offset;
	s_short_ratio_float 	quat_offset[4];
} registers, shadow_registers;

/* Statistical Averages */

#define YAW_HISTORY_PERIOD_MS 2000
#define WORLD_ACCEL_NORM_HISTORY_PERIOD_MS 250
#define WORLD_ACCEL_NORM_HISTORY_SIZE WORLD_ACCEL_NORM_HISTORY_PERIOD_MS / MIN_SAMPLE_PERIOD
#define YAW_HISTORY_SIZE YAW_HISTORY_PERIOD_MS / MIN_SAMPLE_PERIOD
float world_accel_norm_history[WORLD_ACCEL_NORM_HISTORY_SIZE];
float yaw_history[YAW_HISTORY_SIZE];
int world_acceleration_history_index = 0;
int yaw_history_index = 0;
float world_acceleration_norm_current_avg = 0.0f;
float yaw_current_average = 0.0f;

#define DEFAULT_MPU_SAMPLE_PERIOD_MS (1000 / DEFAULT_MPU_HZ)

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

int16_t last_mag_x = 0;
int16_t last_mag_y = 0;
int16_t last_mag_z = 0;

static const uint8_t flash_fast = 0b10101010; /* Self-test Fail */
static const uint8_t flash_slow = 0b11111110; /* IMU Cal */
static const uint8_t flash_on = 0b11111111; /* Startup, Mag Cal */
static const uint8_t flash_off = 0b00000000;

volatile int8_t cal_led_cycle_index = 7;
volatile uint8_t cal_led_cycle = flash_on;
float last_gyro_bias_load_temperature = 0.0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_CAL_LED_On((cal_led_cycle & (1 << cal_led_cycle_index)) ? 1 : 0);
	cal_led_cycle_index--;
	if (cal_led_cycle_index < 0) {
		cal_led_cycle_index = 7;
	}
}

void calc_linear_temp_correction_coefficients(		struct mpu_dmp_calibration_data *first,
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

void interpolate_temp_correction_offsets( 	struct mpu_dmp_calibration_interpolation_coefficients *input,
											float temp_c,
											struct mpu_dmp_calibration_data *output )
{
	output->mpu_temp_c = temp_c;
	for ( int i = 0; i < 3; i++ ) {
		float gyro_output_float 	= (temp_c * input->gyro_m[i]) + input->gyro_b[i];
		output->gyro_bias_q16[i] 	= (long)(gyro_output_float * 65536.0f);
	}
}

_EXTERN_ATTRIB void nav10_init()
{
	registers.op_status		= NAVX_OP_STATUS_INITIALIZING;
	registers.hw_rev		= NAVX_MXP_HARDWARE_REV;
	registers.identifier	= NAVX_MODEL_NAVX_MXP;
	registers.fw_major		= NAVX_MXP_FIRMWARE_VERSION_MAJOR;
	registers.fw_minor		= NAVX_MXP_FIRMWARE_VERSION_MINOR;
	read_unique_id(&chipid);

	__TIM11_CLK_ENABLE();
	/* Set Interrupt Group Priority; this is a low-priority interrupt */
	HAL_NVIC_SetPriority((IRQn_Type)TIM1_TRG_COM_TIM11_IRQn, 3, 15);
	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ((IRQn_Type)TIM1_TRG_COM_TIM11_IRQn);
	/* Initialize low-priority, 125ms timer */
	TimHandle.Instance = TIM11;
	/* Compute the prescaler value to have TIM3 counter clock equal to 10 KHz */
	uint32_t uwPrescalerValue = (uint32_t) ((SystemCoreClock /2) / 10000) - 1;
	/* Initialize TIM3 peripheral for 250ms Interrupt */
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
	HAL_Delay(50);

	FlashStorage.init(sizeof(flash_cal_data));

	mpu_initialize(GPIO_PIN_8);
	enable_dmp();

	HAL_LED1_On(0);
	HAL_LED2_On(0);
	uint16_t flashdatasize;
	bool flashdatavalid;
	registers.cal_status = NAVX_CAL_STATUS_IMU_CAL_INPROGRESS;
	registers.selftest_status = 0;
	flashdata = FlashStorage.get_mem(flashdatavalid,flashdatasize);
	if ( ( flashdatavalid ) &&
	   ( flashdatasize == sizeof(struct flash_cal_data) ) ) {
		if ( ((struct flash_cal_data *)flashdata)->dmpcaldatavalid ) {
			mpu_apply_calibration_data( &((struct flash_cal_data *)flashdata)->mpucaldata );
			mpu_apply_dmp_gyro_biases(&((struct flash_cal_data *)flashdata)->dmpcaldata);
			last_gyro_bias_load_temperature = ((struct flash_cal_data *)flashdata)->dmpcaldata.mpu_temp_c;
			cal_led_cycle = flash_off;
			registers.op_status = NAVX_OP_STATUS_NORMAL;
			registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_ACCUMULATE;
		} else {
			/* DMP Calibration Data is not valid */
			registers.op_status = NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS;
			registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_INPROGRESS;
			cal_led_cycle = flash_slow;
		}
		if ( !is_mag_cal_data_default(&((struct flash_cal_data *)flashdata)->magcaldata) ) {
			registers.cal_status |= NAVX_CAL_STATUS_MAG_CAL_COMPLETE;
		}
		mpu_apply_mag_cal_data(&((struct flash_cal_data *)flashdata)->magcaldata);
		registers.selftest_status = ((struct flash_cal_data *)flashdata)->selfteststatus;
	} else {
		cal_led_cycle = flash_slow;
		registers.op_status = NAVX_OP_STATUS_SELFTEST_IN_PROGRESS;
		uint8_t selftest_status = 0;
		while ( selftest_status != 7 ) {
			selftest_status = run_mpu_self_test(&((struct flash_cal_data *)flashdata)->mpucaldata);
			if ( selftest_status == 7 ) {
				mpu_apply_calibration_data(&((struct flash_cal_data *)flashdata)->mpucaldata);
				mpu_get_mag_cal_data(&((struct flash_cal_data *)flashdata)->magcaldata);
				init_fusion_settings(&((struct flash_cal_data *)flashdata)->fusiondata);
				registers.selftest_status = selftest_status | NAVX_SELFTEST_STATUS_COMPLETE;
				((struct flash_cal_data *)flashdata)->selfteststatus = selftest_status;
				registers.op_status = NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS;
				registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_INPROGRESS;
				cal_led_cycle = flash_slow;
			} else {
				/* Self-test failed, and we have no calibration data. */
				/* Indicate error                                     */
				cal_led_cycle = flash_fast;
				registers.op_status = NAVX_OP_STATUS_ERROR;
				registers.selftest_status = selftest_status;
				/* Detect MPU on I2C bus */
				HAL_LED2_On( mpu_detect() ? 1 : 0);
			}
		}
	}

	GPIO_InitTypeDef GPIO_InitStruct;

	/*Configure GPIO pin : PC8 (MPU) for rising-edge interrupts */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin : PC9 (CAL Button) for dual-edge interrupts */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* EXTI interrupt initialization */
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4/*NVIC_PRIORITYGROUP_0*/);
	HAL_NVIC_SetPriority((IRQn_Type)EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ((IRQn_Type)EXTI9_5_IRQn);

	/* Initiate Data Reception on slave SPI Interface, if it is enabled. */
	if ( HAL_SPI_Slave_Enabled() ) {
		HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer, 3);
	}

	/* Initiate Data Reception on slave I2C Interface */
	HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
}

struct mpu_data mpudata;
struct mpu_config mpuconfig;
unsigned long last_scan = 0;
bool calibration_active = false;
unsigned long cal_button_pressed_timestamp = 0;
unsigned long reset_imu_cal_buttonpress_period_ms = 5000;

static void cal_button_isr(void)
{
	/* If CAL button held down for sufficient duration, clear accel/gyro cal data */
	if ( HAL_CAL_Button_Pressed() ) {
		cal_button_pressed_timestamp = HAL_GetTick();
	} else {
		if ( (HAL_GetTick() - cal_button_pressed_timestamp) >= reset_imu_cal_buttonpress_period_ms) {
			HAL_CAL_LED_On(1);
			((struct flash_cal_data *)flashdata)->dmpcaldatavalid = false;
			FlashStorage.commit();
			HAL_CAL_LED_On(0);
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
float quaternion_accumulator[4] = { 0.0, 0.0, 0.0, 0.0 };
float calibrated_yaw_offset = 0.0;
float calibrated_quaternion_offset[4] = { 0.0, 0.0, 0.0, 0.0 };
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
int num_serial_interfaces = 1;
bool sample_rate_update = true;
uint8_t new_sample_rate = 0;

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
	    uint8_t tuning_var_status;
	    AHRS_TUNING_VAR_ID tuning_var_id, retrieved_var_id;
	    AHRS_DATA_TYPE retrieved_data_type;
	    AHRS_DATA_ACTION data_action;
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

	    			if ( update_type[ifx] == MSGID_AHRS_UPDATE ) {

						num_update_bytes[ifx] = AHRSProtocol::encodeAHRSUpdate( update_buffer[ifx],
													mpudata.yaw - calibrated_yaw_offset,
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
													mpudata.yaw - calibrated_yaw_offset,
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
						 ( imu_cal_bias_change_count > 0 ) ) {

						/* Sensor is still, and DMP calibration constants have been accumulated */
						/* Store to flash memory and begin accumulating yaw offsets.            */

						registers.cal_status &= ~NAVX_CAL_STATUS_IMU_CAL_STATE_MASK;
						registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_ACCUMULATE;
						cal_led_cycle = flash_off;
						((struct flash_cal_data *)flashdata)->dmpcaldatavalid = true;
						FlashStorage.commit();
						registers.op_status = NAVX_OP_STATUS_NORMAL;
						start_timestamp = HAL_GetTick();
					}
				}

				if ( registers.op_status == NAVX_OP_STATUS_NORMAL ) {

					/* Accumulate yaw offset if not yet completely acquired. */

					if ( !yaw_offset_calibration_complete ) {
						if ( ( ( registers.sensor_status & NAVX_SENSOR_STATUS_MOVING ) == 0 ) &&
							 ( registers.sensor_status & NAVX_SENSOR_STATUS_YAW_STABLE ) ) {

							yaw_accumulator += mpudata.yaw;
							for ( int i = 0; i < 4; i++ ) {
								quaternion_accumulator[i] += (float)(mpudata.quaternion[i] >> 16) / 16384.0f;
							}
							yaw_offset_accumulator_count++;

							if ( yaw_offset_accumulator_count >= current_yaw_history_size ) {

								registers.cal_status &= ~NAVX_CAL_STATUS_IMU_CAL_STATE_MASK;
								registers.cal_status |= NAVX_CAL_STATUS_IMU_CAL_COMPLETE;

								calibrated_yaw_offset = yaw_accumulator / yaw_offset_accumulator_count;
								registers.yaw_offset = IMURegisters::encodeSignedHundredthsFloat(calibrated_yaw_offset);
								for ( int i = 0; i < 4; i++ ) {
									calibrated_quaternion_offset[i] =
											quaternion_accumulator[i] / yaw_offset_accumulator_count;
									registers.quat_offset[i] = calibrated_quaternion_offset[i];
								}
								yaw_offset_calibration_complete = true;
								send_stream_response[0] = true;
								send_stream_response[1] = true;
								last_temp_compensation_check_timestamp = HAL_GetTick();
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
								for ( int i = 0; i < 4; i++ ) {
									calibrated_quaternion_offset[i] = 0.0f;
									registers.quat_offset[i] = 0.0f;
								}
								yaw_offset_calibration_complete = true;
								send_stream_response[0] = true;
								send_stream_response[1] = true;
								last_temp_compensation_check_timestamp = HAL_GetTick();

							} else {
								yaw_offset_accumulator_count = 0;
								yaw_accumulator = 0.0;
								for ( int i = 0; i < 4; i++ ) {
									quaternion_accumulator[i] = 0.0f;
								}
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
						if(__HAL_I2C_GET_FLAG(&hi2c2, I2C_FLAG_BUSY) == SET) {
							/* Bus Busy.  Reset the I2C Interface, in an attempt */
							/* to resolve this condition.                        */
							HAL_I2C_MspDeInit(&hi2c2);
							HAL_I2C_MspInit(&hi2c2);
						}
					}
			}
		} else {
			/* No Data Ready Interrupt received */
			timestamp = HAL_GetTick();
			if ( timestamp - last_scan > (unsigned long)500 ) {
				HAL_LED1_On(0);
				HAL_LED2_On(0);
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

#ifdef DEBUG_I2C_COMM
				for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
					serial_interfaces[ifx]->println("Scanning");
				}
#endif
			}
		}

		/* Process inbound configuration requests */
		for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
			/* Scan for start of message, removing any bytes that precede it. */
			bool found_start_of_message = false;
			while ( serial_interfaces[ifx]->available() > 0 ) {
				char rcv_byte = serial_interfaces[ifx]->peek();
				if ( rcv_byte != PACKET_START_CHAR ) {
					serial_interfaces[ifx]->read();
				}
				else {
					HAL_Delay(2);
					found_start_of_message = true;
					break;
				}
			}

			/* If sufficient bytes have been received, process the data and */
			/* if a valid message is received, handle it.                   */

			if( found_start_of_message && ( serial_interfaces[ifx]->available() >= (STREAM_CMD_MESSAGE_LENGTH-1) ) ) {
				size_t bytes_read = 0;
				while ( serial_interfaces[ifx]->available() ) {
					if ( bytes_read >= sizeof(inbound_data) ) {
						break;
					}
					inbound_data[bytes_read++] = serial_interfaces[ifx]->read();
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
							 ( new_stream_type == MSGID_AHRS_UPDATE ) ) {
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
								/* Disallow 0 value, to avoid divide by zero. */
								newcal.earth_mag_field_norm = 0.001f;
							}
							float previous_mag_disturbance_ratio = ((struct flash_cal_data *)flashdata)->magcaldata.mag_disturbance_ratio;
							memcpy(&((struct flash_cal_data *)flashdata)->magcaldata,&newcal,sizeof(newcal));
							((struct flash_cal_data *)flashdata)->magcaldata.mag_disturbance_ratio = previous_mag_disturbance_ratio;
							mpu_apply_mag_cal_data(&newcal);
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
				num_resp_bytes[ifx] = IMUProtocol::encodeStreamResponse(  response_buffer[ifx], update_type[ifx],
																	mpuconfig.gyro_fsr, mpuconfig.accel_fsr, mpuconfig.mpu_update_rate, calibrated_yaw_offset,
																	(uint16_t)(calibrated_quaternion_offset[0] * 16384),
																	(uint16_t)(calibrated_quaternion_offset[1] * 16384),
																	(uint16_t)(calibrated_quaternion_offset[2] * 16384),
																	(uint16_t)(calibrated_quaternion_offset[3] * 16384),
																	(registers.op_status == NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS) ?
																			NAV6_CALIBRATION_STATE_WAIT : (yaw_offset_calibration_complete ?
																					NAV6_CALIBRATION_STATE_COMPLETE : NAV6_CALIBRATION_STATE_ACCUMULATE ) );
			}
			if ( send_mag_cal_response[ifx] ) {
				num_resp_bytes[ifx] = AHRSProtocol::encodeDataSetResponse(response_buffer[ifx], MAG_CALIBRATION, UNSPECIFIED, DATA_GETSET_SUCCESS);
			}
			if ( send_tuning_var_set_response[ifx] ) {
				num_resp_bytes[ifx] = AHRSProtocol::encodeDataSetResponse(response_buffer[ifx], TUNING_VARIABLE, tuning_var_id, tuning_var_status);
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
						tuning_var_status = DATA_GETSET_ERROR;
						break;
					}
					num_resp_bytes[ifx] = AHRSProtocol::encodeTuningVariableCmd(response_buffer[ifx], DATA_GET, retrieved_var_id, value );
				} else if ( retrieved_data_type == MAG_CALIBRATION ) {
					num_resp_bytes[ifx] = AHRSProtocol::encodeMagCalCommand(response_buffer[ifx], DATA_GET,
							(int16_t *)&((struct flash_cal_data *)flashdata)->magcaldata.bias,
							(float *)&((struct flash_cal_data *)flashdata)->magcaldata.xform,
							((struct flash_cal_data *)flashdata)->magcaldata.earth_mag_field_norm);
				} else if ( retrieved_data_type == BOARD_IDENTITY ) {
					num_resp_bytes[ifx] = AHRSProtocol::encodeBoardIdentityResponse(response_buffer[ifx],
							registers.identifier,
							registers.hw_rev,
							registers.fw_major,
							registers.fw_minor,
							NAVX_MXP_REVISION,
							(uint8_t *)&chipid );
				}
			}
		}

		/* Transmit Updates available serial interface(s) */
		/* Transmit responses (if any) over the appropriate serial interface */

		for ( int ifx = 0; ifx < num_serial_interfaces; ifx++ ) {
			if ( num_resp_bytes[ifx] > 0 ) {
				memcpy(update_buffer[ifx] + num_update_bytes[ifx], response_buffer[ifx], num_resp_bytes[ifx]);
			}
			if ( ( num_update_bytes[ifx] + num_resp_bytes[ifx] ) > 0 ) {
				serial_interfaces[ifx]->write(update_buffer[ifx],num_update_bytes[ifx] + num_resp_bytes[ifx]);
			}
		}

		if ( num_update_bytes[0] > 0 ) {
			/* Update shadow registers; disable i2c/spi interrupts around this access. */
		    HAL_NVIC_DisableIRQ((IRQn_Type)I2C3_EV_IRQn);
		    HAL_NVIC_DisableIRQ((IRQn_Type)SPI1_IRQn);
		    memcpy(&shadow_registers, &registers, sizeof(registers));
		    HAL_NVIC_EnableIRQ((IRQn_Type)SPI1_IRQn);
		    if ( prepare_i2c_receive_timeout_count > 0 ) {
				if ( (HAL_GetTick() - prepare_i2c_receive_last_attempt_timestamp) > (unsigned long)5000) {
					if ( HAL_OK == prepare_next_i2c_receive() ) {
						prepare_i2c_receive_timeout_count = 0;
					}
		    	}
		    }
		    HAL_NVIC_EnableIRQ((IRQn_Type)I2C3_EV_IRQn);
		}
	}
}

uint8_t *GetRegisterAddressAndMaximumSize( uint8_t register_address, uint16_t& size )
{
	if ( register_address > NAVX_REG_LAST) {
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
int i2c_error_count = 0;
HAL_I2C_ErrorTypeDef last_i2c_err_code = HAL_I2C_ERROR_NONE;

/* Prepare for next reception on the I2C3 (external control) port */
/* This function includes a retry mechanism, and also will reset  */
/* the port in case of a timeout.                                 */

HAL_StatusTypeDef prepare_next_i2c_receive()
{
	HAL_StatusTypeDef status;
	status = HAL_BUSY;
	unsigned long prepare_i2c_receive_last_attempt_timestamp = HAL_GetTick();
	while ( status == HAL_BUSY )
	{
		status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
		if ( (HAL_GetTick() - prepare_i2c_receive_last_attempt_timestamp) > I2C_PREPARE_RECEIVE_TIMEOUT_MS) {
			HAL_I2C_DeInit(&hi2c3);
			HAL_I2C_Init(&hi2c3);
			status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
			if ( status != HAL_OK ) {
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
				status = HAL_I2C_Slave_Receive_IT(&hi2c3, (uint8_t*)i2c3_RxBuffer, 2);
				if ( status != HAL_OK ) {
					prepare_i2c_receive_timeout_count++;
				}
			}
			break;
		}
	}
	return status;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	HAL_StatusTypeDef status;
	uint8_t *reg_addr;
	uint16_t max_size;
	uint16_t num_bytes_received = I2cHandle->XferSize - I2cHandle->XferCount;
	uint8_t *rx_bytes_start = I2cHandle->pBuffPtr - num_bytes_received;
	bool i2c_transmitting = false;
	bool write = false;
	if ( I2cHandle->Instance == I2C3 ) {
		if ( I2cHandle->Instance->SR2 & I2C_SR2_TRA ) {
		} else {
			/* Master Receive bit set:  1 byte is received, the address of the register to begin reading from */
			if ( num_bytes_received == 2 ) {
				uint8_t requested_address = rx_bytes_start[0];
				uint8_t num_bytes = rx_bytes_start[1];
				if ( requested_address & 0x80 ) {
					write = true;
					requested_address &= ~0x80;
				}
				reg_addr = GetRegisterAddressAndMaximumSize(requested_address, max_size);
				if ( reg_addr ) {
					if ( write ) {
						if ( requested_address == NAVX_REG_UPDATE_RATE_HZ ) {
							*reg_addr = clip_sample_rate(num_bytes);
							new_sample_rate = *reg_addr;
							sample_rate_update = true;
						}
					} else {
						uint8_t i2c_bytes_to_be_transmitted = (num_bytes > max_size) ? max_size : num_bytes;
						memcpy(i2c_tx_buffer,reg_addr,i2c_bytes_to_be_transmitted);
						status = HAL_BUSY;
						while ( status == HAL_BUSY ) /* Consider a timeout on this wait... */
						{
							status = HAL_I2C_Slave_Transmit_IT(&hi2c3, i2c_tx_buffer, i2c_bytes_to_be_transmitted);
						}
						if ( status == HAL_OK ) {
							i2c_transmitting = true;
							i2c_tx_in_progress = true;
							last_i2c_tx_start_timestamp = HAL_GetTick();
						}
					}
				}
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

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
	/* The "AF" (Ack fail) error will occur in the case of a burst read */
	/* if the receiver terminates the read before all bytes are         */
	/* transmitted.  In this case, simply clear the transmit buffer     */
	/* pointer; else, reset the I2C interface.                          */
	if ( I2cHandle->Instance == I2C3 )
	{
		last_i2c_err_code = I2cHandle->ErrorCode;
		if ( I2cHandle->ErrorCode != HAL_I2C_ERROR_AF) {
			HAL_I2C_DeInit(I2cHandle);
			HAL_I2C_Init(I2cHandle);
		}
		if ( I2cHandle->ErrorCode == HAL_I2C_ERROR_AF ) {
			I2cHandle->XferCount = 0;
			I2cHandle->XferSize = 0;
			I2cHandle->pBuffPtr = 0;
		}
		i2c_tx_in_progress = false;
		prepare_next_i2c_receive();
		i2c_error_count++;
	}
}

bool transmitting = false;
uint8_t spi_bytes_to_be_transmitted = 0;
unsigned long last_spi_tx_start_timestamp = 0;
#define SPI_TX_TIMEOUT_MS ((unsigned long)10)

void Reset_SPI() {
	HAL_SPI_DeInit(&hspi1);
	HAL_SPI_Init(&hspi1);
	transmitting = false;
	HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer, 3);
}

int wrong_size_spi_receive_count = 0;
int invalid_char_spi_receive_count = 0;
int invalid_reg_address_spi_count = 0;
int spi_error_count = 0;
int spi_ovr_error_count = 0;
int spi_tx_complete_timeout_count = 0;

uint8_t spi_tx_buffer[NAVX_REG_LAST + 1];

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	uint8_t *reg_addr;
	uint16_t max_size;
	uint8_t reg_address;
	uint8_t reg_count;
	uint8_t received_crc;
	uint8_t calculated_crc;
	int i;
	register uint8_t byte_to_send;
	bool write = false;
	if ( hspi->Instance == SPI1 ) {
		if ( !transmitting ) {
			int num_bytes_received = hspi->RxXferSize - hspi->RxXferCount;
			if ( num_bytes_received == 3 ) {
				reg_address = spi1_RxBuffer[0];
				reg_count = spi1_RxBuffer[1];
				received_crc = spi1_RxBuffer[2];
				calculated_crc = IMURegisters::getCRC(spi1_RxBuffer,2);
				if ( ( reg_count != 0xFF ) && ( reg_address != 0xFF ) && ( received_crc == calculated_crc ) ) {
					if ( reg_address & 0x80 ) {
						write = true;
						reg_address &= ~0x80;
					}
					reg_addr = GetRegisterAddressAndMaximumSize(reg_address, max_size);
					if ( reg_addr ) {
						if ( write ) {
							if ( reg_address == NAVX_REG_UPDATE_RATE_HZ ) {
								*reg_addr = clip_sample_rate(reg_count);
								new_sample_rate = *reg_addr;
								sample_rate_update = true;
								HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer,3);
							}

						} else {
							last_spi_tx_start_timestamp = HAL_GetTick();
							transmitting = true;
							spi_bytes_to_be_transmitted = (reg_count > max_size) ? max_size : reg_count;
							for ( i = 0; i < spi_bytes_to_be_transmitted; i++ ) {
								byte_to_send = reg_addr[i];
								spi_tx_buffer[i] = byte_to_send;
							}
							spi_tx_buffer[spi_bytes_to_be_transmitted] = IMURegisters::getCRC(spi_tx_buffer,(uint8_t)spi_bytes_to_be_transmitted);
							HAL_SPI_Transmit_DMA(&hspi1, spi_tx_buffer, spi_bytes_to_be_transmitted+1);
						}
					} else {
						invalid_reg_address_spi_count++;
						HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer,3);
					}
				} else {
					invalid_char_spi_receive_count++;
					HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer,3);
				}
			} else {
				wrong_size_spi_receive_count++;
				HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer,3);
			}
		}
	}
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if ( hspi->Instance == SPI1 ) {
		// HAL_SPI_ERROR_OVR: Master has transmitted bytes, the slave hasn't read the
		// previous byte in the receive register.  This case occurs when the
		// Receive ISR hasn't been responded to before the master starts writing.
		/* Prepare for next receive */
		spi_error_count++;
		/* This is a bit heavy-handed, perhaps a simpler, faster mechanism exists? */
		transmitting = false;
		if ( hspi->ErrorCode == HAL_SPI_ERROR_OVR) {
			__HAL_SPI_CLEAR_OVRFLAG(hspi);
			spi_ovr_error_count++;
		}
		HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer, 3);
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if ( hspi->Instance == SPI1 ) {
		transmitting = false;
		HAL_SPI_Receive_IT(&hspi1, (uint8_t *)spi1_RxBuffer, 3);
	}
}

