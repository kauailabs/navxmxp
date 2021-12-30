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

#include "mpucontroller.h"
#include "stm32_shim.h"
#include "usb_serial.h"
#include <math.h>
#include "helper_3dmath.h"
#include "navx-mxp_hal.h"

extern "C" {
#include "mpl.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "invensense.h"
#include "mltypes.h"
#include "invensense_adv.h"
#include "eMPL_outputs.h"
#include "stm32f4xx_hal.h"
}
#include <stdio.h>
#define PEDO_READ_MS    (1000)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)
#define COMPASS_ON      (0x04)

extern I2C_HandleTypeDef hi2c2;

int _MLPrintLog(int priority, const char *tag, const char *fmt, ...) { return 0; }

/******************************************************
 * DMP
 *****************************************************/

struct hal_s {
    unsigned char lp_accel_mode;
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned char motion_int_mode;
    unsigned long no_dmp_hz;
    unsigned long next_pedo_ms;
    unsigned long next_temp_ms;
    unsigned long next_compass_ms;
    unsigned int report;
    unsigned short dmp_features;
};
static struct hal_s hal = {0};
struct mag_calibration_data magcaldata;

/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s {
    signed char orientation[9];
};

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 */

/* 'Logical' Axes Orientation - As viewed by navX MXP Users.         */
/* This is often referred to as the "World" reference frame.         */

#define MPU_LOGICAL_AXIS_X	0   // PITCH: Increases as 'right' lifts
#define MPU_LOGICAL_AXIS_Y	1	// ROLL:  Increases as 'front' lifts
#define MPU_LOGICAL_AXIS_Z	2	// YAW:   Increases as rotates clockwise

#define NUM_LOGICAL_AXES	3

/* Defines the transform from mpu axes to board axes orientation */

static struct platform_data_s mpu_to_board_orientation_matrices[NUM_LOGICAL_AXES * 2] = {
    // MPU_X_AXIS_DOWN_YAW
    { {  0, -1,  0,     // x = chip y (inverted)
         0,  0,  1,     // y = chip z
        -1,  0,  0} },  // z = chip x (inverted)
    // MPU_Y_AXIS_DOWN_YAW
    { { -1,  0,  0,     // x = chip x (inverted)
         0,  0, -1,     // y = chip z (inverted)
         0, -1,  0} },  // z = chip y (inverted)
    // MPU_Z_AXIS_DOWN_YAW
    { { -1,  0,  0,     // x = chip x (inverted)
         0,  1,  0,     // y = chip y
         0,  0, -1} },  // z = chip z (inverted)
    // MPU_X_AXIS_UP_YAW
    { {  0, -1,  0,     // x = chip y (inverted)
         0,  0, -1,     // y = chip z (inverted)
         1,  0,  0} },  // z = chip x
    // MPU_Y_AXIS_UP_YAW
    { {  1,  0,  0,     // x = chip x
         0,  0, -1,     // y = chip z (inverted)
         0,  1,  0} },  // z = chip y
    // MPU_Z_AXIS_UP_YAW
    { {  1,  0,  0,     // x = chip x
         0,  1,  0,     // y = chip y
         0,  0,  1} }   // z = chip z
};

const int default_mpu_yaw_axis = MPU_LOGICAL_AXIS_Z;
const bool default_mpu_yaw_axis_up = true;
float motion_detection_threshold_g = 0.02f;

uint8_t curr_mpu_yaw_axis = default_mpu_yaw_axis;
bool curr_mpu_yaw_axis_up = default_mpu_yaw_axis_up;
struct platform_data_s curr_mpu_to_board_orientation_matrix;

_EXTERN_ATTRIB int set_current_mpu_to_board_xform(uint8_t yaw_axis, bool up) {
    platform_data_s *p_selected = 0;
    int status = -1;
    unsigned short gyro_fsr;
    unsigned char accel_fsr;
    if ( yaw_axis > (NUM_LOGICAL_AXES-1) ) return status;
    size_t index = yaw_axis;
    if ( up ) {
        index += NUM_LOGICAL_AXES;
    }
    curr_mpu_yaw_axis = yaw_axis;
    p_selected = &(mpu_to_board_orientation_matrices[index]);
    memcpy(&curr_mpu_to_board_orientation_matrix, p_selected, sizeof(platform_data_s));
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
    inv_set_gyro_orientation_and_scale(
            inv_orientation_matrix_to_scalar(curr_mpu_to_board_orientation_matrix.orientation),
            (long)gyro_fsr<<15);
    inv_set_accel_orientation_and_scale(
            inv_orientation_matrix_to_scalar(curr_mpu_to_board_orientation_matrix.orientation),
            (long)accel_fsr<<15);
    status = dmp_set_orientation(
            inv_orientation_matrix_to_scalar(curr_mpu_to_board_orientation_matrix.orientation));
    return status;
}

void set_default_mpu_to_board_xform() {
    set_current_mpu_to_board_xform(default_mpu_yaw_axis, default_mpu_yaw_axis_up);
}

/* Defines the transform from compass axes to mpu axes orientation */

static struct platform_data_s compass_to_mpu_orientation_matrix = {
        {  0,  1,  0, // x = chip y
                1,  0,  0, // y = chip x
                0,  0, -1} // z = chip z (inverted)
};

void xform_compass_to_mpu_coordinates( short raw_compass_data[3] ) {
    short raw_compass_data_copy[3];
    for ( int i = 0; i < 3; i++ ) {
        raw_compass_data_copy[i] = raw_compass_data[i];
    }
    for ( int i = 0; i < 3; i++ ) {
        raw_compass_data[i] = 0;
        for ( int j = 0; j < 3; j++ ) {
            if ( compass_to_mpu_orientation_matrix.orientation[(i*3)+j] != 0 ) {
                raw_compass_data[i] =
                        raw_compass_data_copy[j] *
                        compass_to_mpu_orientation_matrix.orientation[(i*3)+j];
            }
        }
    }
};

void xform_mpu_to_board_coordinates( short mpu_coordinate_data[3] ) {
    short mpu_coordinate_data_copy[3];
    for ( int i = 0; i < 3; i++ ) {
        mpu_coordinate_data_copy[i] = mpu_coordinate_data[i];
    }
    for ( int i = 0; i < 3; i++ ) {
        mpu_coordinate_data[i] = 0;
        for ( int j = 0; j < 3; j++ ) {
            if ( curr_mpu_to_board_orientation_matrix.orientation[(i*3)+j] != 0 ) {
                mpu_coordinate_data[i] =
                        mpu_coordinate_data_copy[j] *
                        curr_mpu_to_board_orientation_matrix.orientation[(i*3)+j];
            }
        }
    }
}

#define COMPASS_ENABLED 1
unsigned char *mpl_key = (unsigned char*)"eMPL 5.1";

static const int16_t default_magcal_bias = 0;
static const float default_magcal_xform = 1.0f;
/* By default, set the norm of the earth's mag field to a small value. */
/* This effectively results in constant disturbance detection, which   */
/* is appropriate since the disturbance detection algorithm cannot     */
/* be trusted until the magnetometer has been calibrated.              */
static const float default_magcal_earth_mag_field_norm = 1.0f;
static const float default_magcal_magdisturbance_ratio = 0.19f; /* Empirically-determined default */

float yaw_at_last_new_compass_heading = 0.0f;
float last_new_compass_heading = 0.0f;
float last_valid_fused_heading = 0.0f;
float yaw_at_last_valid_fused_heading = 0.0f;
float compass_heading_noise_band_degrees = 2.0f;
bool first_valid_fused_heading_detected = false;
int valid_fused_heading_count = 0;

_EXTERN_ATTRIB bool is_mag_cal_data_default( struct mag_calibration_data *magcaldata )
{
    bool is_default = true;
    for ( size_t i = 0; i < sizeof(magcaldata->bias)/sizeof(magcaldata->bias[0]); i++ ) {
        if ( magcaldata->bias[i] != default_magcal_bias ) {
            is_default = false;
        }
    }
    for ( int x = 0; x < 3; x++ ) {
        for ( int y = 0; y < 3; y++ ) {
            if ( magcaldata->xform[x][y] != default_magcal_xform ) {
                is_default = false;
            }
        }
    }
    if ( magcaldata->earth_mag_field_norm != default_magcal_earth_mag_field_norm ) {
        is_default = false;
    }
    if ( magcaldata->mag_disturbance_ratio != default_magcal_magdisturbance_ratio ) {
        is_default = false;
    }
    return is_default;
}

_EXTERN_ATTRIB void get_default_mag_cal_data(struct mag_calibration_data *magcaldata)
{
    for ( size_t i = 0; i < sizeof(magcaldata->bias)/sizeof(magcaldata->bias[0]); i++ ) {
        magcaldata->bias[i] = default_magcal_bias;
    }
    for ( int x = 0; x < 3; x++ ) {
        for ( int y = 0; y < 3; y++ ) {
            magcaldata->xform[x][y] = default_magcal_xform;
        }
    }
    magcaldata->earth_mag_field_norm = default_magcal_earth_mag_field_norm;
    magcaldata->mag_disturbance_ratio = default_magcal_magdisturbance_ratio;
}

_EXTERN_ATTRIB void mpu_apply_mag_cal_data(struct mag_calibration_data *caldata) {
    memcpy(&magcaldata,caldata,sizeof(magcaldata));
    /* Reset the fused heading tracking variables */
    last_new_compass_heading = 0.0f;
    last_valid_fused_heading = 0.0f;
    yaw_at_last_valid_fused_heading = 0.0f;
    first_valid_fused_heading_detected = false;
    valid_fused_heading_count = 0;
}

_EXTERN_ATTRIB void mpu_get_mag_cal_data(struct mag_calibration_data *caldata) {
    memcpy(caldata, &magcaldata, sizeof(magcaldata));
}

/* Every time new gyro data is available, this function is called in an
 * ISR context. In this example, it sets a flag protecting the FIFO read
 * function.
 */
static void gyro_data_ready_cb(uint8_t gpio_pin)
{
    hal.new_gyro = 1;
}

/* This function is invoked by core/driver/stm32L/log_stm32.c */

_EXTERN_ATTRIB void fputchar( char *string)
{
}

static void tap_cb(unsigned char direction, unsigned char count)
{
    switch (direction) {
    case TAP_X_UP:
        break;
    case TAP_X_DOWN:
        break;
    case TAP_Y_UP:
        break;
    case TAP_Y_DOWN:
        break;
    case TAP_Z_UP:
        break;
    case TAP_Z_DOWN:
        break;
    default:
        return;
    }
    return;
}

static void android_orient_cb(unsigned char orientation)
{
    switch (orientation) {
    case ANDROID_ORIENT_PORTRAIT:
        break;
    case ANDROID_ORIENT_LANDSCAPE:
        break;
    case ANDROID_ORIENT_REVERSE_PORTRAIT:
        break;
    case ANDROID_ORIENT_REVERSE_LANDSCAPE:
        break;
    default:
        return;
    }
}

unsigned long sensor_timestamp = 0;
unsigned long sensor_timestamp_dmp_fifo = 0;
unsigned long sensor_timestamp_mpu_temp = 0;
unsigned char accel_fsr = 0;

_EXTERN_ATTRIB void mpu_initialize(unsigned short mpu_interrupt_pin)
{
    inv_error_t result;
    unsigned short gyro_rate, gyro_fsr;
    struct int_param_s int_param;

#ifdef COMPASS_ENABLED
    unsigned short compass_fsr;
    get_default_mag_cal_data(&magcaldata);
#endif

    /* Set up gyro.
     * Every function preceded by mpu_ is a driver function and can be found
     * in inv_mpu.h.
     */
    int_param.cb = gyro_data_ready_cb;
    int_param.pin = mpu_interrupt_pin;
    result = mpu_init(&int_param);
    if (result) {
        /* Reset */
    }
#if 0 /* MPL Initialization - not used */
    /* If you're not using an MPU9150 AND you're not using DMP features, this
     * function will place all slaves on the primary bus.
     * mpu_set_bypass(1);
     */

    result = inv_init_mpl();
    if (result) {
        /* Reset */
    }

    /* Compute 6-axis and 9-axis quaternions. */
    //inv_enable_quaternion();
    /* Update gyro biases when not in motion.
     * WARNING: These algorithms are mutually exclusive.
     */
    inv_enable_fast_nomot();
    /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();

#ifndef ENABLE_NAVX_HW_CALIBRATION
    inv_enable_in_use_auto_calibration();
#endif
#ifdef COMPASS_ENABLED
    /* Compass calibration algorithms. */
#endif
    /* If you need to estimate your heading before the compass is calibrated,
     * enable this algorithm. It becomes useless after a good figure-eight is
     * detected, so we'll just leave it out to save memory.
     * inv_enable_heading_from_gyro();
     */

    inv_enable_heading_from_gyro();

    /* Allows use of the MPL APIs in read_from_mpl. */
    inv_enable_eMPL_outputs();

    result = inv_start_mpl();
    if (result == INV_ERROR_NOT_AUTHORIZED) {
        while (1) {
            delay_ms(5000);
        }
    }
    if (result) {
        /* Reset */
    }
#endif
    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
#ifdef COMPASS_ENABLED
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
#else
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#endif
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);
#ifdef COMPASS_ENABLED
    /* The compass sampling rate can be less than the gyro/accel sampling rate.
     * Use this function for proper power management.
     */
    mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);
#endif

    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
#ifdef COMPASS_ENABLED
    mpu_get_compass_fsr(&compass_fsr);
#endif
#if 0
    /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / gyro_rate);
    inv_set_accel_sample_rate(1000000L / gyro_rate);
#ifdef COMPASS_ENABLED
    /* The compass rate is independent of the gyro and accel rates. As long as
     * inv_set_compass_sample_rate is called with the correct value, the 9-axis
     * fusion algorithm's compass correction gain will work properly.
     */
    inv_set_compass_sample_rate(COMPASS_READ_MS * 1000L);
#endif
#endif
    /* Initialize HAL state variables. */
#ifdef COMPASS_ENABLED
    hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
#else
    hal.sensors = ACCEL_ON | GYRO_ON;
#endif
    hal.dmp_on = 0;
    hal.report = 0;
    hal.next_pedo_ms = 0;
    hal.next_compass_ms = 0;
    hal.next_temp_ms = 0;

    /* Compass reads are handled by scheduler. */
    get_ms(&sensor_timestamp);

    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    dmp_load_motion_driver_firmware();
    set_current_mpu_to_board_xform(MPU_LOGICAL_AXIS_Z, true);
    dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);

    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
            DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
            DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(hal.dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(0);
    hal.dmp_on = 0;

    /* Enable Interrupts */
}

_EXTERN_ATTRIB void get_mpu_config(struct mpu_config *pconfig)
{
    dmp_get_fifo_rate(&pconfig->mpu_update_rate);
    mpu_get_gyro_fsr(&pconfig->gyro_fsr);
    mpu_get_accel_fsr(&pconfig->accel_fsr);
}

/* Read all 3 accelerometer axes, whichever axis has the greatest
 * absolute value (~1G) is the axis best suited to be used
 * for orientation.
 *
 * Note that this function has a threshold, meaning that this
 * function will only succeed if (a) the device is still and
 * thus acceleration due to motion is minimal and (b) an axis
 * can be found which is at least X% (80?) of 1G, translating
 * to a maximum allowable angle from vertical.
 */

const uint8_t INVALID_AXIS = 255;

_EXTERN_ATTRIB int sense_current_mpu_yaw_orientation( uint8_t *mpu_yaw_axis, bool* up)
{
    short raw_accel_data[3];
    float accel_data_g[3];
    long raw_accel_data_accumulator[3] = {0,0,0};
    unsigned long sample_timestamp;
    unsigned char accel_fsr;
    unsigned short sample_rate;
    unsigned long sample_period_ms;
    int iteration_count = 10;
    mpu_get_accel_fsr(&accel_fsr);
    mpu_get_sample_rate(&sample_rate);
    sample_period_ms = 1000 / sample_rate;
    unsigned char valid_yaw_axis = INVALID_AXIS;
    bool valid_axis_up = true;

    int curr_iteration = 0;
    while ( curr_iteration < iteration_count ) {
        if(!mpu_get_accel_reg(raw_accel_data, &sample_timestamp)) {
            /* Accumulate acceleration*/
            for ( int i = 0; i < 3; i++) {
                raw_accel_data_accumulator[i] += raw_accel_data[i];
            }
        } else {
            return -1;
        }
        delay_ms(sample_period_ms);
        curr_iteration++;
    }
    /* Compute mean acceleration in each axis */
    for ( int i = 0; i < 3; i++) {
        raw_accel_data[i] = (short)(raw_accel_data_accumulator[i] / iteration_count);
    }
    /* Find the axis, if any, which has a value close to +/- 1g */
    for ( int i = 0; i < 3; i++) {
        accel_data_g[i] = (((float)raw_accel_data[i]) / (32768.0 / accel_fsr));
        if ( ( fabs(accel_data_g[i]) > .65 ) && ( fabs(accel_data_g[i]) < 1.35 ) ) {
            if ( valid_yaw_axis == INVALID_AXIS ) {
                valid_yaw_axis = i;
                valid_axis_up = (accel_data_g[i] > 0);
            } else {
                /* More than on axis at/near 1G.  This is an error. */
                return -1;
            }
        }
    }
    if ( valid_yaw_axis != INVALID_AXIS ) {
        *mpu_yaw_axis = valid_yaw_axis;
        *up = valid_axis_up;
    }
    return (valid_yaw_axis != INVALID_AXIS) ? 0 : -1;
}

_EXTERN_ATTRIB int run_mpu_self_test(struct mpu_selftest_calibration_data *caldata, uint8_t yaw_axis, bool yaw_axis_up)
{
    int result;
    int i;
    long gyro[3], accel[3];

#if defined (MPU6500) || defined (MPU9250)
    result = mpu_run_6500_self_test(gyro, accel, 0);
#elif defined (MPU6050) || defined (MPU9150)
    result = mpu_run_self_test(gyro, accel);
#endif

    /* The MPU driver self test always assumes the Yaw axis is the Z axis */
    /* and subtracts/adds 1G to that axis to offset gravity.  Therefore,  */
    /* if a different yaw axis is chosen, gravity should be added back to */
    /* the Z axis, and removed from the chosen yaw axis.                  */

    if ( ( yaw_axis < 3 ) && ( yaw_axis != 2 ) ) {
        /* Add back previously-subtracted gravity */
        if ( accel[2] < 0 ) {
            accel[2] += 65536L;
        } else {
            accel[2] -= 65536L;
        }
        /* Remove gravity from the chosen yaw axis */
        if ( yaw_axis_up ) {
            accel[yaw_axis] -= 65536L;
        } else {
            accel[yaw_axis] += 65536L;
        }
        /* If a different yaw axis is chosen, the bias test in the MPU    */
        /* driver will fail.  In this case, clear any accelerometer error */
        /* result.  Note that in the factory, self tests should always    */
        /* be run w/the default axis orientation (Yaw axis = z, dir = up) */
        result |= 0x2;
    }

    if (result == 0x7) {
        /* Test passed. We can trust the gyro data here, so now we need to update calibrated data*/
        for ( i=0; i < 3; i++ ) {
            caldata->accel_bias[i] = accel[i];
            caldata->gyro_bias[i]  = gyro[i];
        }
        mpu_get_accel_sens(&caldata->accel_sensitivity);
        mpu_get_gyro_sens(&caldata->gyro_sensitivity);

    }
    return result;
}

uint8_t curr_sample_rate = DEFAULT_MPU_HZ;
float   curr_sample_period = 1.0f / DEFAULT_MPU_HZ + 10;
float   curr_sample_period_squared = (1.0f / DEFAULT_MPU_HZ ) * (1.0f / DEFAULT_MPU_HZ);

void update_curr_sample_rate( uint8_t new_sample_rate )
{
    curr_sample_rate = new_sample_rate;
    curr_sample_period = 1.0f / curr_sample_rate;
    curr_sample_period_squared = curr_sample_period * curr_sample_period;
}

_EXTERN_ATTRIB void enable_dmp()
{
    if ( hal.dmp_on ) return;

    unsigned short sample_rate;
    hal.dmp_on = 1;
    /* Preserve current FIFO rate. */
    mpu_get_sample_rate(&sample_rate);
    update_curr_sample_rate(sample_rate);
    dmp_set_fifo_rate(sample_rate);
#if 0
    inv_set_quat_sample_rate(1000000L / sample_rate);
#endif
    mpu_set_dmp_state(1);
    MPL_LOGI("DMP enabled.\n");
}

#define USE_CAL_MPU_REGISTERS

_EXTERN_ATTRIB int mpu_apply_calibration_data(struct mpu_selftest_calibration_data *caldata)
{
    long gyro[3], accel[3];

#ifdef USE_CAL_MPU_REGISTERS
    /*
     * This portion of the code uses the HW offset registers that are in the MPUxxxx devices
     * instead of pushing the cal data to the MPL software library
     * These values must be in h/w units, an w/a 1000dps gyro / 8G accel scale
     */
    unsigned char i = 0;

    for(i = 0; i<3; i++) {
        gyro[i] = (long)(caldata->gyro_bias[i] * 32.768f); //convert to +-1000dps in HW units
        gyro[i] = (long)(gyro[i] >> 16);
        accel[i] = caldata->accel_bias[i];
        accel[i] *= 2048.f; //convert to +-16G in HW units
        accel[i] = accel[i] >> 16;
    }
    mpu_set_gyro_bias_reg(gyro);
#if defined (MPU6500) || defined (MPU9250)
    /* Disable setting accelerometer bias, since this interacts in */
    /* less-than-desirable ways w/the DMP's accelerometer bias     */
    /* calculations.  The only downside to this is that the raw    */
    /* Accelerometer values are not bias-corrected.                */
    mpu_set_accel_bias_6500_reg(accel);
#elif defined (MPU6050) || defined (MPU9150)
    mpu_set_accel_bias_6050_reg(accel);
#endif

#elif defined (USE_CAL_DMP_REGISTERS)
    float sens;
    unsigned short accel_sens;
    mpu_get_gyro_sens(&sens);
    gyro[0] = (long)(caldata->gyro_bias[0] * sens);
    gyro[1] = (long)(caldata->gyro_bias[1] * sens);
    gyro[2] = (long)(caldata->gyro_bias[2] * sens);
    dmp_set_gyro_bias(gyro);
    mpu_get_accel_sens(&accel_sens);
    accel[0] = (long)(caldata->accel_bias[0] *accel_sens);
    accel[1] = (long)(caldata->accel_bias[1] *accel_sens);
    accel[2] = (long)(caldata->accel_bias[2] *accel_sens);
    dmp_set_accel_bias(accel);
#else  /* !USE_CAL_HW_REGISTERS */

    /* Push the calibration biases to the MPL Library. These biases are */
    /* in h/w units, but * 2<<16                                        */

    accel[0] = caldata->accel_bias[0] * caldata->accel_sensitivity;
    accel[1] = caldata->accel_bias[1] * caldata->accel_sensitivity;
    accel[2] = caldata->accel_bias[2] * caldata->accel_sensitivity;
    inv_set_accel_bias(accel, 3);
    gyro[0] = (long) (caldata->gyro_bias[0] * caldata->gyro_sensitivity);
    gyro[1] = (long) (caldata->gyro_bias[1] * caldata->gyro_sensitivity);
    gyro[2] = (long) (caldata->gyro_bias[2] * caldata->gyro_sensitivity);
    inv_set_gyro_bias(gyro, 3);
#endif /* USE_CAL_HW_REGISTERS */
    return 0;
}

#define PI 3.1415926f
const float radians_to_degrees = 180.0f / M_PI;
const float degrees_to_radians = M_PI / 180.0f;

_EXTERN_ATTRIB int dmp_data_ready()
{
    return hal.new_gyro;
}

struct FloatVectorStruct {
    float x;
    float y;
    float z;
};

void getEuler(float *data, Quaternion *q) {

    data[0] = atan2(2*q -> x*q -> y - 2*q -> w*q -> z, 2*q -> w*q -> w + 2*q -> x*q -> x - 1);   // psi
    data[1] = -asin(2*q -> x*q -> z + 2*q -> w*q -> y);                              // theta
    data[2] = atan2(2*q -> y*q -> z - 2*q -> w*q -> x, 2*q -> w*q -> w + 2*q -> z*q -> z - 1);   // phi
}

void getGravity(struct FloatVectorStruct *v, Quaternion *q) {

    v -> x = 2 * (q -> x*q -> z - q -> w*q -> y);
    v -> y = 2 * (q -> w*q -> x + q -> y*q -> z);
    v -> z = q -> w*q -> w - q -> x*q -> x - q -> y*q -> y + q -> z*q -> z;
}

void dmpGetYawPitchRoll(float *data, Quaternion *q, struct FloatVectorStruct *gravity) {

    // yaw: (about Z axis)
    data[0] = atan2(2*q -> x*q -> y - 2*q -> w*q -> z, 2*q -> w*q -> w + 2*q -> x*q -> x - 1);
    // pitch: (node up/down, about X axis)
    data[1] = atan(gravity -> y / sqrt(gravity -> x*gravity -> x + gravity -> z*gravity -> z));
    // roll: (tilt left/right, about Y axis)
    data[2] = atan(gravity -> x / sqrt(gravity -> y*gravity -> y + gravity -> z*gravity -> z));
}

long last_temperature = 32 * 65535; /* Reasonable default */
long compass_accumulator[3] = {0,0,0};
long compass_accumulator_count = 0;
unsigned long next_compass_ms = 0;
#define COMPASS_SAMPLE_PERIOD_MS ((unsigned long)10)
int new_compass = 0;
int new_temp = 0;
int new_data = 0;
uint32_t non_moving_sample_count[2] = { 100, 100 };
float curr_yaw_offset_radians = 0;

_EXTERN_ATTRIB int periodic_compass_update() {
    short new_compass_data_short[3];
    int ret = 0;
    unsigned long curr_timestamp = HAL_GetTick();

    /* Read compass data if sufficient time has passed. */
    if (curr_timestamp > next_compass_ms) {
        next_compass_ms = curr_timestamp + COMPASS_SAMPLE_PERIOD_MS;

        ret = mpu_get_compass_reg(new_compass_data_short, &curr_timestamp);
        if (!ret) {
            // Xform Compass data from chip to board orientation
            xform_compass_to_mpu_coordinates( new_compass_data_short );
            /* Accumulate Compass Data */
            compass_accumulator[0] += new_compass_data_short[0];
            compass_accumulator[1] += new_compass_data_short[1];
            compass_accumulator[2] += new_compass_data_short[2];
            compass_accumulator_count++;
        }
    }
    return ret;
}

_EXTERN_ATTRIB int get_dmp_data( struct mpu_data *pdata )
{
    short gyro[3], accel_short[3];
    FloatVectorStruct gravity;
    short sensors;
    unsigned char more;
    uint64_t hires_timestamp;
    if ( !hal.new_gyro ) return -1;
    if ( NULL == pdata ) return -1;

    hires_timestamp = HAL_IOCX_HIGHRESTIMER_Get();
    get_ms(&sensor_timestamp);
    if (!dmp_read_fifo(gyro, accel_short, pdata->quaternion, &sensor_timestamp_dmp_fifo, &sensors, &more) ) {

        /* Read mpu temperature data if sufficient time has passed. */
        if (sensor_timestamp > hal.next_temp_ms) {
            hal.next_temp_ms = sensor_timestamp + TEMP_READ_MS;
            new_temp = 1;
        }

#ifdef COMPASS_ENABLED
        /* Due to compass noise, average each sample (period = COMPASS_READ_MS) */
        if ((sensor_timestamp > hal.next_compass_ms) && !hal.lp_accel_mode &&
                hal.new_gyro && (hal.sensors & COMPASS_ON)) {
            hal.next_compass_ms = sensor_timestamp + COMPASS_READ_MS;
            new_compass = 1;
        }
#endif

        if ( !more ) {
            hal.new_gyro = 0;
        }

        if (sensors & INV_XYZ_GYRO) {
            new_data = 1;
            if (new_temp) {
                new_temp = 0;
                if ( !mpu_get_temperature(&last_temperature, &sensor_timestamp_mpu_temp) ) {
                    /* Correct for discrepancy between datasheet and mpu_get_temperature implementation */
                    last_temperature -= (14 * 65536);
                    pdata->temp_c = last_temperature / 65536.0;
                }
            }
        }
        if (sensors & INV_XYZ_ACCEL) {
            /* Transform raw accelerometer data to board orientation */
            xform_mpu_to_board_coordinates(accel_short);
            new_data = 1;
        }
        if (sensors & INV_WXYZ_QUAT) {
            new_data = 1;
        }

#ifdef COMPASS_ENABLED
        periodic_compass_update();
        if (new_compass && ( compass_accumulator_count > 0 )) {
            pdata->raw_compass[0] = (short)(compass_accumulator[0] / compass_accumulator_count);
            pdata->raw_compass[1] = (short)(compass_accumulator[1] / compass_accumulator_count);
            pdata->raw_compass[2] = (short)(compass_accumulator[2] / compass_accumulator_count);

            /* Reset accumulator */
            compass_accumulator_count = 0;
            compass_accumulator[0] = 0;
            compass_accumulator[1] = 0;
            compass_accumulator[2] = 0;

            new_data = 1;
        }
#endif

        if ( new_data ) {
            //inv_execute_on_data();
            /* This function reads bias-compensated sensor data and sensor
             * fusion outputs from the MPL. The outputs are formatted as seen
             * in eMPL_outputs.c. This function only needs to be called at the
             * rate requested by the host.
             */
            float ypr[3];
            float last_accel[3];			// Last sample period acceleration values
            last_accel[0] = pdata->world_linear_accel[0];
            last_accel[1] = pdata->world_linear_accel[1];
            last_accel[2] = pdata->world_linear_accel[2];

            /* Rotate the Quaternion from the MPU by the inverse of the current yaw offset */

            Quaternion q_in( (((float)(pdata->quaternion[0]))/65536) / 16384.0f,
            		(((float)(pdata->quaternion[1]))/65536) / 16384.0f,
            		(((float)(pdata->quaternion[2]))/65536) / 16384.0f,
            		(((float)(pdata->quaternion[3]))/65536) / 16384.0f);

            Quaternion world_frame_rotation( cos(-curr_yaw_offset_radians/2), 0.0f, 0.0f, -sin(-curr_yaw_offset_radians/2) );

            Quaternion q = world_frame_rotation.getProduct(q_in);

            pdata->quaternion[0] = ((long)(q.w * 16384.0f)) << 16;
            pdata->quaternion[1] = ((long)(q.x * 16384.0f)) << 16;
            pdata->quaternion[2] = ((long)(q.y * 16384.0f)) << 16;
            pdata->quaternion[3] = ((long)(q.z * 16384.0f)) << 16;

            getGravity(&gravity, &q);
            dmpGetYawPitchRoll(ypr, &q, &gravity);

            pdata->yaw = ypr[0] * radians_to_degrees;
            pdata->pitch = ypr[1] * radians_to_degrees;
            pdata->roll = ypr[2] * radians_to_degrees;

            float linear_accel[3];

            linear_accel[0] = (((float)accel_short[0]) / (32768.0 / accel_fsr)) - gravity.x;
            linear_accel[1] = (((float)accel_short[1]) / (32768.0 / accel_fsr)) - gravity.y;
            linear_accel[2] = (((float)accel_short[2]) / (32768.0 / accel_fsr)) - gravity.z;

            float q1[4];
            float q2[4];
            float q_product[4];
            float q_conjugate[4];
            float q_final[4];

            // Calculate world-frame acceleration

            q1[0] = q.w;
            q1[1] = q.x;
            q1[2] = q.y;
            q1[3] = q.z;

            q2[0] = 0;
            q2[1] = linear_accel[0];
            q2[2] = linear_accel[1];
            q2[3] = linear_accel[2];

            // Rotate linear acceleration so that it's relative to the world reference frame

            // http://www.cprogramming.com/tutorial/3d/quaternions.html
            // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
            // http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
            // ^ or: http://webcache.googleusercontent.com/search?q=cache:xgJAp3bDNhQJ:content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation&hl=en&gl=us&strip=1

            // P_out = q * P_in * conj(q)
            // - P_out is the output vector
            // - q is the orientation quaternion
            // - P_in is the input vector (a*aReal)
            // - conj(q) is the conjugate of the orientation quaternion (q=[w,x,y,z], q*=[w,-x,-y,-z])

            // calculate quaternion product
            // Quaternion multiplication is defined by:
            //     (Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
            //     (Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
            //     (Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
            //     (Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2

            q_product[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];  // new w
            q_product[1] = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2];  // new x
            q_product[2] = q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1];  // new y
            q_product[3] = q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0];  // new z

            q_conjugate[0] = q1[0];
            q_conjugate[1] = -q1[1];
            q_conjugate[2] = -q1[2];
            q_conjugate[3] = -q1[3];

            q_final[0] = q_product[0]*q_conjugate[0] - q_product[1]*q_conjugate[1] - q_product[2]*q_conjugate[2] - q_product[3]*q_conjugate[3];  // new w
            q_final[1] = q_product[0]*q_conjugate[1] + q_product[1]*q_conjugate[0] + q_product[2]*q_conjugate[3] - q_product[3]*q_conjugate[2];  // new x
            q_final[2] = q_product[0]*q_conjugate[2] - q_product[1]*q_conjugate[3] + q_product[2]*q_conjugate[0] + q_product[3]*q_conjugate[1];  // new y
            q_final[3] = q_product[0]*q_conjugate[3] + q_product[1]*q_conjugate[2] - q_product[2]*q_conjugate[1] + q_product[3]*q_conjugate[0];  // new z

            /* At this point, linear acceleration is in world frame */

            pdata->world_linear_accel[0] = q_final[1];
            pdata->world_linear_accel[1] = q_final[2];
            pdata->world_linear_accel[2] = q_final[3];

            float world_linear_accel_norm[2];

            for ( int i = 0; i < 2; i++ ) {
                world_linear_accel_norm[i] = fabs (pdata->world_linear_accel[i]);
                if ( non_moving_sample_count[i] != 0 ) {
                    if ( world_linear_accel_norm[i] > motion_detection_threshold_g ) {
                        non_moving_sample_count[i] = 0;
                    } else {
                        non_moving_sample_count[i]++;
                    }
                } else {
                    if ( world_linear_accel_norm[i] > (motion_detection_threshold_g * 10) ) {
                        non_moving_sample_count[i] = 0;
                    } else {
                        non_moving_sample_count[i]++;
                    }
                }
            }

            for ( int i = 0; i < 2; i++ ) {
                if ( non_moving_sample_count[i] > 2 ) {
                    /* Reset velocity integration */
                    pdata->velocity[i] = 0.0f;
                } else {
                    /* Integrate velocity and displacement */
                    float curr_accel_m_s2 = pdata->world_linear_accel[i] * 9.80665f;
                    float last_accel_m_s2 = last_accel[i] * 9.80665f;
                    /* Perform trapezoidal integration, to reduce sampling losses. */
                    /* Vf = Vi * a*t -- adding trapezoidal integration:  Vf = Vi + ((last_a + ((curr_a - last_a)/2)) * t) */
                    float curr_velocity_m_s = pdata->velocity[i] + ((last_accel_m_s2 + ((curr_accel_m_s2 - last_accel_m_s2)/2.0)) * curr_sample_period);
                    /* d = (Vi + Vf)/2) * t -- adding trapezoidal integration:  d = (Vi + ((Vf-Vi)/2) * t */
                    pdata->displacement[i] += (((pdata->velocity[i] + (curr_velocity_m_s - pdata->velocity[i]))/2.0) * curr_sample_period);
                    pdata->velocity[i] = curr_velocity_m_s;
                }
            }

            float mag_field[3];

            /* Apply compass calibration biases */
            mag_field[0] = pdata->raw_compass[0] - magcaldata.bias[0];
            mag_field[1] = pdata->raw_compass[1] - magcaldata.bias[1];
            mag_field[2] = pdata->raw_compass[2] - magcaldata.bias[2];

            /* Determine whether the current magnetometer reading differs the norm of the earth's */
            /* magnetic field by more than the disturbance ratio.  In this case, a magnetic       */
            /* anomaly is declared.                                                               */
            float mag_field_norm = (float)sqrt( ( mag_field[0] * mag_field[0] ) +
                    ( mag_field[1] * mag_field[1] ) +
                    ( mag_field[2] * mag_field[2] ) );

            float mag_field_difference_from_norm = fabs(mag_field_norm - magcaldata.earth_mag_field_norm );
            pdata->ratio_of_mag_field_norm = mag_field_difference_from_norm / fabs(magcaldata.earth_mag_field_norm);
            if ( pdata->ratio_of_mag_field_norm > magcaldata.mag_disturbance_ratio ) {
                pdata->magnetic_anomaly_detected = true;
            } else {
                pdata->magnetic_anomaly_detected = false;
            }

            /* Correct for soft iron distortion with the calibration matrix */
            pdata->calibrated_compass[0] = 0.0;
            pdata->calibrated_compass[1] = 0.0;
            pdata->calibrated_compass[2] = 0.0;
            for (int i=0; i<3; ++i)
                for (int j=0; j<3; ++j)
                    pdata->calibrated_compass[i] += (magcaldata.xform[i][j] * mag_field[j]);

            /* Stabilize the radius of the sphere by multiplying by the ratio from the norm. */
            pdata->mag_norm_scalar = magcaldata.earth_mag_field_norm/mag_field_norm;
            //apply the current scaler to the calibrated coordinates (global array calibrated_values)
            pdata->calibrated_compass[0] *= pdata->mag_norm_scalar;
            pdata->calibrated_compass[1] *= pdata->mag_norm_scalar;
            pdata->calibrated_compass[2] *= pdata->mag_norm_scalar;

            /* Perform tilt compensation, based upon pitch/roll        */
            /* Note that roll (roation about Y Axis) is inverted here, */
            /* since the sign of the Y axis is inverted within the     */
            /* Quaternion calculated by the MPU                        */

            float inverted_roll = -ypr[2];
            float pitch = ypr[1];

            float cos_roll = cos(inverted_roll);
            float sin_roll = sin(inverted_roll);
            float cos_pitch = cos(pitch);
            float sin_pitch = sin(pitch);

            /* Transform compass data to board orientation */
            xform_mpu_to_board_coordinates(pdata->calibrated_compass);

            /* The following tilt compensation algorithm was derived from      */
            /* ST Micro’s AN3192, with the exception that the Roll and Pitch   */
            /* Terms are swapped due to a difference in the X/Y axis reference */
            /* frame used by ST and that used by the navX-Micro.               */
            /* http://solutions-cubed.com/app-notes-downloads/#BM004           */

            float MAG_X = pdata->calibrated_compass[0] * cos_roll + pdata->calibrated_compass[2] * sin_roll;
            float MAG_Y = pdata->calibrated_compass[0] * sin_pitch * sin_roll +
                    pdata->calibrated_compass[1] * cos_pitch -
                    pdata->calibrated_compass[2] * sin_pitch * cos_roll;
            float tilt_compensated_heading_radians = atan2(MAG_Y,MAG_X);
            pdata->heading = tilt_compensated_heading_radians * radians_to_degrees;

            /* If new compass heading differs by more than the yaw delta since the last update, */
            /* (adding in a margin for the understood compass noise bandwidth), then declare a  */
            /* Magnetic anomaly.                                                                */

            bool interpolate_fused_heading = true;
            float latest_gyro_interpolated_heading = last_valid_fused_heading +
                    (pdata->yaw - yaw_at_last_valid_fused_heading);
            /* Normalize to 0-360 degree range */
            if ( latest_gyro_interpolated_heading < 0 ) {
                latest_gyro_interpolated_heading += 360.0f;
            } else if ( latest_gyro_interpolated_heading > 360.0f ) {
                latest_gyro_interpolated_heading -= 360.0f;
            }
            if ( new_compass ) {
                float compass_delta = fabs(pdata->heading - last_new_compass_heading);
                float yaw_delta = fabs(pdata->yaw - yaw_at_last_new_compass_heading);
                if ( first_valid_fused_heading_detected &&
                        ( compass_delta > (yaw_delta + compass_heading_noise_band_degrees) ) ) {
                    pdata->magnetic_anomaly_detected = true;
                }

                /* Interpolate the fused heading, deriving it from the compass and gyro    */
                /* data, ignoring the compass heading when magnetic anomalies are detected */
                /* and interpolating values in-between compass heading updates.            */
                /* TODO:  Ignore updates from yaw until gyro has been calibrated?          */
                /* Only declare new compass readings valid if rotation less than the       */
                /* compass noise band has occurred.                                        */
                /* Then, fuse latest gyro-derived heading with compass heading using a     */
                /* complementary filter.                                                   */

                if ( ( !pdata->magnetic_anomaly_detected ) &&
                        ( yaw_delta < compass_heading_noise_band_degrees ) ) {
                    valid_fused_heading_count++;
                    if ( valid_fused_heading_count >= 3 ) {
                        if ( !first_valid_fused_heading_detected ) {
                            first_valid_fused_heading_detected = true;
                            pdata->fused_heading = pdata->heading;
                        } else {
#define FUSED_HEADING_COMPLEMENTARY_FILTER_TAU 0.20f
                            float continuous_compass_heading = pdata->heading;
                            if ( continuous_compass_heading < 0.0f ) {
                                continuous_compass_heading += 360.0f;
                            }
                            /* Complementary filter gyro and compass data */
                            pdata->fused_heading = (latest_gyro_interpolated_heading * (1.0f-FUSED_HEADING_COMPLEMENTARY_FILTER_TAU)) +
                                    (continuous_compass_heading * (FUSED_HEADING_COMPLEMENTARY_FILTER_TAU));
                        }
                        yaw_at_last_valid_fused_heading = pdata->yaw;
                        last_valid_fused_heading = pdata->fused_heading;
                        interpolate_fused_heading = false;
                    }
                } else {
                    valid_fused_heading_count = 0;
                }
                new_compass = 0;
                yaw_at_last_new_compass_heading = pdata->yaw;
                last_new_compass_heading = pdata->heading;
            }

            if ( interpolate_fused_heading ) {
                /* Interpolate fused heading based upon change in gyro yaw */
                pdata->fused_heading = latest_gyro_interpolated_heading;
            }

            /* Modify compass heading ranges from -180-180 to 0-360 degrees */
            if ( pdata->heading < 0 ) {
                pdata->heading += 360;
            }
            /* Fused heading possible ranges are 0 (compass) + -180 [-180] */
            /* to 360 + 180 (540).  Modify the range to be 0-360 degrees.         */
            if ( pdata->fused_heading < 0 ) {
                pdata->fused_heading += 360;
            } else if ( pdata->fused_heading > 360 ) {
                pdata->fused_heading -= 360;
            }
            pdata->fused_heading_valid = first_valid_fused_heading_detected;

            pdata->calibrated_gyro_x = gyro[0];
            pdata->calibrated_gyro_y = gyro[1];
            pdata->calibrated_gyro_z = gyro[2];

            pdata->raw_gyro[0] = gyro[0];
            pdata->raw_gyro[1] = gyro[1];
            pdata->raw_gyro[2] = gyro[2];

            pdata->raw_accel[0] = accel_short[0];
            pdata->raw_accel[1] = accel_short[1];
            pdata->raw_accel[2] = accel_short[2];

            pdata->timestamp = sensor_timestamp;
            pdata->hires_timestamp = hires_timestamp;
        }
        return 0;
    } else {
        /* Error communicating with MPU.  Clear the new data received flag. */
        hal.new_gyro = 0;
        return -1;
    }
}

/* The following #defines were copied from inv_dmp_motion_driver.c */
/* This is a hack, but allows ensure we dont' have to modify the   */
/* source code of the Invensense driver.                           */

#define D_EXT_GYRO_BIAS_X       (61 * 16)			// 12 bytes
#define D_EXT_GYRO_BIAS_Y       (61 * 16) + 4
#define D_EXT_GYRO_BIAS_Z       (61 * 16) + 8

uint8_t last_dmp_gyro_bias[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

union GYRO_BIAS {
    uint8_t     dmp_gyro_bias_bytes[12];
    uint32_t    dmp_gyro_bias_big_endian[3];
};

_EXTERN_ATTRIB int mpu_did_dmp_gyro_biases_change(struct mpu_dmp_calibration_data *dmpcaldata) {
    GYRO_BIAS curr_gyro_bias;
    int gyro_bias_cmp;
    int changed = 0;
    if (!mpu_read_mem(D_EXT_GYRO_BIAS_X,12,curr_gyro_bias.dmp_gyro_bias_bytes)) {
		gyro_bias_cmp = memcmp(curr_gyro_bias.dmp_gyro_bias_bytes,last_dmp_gyro_bias,12);
		if ( gyro_bias_cmp != 0 ) {
			changed = 1;
			memcpy(last_dmp_gyro_bias,curr_gyro_bias.dmp_gyro_bias_bytes,12);
			for ( int i = 0; i < 3; i++ ) {
				/* Convert from big-endian to little-endian */
				uint32_t big_endian_bias = curr_gyro_bias.dmp_gyro_bias_big_endian[i];
				dmpcaldata->gyro_bias_q16[i] = (int32_t)__REV(big_endian_bias);
			}
			dmpcaldata->mpu_temp_c = last_temperature / 65536.0;
		}
    }
    return changed;
}

_EXTERN_ATTRIB int mpu_apply_dmp_gyro_biases(struct mpu_dmp_calibration_data *dmpcaldata) {
    GYRO_BIAS new_gyro_bias;
    for ( int i = 0; i < 3; i++ ) {
        /* Convert from little-endian to big-endian */
        new_gyro_bias.dmp_gyro_bias_big_endian[i] = __REV((uint32_t)dmpcaldata->gyro_bias_q16[i]);
    }
    mpu_write_mem(D_EXT_GYRO_BIAS_X,12,new_gyro_bias.dmp_gyro_bias_bytes);
    return 0;
}

_EXTERN_ATTRIB int mpu_set_new_sample_rate( uint8_t new_sample_rate ) {
    update_curr_sample_rate(new_sample_rate);
#if 0
    long int mpl_rate_us = 1000000L / new_sample_rate;
#endif
    if (hal.dmp_on) {
        dmp_set_fifo_rate(new_sample_rate);
#if 0
        inv_set_quat_sample_rate(mpl_rate_us);
#endif
    } else {
        mpu_set_sample_rate(new_sample_rate);
    }
#if 0
    inv_set_gyro_sample_rate(mpl_rate_us);
    inv_set_accel_sample_rate(mpl_rate_us);
#endif
    return 0;
}

_EXTERN_ATTRIB bool mpu_detect()
{
    uint8_t address = 0x68;
    return ( HAL_I2C_IsDeviceReady(&hi2c2, address<<1, 3, 100) == HAL_OK );
}

/* Reset integrators for velocity and displacement.  Each of the lower 6 bits of the */
/* "axes to reset" variable corresponds to a integration variable.                   */

_EXTERN_ATTRIB void reset_velocity_and_dispacement_integrator( struct mpu_data *pdata,
        uint8_t axes_to_reset )
{
    if ( axes_to_reset & 0x01 ) {
        pdata->velocity[0] = 0.0f;
    }
    if ( axes_to_reset & 0x02 ) {
        pdata->velocity[1] = 0.0f;
    }
    if ( axes_to_reset & 0x04 ) {
        pdata->velocity[2] = 0.0f;
    }
    if ( axes_to_reset & 0x08 ) {
        pdata->displacement[0] = 0.0f;
    }
    if ( axes_to_reset & 0x10 ) {
        pdata->displacement[1] = 0.0f;
    }
    if ( axes_to_reset & 0x20 ) {
        pdata->displacement[2] = 0.0f;
    }
}

_EXTERN_ATTRIB void set_motion_detection_threshold_g( float threshold_g )
{
    motion_detection_threshold_g = threshold_g;
}

_EXTERN_ATTRIB void set_yaw_offset( float yaw_offset_angle_degrees )
{
    if ( yaw_offset_angle_degrees < 0.0f ) {
        yaw_offset_angle_degrees += 360.0f;
    }
    curr_yaw_offset_radians = yaw_offset_angle_degrees * degrees_to_radians;
}

_EXTERN_ATTRIB void get_yaw_offset( float *yaw_offset_angle_degrees )
{
    float curr_yaw_offset = curr_yaw_offset_radians * radians_to_degrees;
    if ( curr_yaw_offset >= 180.0f ) {
        curr_yaw_offset -= 360.0f;
    }
    *yaw_offset_angle_degrees = curr_yaw_offset;
}
