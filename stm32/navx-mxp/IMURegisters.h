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

#ifndef IMU_REGISTERS_H_
#define IMU_REGISTERS_H_

#include "IMUProtocol.h"

/*******************************************************************/
/*******************************************************************/
/*                      Register Definitions                       */
/*******************************************************************/
/* NOTE:  All multi-byte registers are in little-endian format.    */
/*        All registers with 'signed' data are twos-complement.    */
/*        Data Type Summary:                                       */
/*        unsigned byte:           0   - 255    (8 bits)           */
/*        unsigned short:          0   - 65535  (16 bits)          */
/*        signed short:        -32768  - 32767  (16 bits)          */
/*        signed hundredeths:  -327.68 - 327.67 (16 bits)		   */
/*        unsigned hundredths:    0.0  - 655.35 (16 bits)          */
/*        signed thousandths:  -32.768 - 32.767 (16 bits)          */
/*        signed short ratio: -1/16384 - 1/16384 (16 bits)         */
/*        16:16:           -32768.9999 - 32767.9999 (32 bits)      */
/*        unsigned long:             0 - 4294967295 (32 bits)      */
/*******************************************************************/

typedef int16_t		s_short_hundred_float;
typedef uint16_t	u_short_hundred_float;
typedef int16_t     s_short_thousand_float;
typedef int16_t     s_short_ratio_float;
typedef int32_t     s_1616_float;

/**********************************************/
/* Device Identification Registers            */
/**********************************************/

#define NAVX_REG_WHOAMI             0x00 /* IMU_MODEL_XXX */
#define NAVX_REG_HW_REV             0x01
#define NAVX_REG_FW_VER_MAJOR       0x02
#define NAVX_REG_FW_VER_MINOR       0x03

/**********************************************/
/* Status and Control Registers               */
/**********************************************/

/* Read-write */
#define NAVX_REG_UPDATE_RATE_HZ     0x04 /* Range:  4 - 50 [unsigned byte] */
/* Read-only */
/* Accelerometer Full-Scale Range:  in units of G [unsigned byte] */
#define NAVX_REG_ACCEL_FSR_G        0x05
/* Gyro Full-Scale Range (Degrees/Sec):  Range:  250, 500, 1000 or 2000 [unsigned short] */
#define NAVX_REG_GYRO_FSR_DPS_L     0x06 /* Lower 8-bits of Gyro Full-Scale Range */
#define NAVX_REG_GYRO_FSR_DPS_H     0x07 /* Upper 8-bits of Gyro Full-Scale Range */
#define NAVX_REG_OP_STATUS          0x08 /* NAVX_OP_STATUS_XXX */
#define NAVX_REG_CAL_STATUS         0x09 /* NAVX_CAL_STATUS_XXX */
#define NAVX_REG_SELFTEST_STATUS    0x0A /* NAVX_SELFTEST_STATUS_XXX */
#define NAVX_REG_CAPABILITY_FLAGS_L 0x0B
#define NAVX_REG_CAPABILITY_FLAGS_H 0x0C

/**********************************************/
/* Processed Data Registers                   */
/**********************************************/

#define NAVX_REG_SENSOR_STATUS_L    0x10 /* NAVX_SENSOR_STATUS_XXX */
#define NAVX_REG_SENSOR_STATUS_H    0x11
/* Timestamp:  [unsigned long] */
#define NAVX_REG_TIMESTAMP_L_L      0x12
#define NAVX_REG_TIMESTAMP_L_H      0x13
#define NAVX_REG_TIMESTAMP_H_L      0x14
#define NAVX_REG_TIMESTAMP_H_H      0x15

/* Yaw, Pitch, Roll:  Range: -180.00 to 180.00 [signed hundredths] */
/* Compass Heading:   Range: 0.00 to 360.00 [unsigned hundredths] */
/* Altitude in Meters:  In units of meters [16:16] */

#define NAVX_REG_YAW_L              0x16 /* Lower 8 bits of Yaw     */
#define NAVX_REG_YAW_H              0x17 /* Upper 8 bits of Yaw     */
#define NAVX_REG_ROLL_L             0x18 /* Lower 8 bits of Roll    */
#define NAVX_REG_ROLL_H             0x19 /* Upper 8 bits of Roll    */
#define NAVX_REG_PITCH_L            0x1A /* Lower 8 bits of Pitch   */
#define NAVX_REG_PITCH_H            0x1B /* Upper 8 bits of Pitch   */
#define NAVX_REG_HEADING_L          0x1C /* Lower 8 bits of Heading */
#define NAVX_REG_HEADING_H          0x1D /* Upper 8 bits of Heading */
#define NAVX_REG_FUSED_HEADING_L    0x1E /* Upper 8 bits of Fused Heading */
#define NAVX_REG_FUSED_HEADING_H    0x1F /* Upper 8 bits of Fused Heading */
#define NAVX_REG_ALTITUDE_I_L       0x20
#define NAVX_REG_ALTITUDE_I_H       0x21
#define NAVX_REG_ALTITUDE_D_L       0x22
#define NAVX_REG_ALTITUDE_D_H       0x23

/* World-frame Linear Acceleration: In units of +/- G * 1000 [signed thousandths] */

#define NAVX_REG_LINEAR_ACC_X_L     0x24 /* Lower 8 bits of Linear Acceleration X */
#define NAVX_REG_LINEAR_ACC_X_H     0x25 /* Upper 8 bits of Linear Acceleration X */
#define NAVX_REG_LINEAR_ACC_Y_L     0x26 /* Lower 8 bits of Linear Acceleration Y */
#define NAVX_REG_LINEAR_ACC_Y_H     0x27 /* Upper 8 bits of Linear Acceleration Y */
#define NAVX_REG_LINEAR_ACC_Z_L     0x28 /* Lower 8 bits of Linear Acceleration Z */
#define NAVX_REG_LINEAR_ACC_Z_H     0x29 /* Upper 8 bits of Linear Acceleration Z */

/* Quaternion:  Range -1 to 1 [signed short ratio] */

#define NAVX_REG_QUAT_W_L           0x2A /* Lower 8 bits of Quaternion W */
#define NAVX_REG_QUAT_W_H           0x2B /* Upper 8 bits of Quaternion W */
#define NAVX_REG_QUAT_X_L           0x2C /* Lower 8 bits of Quaternion X */
#define NAVX_REG_QUAT_X_H           0x2D /* Upper 8 bits of Quaternion X */
#define NAVX_REG_QUAT_Y_L           0x2E /* Lower 8 bits of Quaternion Y */
#define NAVX_REG_QUAT_Y_H           0x2F /* Upper 8 bits of Quaternion Y */
#define NAVX_REG_QUAT_Z_L           0x30 /* Lower 8 bits of Quaternion Z */
#define NAVX_REG_QUAT_Z_H           0x31 /* Upper 8 bits of Quaternion Z */

/**********************************************/
/* Raw Data Registers                         */
/**********************************************/

/* Sensor Die Temperature:  Range +/- 150, In units of Centigrade * 100 [signed hundredths float */

#define NAVX_REG_MPU_TEMP_C_L       0x32 /* Lower 8 bits of Temperature */
#define NAVX_REG_MPU_TEMP_C_H       0x33 /* Upper 8 bits of Temperature */

/* Raw, Calibrated Angular Rotation, in device units.  Value in DPS = units / GYRO_FSR_DPS [signed short] */

#define NAVX_REG_GYRO_X_L           0x34
#define NAVX_REG_GYRO_X_H           0x35
#define NAVX_REG_GYRO_Y_L           0x36
#define NAVX_REG_GYRO_Y_H           0x37
#define NAVX_REG_GYRO_Z_L           0x38
#define NAVX_REG_GYRO_Z_H           0x39

/* Raw, Calibrated, Acceleration Data, in device units.  Value in G = units / ACCEL_FSR_G [signed short] */

#define NAVX_REG_ACC_X_L            0x3A
#define NAVX_REG_ACC_X_H            0x3B
#define NAVX_REG_ACC_Y_L            0x3C
#define NAVX_REG_ACC_Y_H            0x3D
#define NAVX_REG_ACC_Z_L            0x3E
#define NAVX_REG_ACC_Z_H            0x3F

/* Raw, Calibrated, Un-tilt corrected Magnetometer Data, in device units.  1 unit = 0.15 uTesla [signed short] */

#define NAVX_REG_MAG_X_L            0x40
#define NAVX_REG_MAG_X_H            0x41
#define NAVX_REG_MAG_Y_L            0x42
#define NAVX_REG_MAG_Y_H            0x43
#define NAVX_REG_MAG_Z_L            0x44
#define NAVX_REG_MAG_Z_H            0x45

/* Calibrated Pressure in millibars Valid Range:  10.00 Max:  1200.00 [16:16 float]  */

#define NAVX_REG_PRESSURE_IL        0x46
#define NAVX_REG_PRESSURE_IH        0x47
#define NAVX_REG_PRESSURE_DL        0x48
#define NAVX_REG_PRESSURE_DH        0x49

/* Pressure Sensor Die Temperature:  Range +/- 150.00C [signed hundredths] */

#define NAVX_REG_PRESSURE_TEMP_L    0x4A
#define NAVX_REG_PRESSURE_TEMP_H    0x4B

/**********************************************/
/* Calibration Registers                      */
/**********************************************/

/* Yaw Offset: Range -180.00 to 180.00 [signed hundredths] */

#define NAVX_REG_YAW_OFFSET_L       0x4C /* Lower 8 bits of Yaw Offset */
#define NAVX_REG_YAW_OFFSET_H       0x4D /* Upper 8 bits of Yaw Offset */

/* Quaternion Offset:  Range: -1 to 1 [signed short ratio]  */

#define NAVX_REG_QUAT_OFFSET_W_L    0x4E /* Lower 8 bits of Quaternion W */
#define NAVX_REG_QUAT_OFFSET_W_H    0x4F /* Upper 8 bits of Quaternion W */
#define NAVX_REG_QUAT_OFFSET_X_L    0x50 /* Lower 8 bits of Quaternion X */
#define NAVX_REG_QUAT_OFFSET_X_H    0x51 /* Upper 8 bits of Quaternion X */
#define NAVX_REG_QUAT_OFFSET_Y_L    0x52 /* Lower 8 bits of Quaternion Y */
#define NAVX_REG_QUAT_OFFSET_Y_H    0x53 /* Upper 8 bits of Quaternion Y */
#define NAVX_REG_QUAT_OFFSET_Z_L    0x54 /* Lower 8 bits of Quaternion Z */
#define NAVX_REG_QUAT_OFFSET_Z_H    0x55 /* Upper 8 bits of Quaternion Z */

/**********************************************/
/* Integrated Data Registers                  */
/**********************************************/

/* Integration Control (Write-Only)           */
#define NAVX_REG_INTEGRATION_CTL    0x56
#define NAVX_REG_PAD_UNUSED         0x57

/* Velocity:  Range -32768.9999 - 32767.9999 in units of Meters/Sec      */

#define NAVX_REG_VEL_X_I_L          0x58
#define NAVX_REG_VEL_X_I_H          0x59
#define NAVX_REG_VEL_X_D_L          0x5A
#define NAVX_REG_VEL_X_D_H          0x5B
#define NAVX_REG_VEL_Y_I_L          0x5C
#define NAVX_REG_VEL_Y_I_H          0x5D
#define NAVX_REG_VEL_Y_D_L          0x5E
#define NAVX_REG_VEL_Y_D_H          0x5F
#define NAVX_REG_VEL_Z_I_L          0x60
#define NAVX_REG_VEL_Z_I_H          0x61
#define NAVX_REG_VEL_Z_D_L          0x62
#define NAVX_REG_VEL_Z_D_H          0x63

/* Displacement:  Range -32768.9999 - 32767.9999 in units of Meters      */

#define NAVX_REG_DISP_X_I_L         0x64
#define NAVX_REG_DISP_X_I_H         0x65
#define NAVX_REG_DISP_X_D_L         0x66
#define NAVX_REG_DISP_X_D_H         0x67
#define NAVX_REG_DISP_Y_I_L         0x68
#define NAVX_REG_DISP_Y_I_H         0x69
#define NAVX_REG_DISP_Y_D_L         0x6A
#define NAVX_REG_DISP_Y_D_H         0x6B
#define NAVX_REG_DISP_Z_I_L         0x6C
#define NAVX_REG_DISP_Z_I_H         0x6D
#define NAVX_REG_DISP_Z_D_L         0x6E
#define NAVX_REG_DISP_Z_D_H         0x6F

#define NAVX_REG_LAST NAVX_REG_DISP_Z_D_H

/* NAVX_MODEL */

#define NAVX_MODEL_NAVX_MXP                         0x32

/* NAVX_CAL_STATUS */

#define NAVX_CAL_STATUS_IMU_CAL_STATE_MASK          0x03
#define NAVX_CAL_STATUS_IMU_CAL_INPROGRESS          0x00
#define NAVX_CAL_STATUS_IMU_CAL_ACCUMULATE          0x01
#define NAVX_CAL_STATUS_IMU_CAL_COMPLETE            0x02

#define NAVX_CAL_STATUS_MAG_CAL_COMPLETE            0x04
#define NAVX_CAL_STATUS_BARO_CAL_COMPLETE           0x08

/* NAVX_SELFTEST_STATUS */

#define NAVX_SELFTEST_STATUS_COMPLETE               0x80

#define NAVX_SELFTEST_RESULT_GYRO_PASSED            0x01
#define NAVX_SELFTEST_RESULT_ACCEL_PASSED           0x02
#define NAVX_SELFTEST_RESULT_MAG_PASSED             0x04
#define NAVX_SELFTEST_RESULT_BARO_PASSED            0x08

/* NAVX_OP_STATUS */

#define NAVX_OP_STATUS_INITIALIZING                 0x00
#define NAVX_OP_STATUS_SELFTEST_IN_PROGRESS       	0x01
#define NAVX_OP_STATUS_ERROR                        0x02 /* E.g., Self-test fail, I2C error */
#define NAVX_OP_STATUS_IMU_AUTOCAL_IN_PROGRESS      0x03
#define NAVX_OP_STATUS_NORMAL                       0x04

/* NAVX_SENSOR_STATUS */

#define NAVX_SENSOR_STATUS_MOVING                   0x01
#define NAVX_SENSOR_STATUS_YAW_STABLE               0x02
#define NAVX_SENSOR_STATUS_MAG_DISTURBANCE          0x04
#define NAVX_SENSOR_STATUS_ALTITUDE_VALID           0x08
#define NAVX_SENSOR_STATUS_SEALEVEL_PRESS_SET       0x10
#define NAVX_SENSOR_STATUS_FUSED_HEADING_VALID      0x20

/* NAVX_REG_CAPABILITY_FLAGS (Aligned w/NAV6 Flags, see IMUProtocol.h) */

#define NAVX_CAPABILITY_FLAG_OMNIMOUNT              0x0004
#define NAVX_CAPABILITY_FLAG_OMNIMOUNT_CONFIG_MASK  0x0038
#define NAVX_CAPABILITY_FLAG_VEL_AND_DISP           0x0040
#define NAVX_CAPABILITY_FLAG_YAW_RESET              0x0080
#define NAVX_CAPABILITY_FLAG_AHRSPOS_TS				0x0100

/* NAVX_OMNIMOUNT_CONFIG */

#define OMNIMOUNT_DEFAULT                           0 /* Same as Y_Z_UP */
#define OMNIMOUNT_YAW_X_UP                          1
#define OMNIMOUNT_YAW_X_DOWN                        2
#define OMNIMOUNT_YAW_Y_UP                          3
#define OMNIMOUNT_YAW_Y_DOWN                        4
#define OMNIMOUNT_YAW_Z_UP                          5
#define OMNIMOUNT_YAW_Z_DOWN                        6

/* NAVX_INTEGRATION_CTL */

#define NAVX_INTEGRATION_CTL_RESET_VEL_X            0x01
#define NAVX_INTEGRATION_CTL_RESET_VEL_Y            0x02
#define NAVX_INTEGRATION_CTL_RESET_VEL_Z            0x04
#define NAVX_INTEGRATION_CTL_RESET_DISP_X           0x08
#define NAVX_INTEGRATION_CTL_RESET_DISP_Y           0x10
#define NAVX_INTEGRATION_CTL_RESET_DISP_Z           0x20
#define NAVX_INTEGRATION_CTL_VEL_AND_DISP_MASK      0x3F

#define NAVX_INTEGRATION_CTL_RESET_YAW              0x80

class IMURegisters
{
public:
    /************************************************************/
    /* NOTE:                                                    */
    /* The following functions assume a little-endian processor */
    /************************************************************/

    static inline uint16_t decodeProtocolUint16( char *uint16_bytes ) {
        return *((uint16_t *)uint16_bytes);
    }
    static inline void encodeProtocolUint16( uint16_t val, char *uint16_bytes) {
        *((uint16_t *)uint16_bytes) = val;
    }

    static inline int16_t decodeProtocolInt16( char *int16_bytes ) {
        return *((int16_t *)int16_bytes);
    }
    static inline void encodeProtocolInt16( int16_t val, char *int16_bytes) {
        *((int16_t *)int16_bytes) = val;
    }
	
    static inline uint32_t decodeProtocolUint32( char *uint32_bytes ) {
        return *((uint32_t *)uint32_bytes);
    }	

    static inline int32_t decodeProtocolInt32( char *int32_bytes ) {
        return *((int32_t *)int32_bytes);
    }
    static inline void encodeProtocolInt32( int32_t val, char *int32_bytes) {
        *((int32_t *)int32_bytes) = val;
    }

    /* -327.68 to +327.68 */
    static inline float decodeProtocolSignedHundredthsFloat( char *uint8_signed_angle_bytes ) {
        float signed_angle = (float)decodeProtocolInt16(uint8_signed_angle_bytes);
        signed_angle /= 100;
        return signed_angle;
    }
    static inline void encodeProtocolSignedHundredthsFloat( float input, char *uint8_signed_hundredths_float) {
        int16_t input_as_int = (int16_t)(input * 100.0f);
        encodeProtocolInt16(input_as_int,uint8_signed_hundredths_float);
    }

    static inline s_short_hundred_float encodeSignedHundredthsFloat( float input ) {
        return (s_short_hundred_float)(input * 100.0f);
    }
    static inline u_short_hundred_float encodeUnsignedHundredthsFloat(float input ) {
        return (u_short_hundred_float)(input * 100.0f);
    }

    static inline s_short_ratio_float encodeRatioFloat(float input_ratio) {
        return (s_short_hundred_float)(input_ratio *= 32768.0f);
    }
    static inline s_short_thousand_float encodeSignedThousandthsFloat(float input) {
        return (s_short_thousand_float)(input * 1000.0f);
    }

    /* 0 to 655.35 */
    static inline float decodeProtocolUnsignedHundredthsFloat( char *uint8_unsigned_hundredths_float ) {
        float unsigned_float = (float)decodeProtocolUint16(uint8_unsigned_hundredths_float);
        unsigned_float /= 100;
        return unsigned_float;
    }
    static inline void encodeProtocolUnsignedHundredthsFloat( float input, char *uint8_unsigned_hundredths_float) {
        uint16_t input_as_uint = (uint16_t)(input * 100.0f);
        encodeProtocolUint16(input_as_uint,uint8_unsigned_hundredths_float);
    }

    /* -32.768 to +32.768 */
    static inline float decodeProtocolSignedThousandthsFloat( char *uint8_signed_angle_bytes ) {
        float signed_angle = (float)decodeProtocolInt16(uint8_signed_angle_bytes);
        signed_angle /= 1000;
        return signed_angle;
    }
    static inline void encodeProtocolSignedThousandthsFloat( float input, char *uint8_signed_thousandths_float) {
        int16_t input_as_int = (int16_t)(input * 1000.0f);
        encodeProtocolInt16(input_as_int,uint8_signed_thousandths_float);
    }

    /* In units of -1 to 1, multiplied by 16384 */
    static inline float decodeProtocolRatio( char *uint8_ratio ) {
        float ratio = (float)decodeProtocolInt16(uint8_ratio);
        ratio /= 32768.0f;
        return ratio;
    }
    static inline void encodeProtocolRatio( float ratio, char *uint8_ratio ) {
        ratio *= 32768.0f;
        encodeProtocolInt16(ratio,uint8_ratio);
    }

    /* <int16>.<uint16> (-32768.9999 to 32767.9999) */
    static float decodeProtocol1616Float( char *uint8_16_16_bytes ) {
        float result = (float)decodeProtocolInt32( uint8_16_16_bytes );
        result /= 65536.0f;
        return result;
    }
    static void encodeProtocol1616Float( float val, char *uint8_16_16_bytes ) {
        val *= 65536.0f;
        int32_t packed_float = (int32_t)val;
        encodeProtocolInt32(packed_float, uint8_16_16_bytes);
    }

#define CRC7_POLY 0x91

    static void buildCRCLookupTable( uint8_t* table, size_t length )
    {
        size_t crc;
        size_t i, j;
        if ( length == 256 ) {
            for ( i = 0; i < length; i++ ) {
                crc = (uint8_t)i;
                for (j = 0; j < 8; j++) {
                    if (crc & 1) {
                        crc ^= CRC7_POLY;
                    }
                    crc >>= 1;
                }
                table[i] = crc;
            }
        }
    }

    static inline uint8_t getCRCWithTable( uint8_t* table, uint8_t message[], uint8_t length )
    {
        uint8_t i, crc = 0;

        for (i = 0; i < length; i++)
        {
            crc ^= message[i];
            crc = table[crc];
        }
        return crc;
    }

    static uint8_t getCRC(uint8_t message[], uint8_t length)
    {
        uint8_t i, j, crc = 0;

        for (i = 0; i < length; i++)
        {
            crc ^= message[i];
            for (j = 0; j < 8; j++)
            {
                if (crc & 1) {
                    crc ^= CRC7_POLY;
                }
                crc >>= 1;
            }
        }
        return crc;
    }
};


#endif /* IMU_REGISTERS_H_ */
