/*
 * AHRS.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include <sstream>
#include <string>
#include <iomanip>
#include <AHRS.h>
#include <AHRSProtocol.h>
#include "IIOProvider.h"
#include "IIOCompleteNotification.h"
#include "IBoardCapabilities.h"
#include "InertialDataIntegrator.h"
#include "OffsetTracker.h"
#include "ContinuousAngleTracker.h"
#include "RegisterIOSPI.h"
#include "RegisterIOI2C.h"
#include "SerialIO.h"

static const uint8_t    NAVX_DEFAULT_UPDATE_RATE_HZ         = 60;
static const int        YAW_HISTORY_LENGTH                  = 10;
static const int16_t    DEFAULT_ACCEL_FSR_G                 = 2;
static const int16_t    DEFAULT_GYRO_FSR_DPS                = 2000;
static const uint32_t   DEFAULT_SPI_BITRATE                 = 500000;
static const uint8_t    NAVX_MXP_I2C_ADDRESS                = 0x32;

class AHRSInternal : public IIOCompleteNotification, public IBoardCapabilities {
    AHRS *ahrs;
    friend class AHRS;
    AHRSInternal(AHRS* ahrs) {
        this->ahrs = ahrs;
    }

    virtual ~AHRSInternal() {}

    /***********************************************************/
    /* IIOCompleteNotification Interface Implementation        */
    /***********************************************************/

    void SetYawPitchRoll(IMUProtocol::YPRUpdate& ypr_update, long sensor_timestamp) {
        ahrs->yaw               	= ypr_update.yaw;
        ahrs->pitch             	= ypr_update.pitch;
        ahrs->roll              	= ypr_update.roll;
        ahrs->compass_heading   	= ypr_update.compass_heading;
        ahrs->last_sensor_timestamp	= sensor_timestamp;
    }

    void SetAHRSPosData(AHRSProtocol::AHRSPosUpdate& ahrs_update, long sensor_timestamp) {
        /* Update base IMU class variables */

        ahrs->yaw                    = ahrs_update.yaw;
        ahrs->pitch                  = ahrs_update.pitch;
        ahrs->roll                   = ahrs_update.roll;
        ahrs->compass_heading        = ahrs_update.compass_heading;
        ahrs->yaw_offset_tracker->UpdateHistory(ahrs_update.yaw);

        /* Update AHRS class variables */

        // 9-axis data
        ahrs->fused_heading          = ahrs_update.fused_heading;

        // Gravity-corrected linear acceleration (world-frame)
        ahrs->world_linear_accel_x   = ahrs_update.linear_accel_x;
        ahrs->world_linear_accel_y   = ahrs_update.linear_accel_y;
        ahrs->world_linear_accel_z   = ahrs_update.linear_accel_z;

        // Gyro/Accelerometer Die Temperature
        ahrs->mpu_temp_c             = ahrs_update.mpu_temp;

        // Barometric Pressure/Altitude
        ahrs->altitude               = ahrs_update.altitude;
        ahrs->baro_pressure          = ahrs_update.barometric_pressure;

        // Status/Motion Detection
        ahrs->is_moving              =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_MOVING) != 0)
                        ? true : false);
        ahrs->is_rotating                =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_YAW_STABLE) != 0)
                        ? false : true);
        ahrs->altitude_valid             =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_ALTITUDE_VALID) != 0)
                        ? true : false);
        ahrs->is_magnetometer_calibrated =
                (((ahrs_update.cal_status &
                        NAVX_CAL_STATUS_MAG_CAL_COMPLETE) != 0)
                        ? true : false);
        ahrs->magnetic_disturbance       =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_MAG_DISTURBANCE) != 0)
                        ? true : false);

        ahrs->quaternionW                = ahrs_update.quat_w;
        ahrs->quaternionX                = ahrs_update.quat_x;
        ahrs->quaternionY                = ahrs_update.quat_y;
        ahrs->quaternionZ                = ahrs_update.quat_z;

        ahrs->last_sensor_timestamp	= sensor_timestamp;

        /* Notify external data arrival subscribers, if any. */
        for (int i = 0; i < MAX_NUM_CALLBACKS; i++) {
            ITimestampedDataSubscriber *callback = ahrs->callbacks[i];
            if (callback != NULL) {
            	long system_timestamp = (long)(Timer::GetFPGATimestamp() * 1000);
                callback->timestampedDataReceived(system_timestamp,
                		sensor_timestamp,
                		ahrs_update,
                		ahrs->callback_contexts[i]);
            }
        }

        ahrs->velocity[0]     = ahrs_update.vel_x;
        ahrs->velocity[1]     = ahrs_update.vel_y;
        ahrs->velocity[2]     = ahrs_update.vel_z;
        ahrs->displacement[0] = ahrs_update.disp_x;
        ahrs->displacement[1] = ahrs_update.disp_y;
        ahrs->displacement[2] = ahrs_update.disp_z;

        ahrs->yaw_angle_tracker->NextAngle(ahrs->GetYaw());
        ahrs->last_sensor_timestamp	= sensor_timestamp;
    }

    void SetRawData(AHRSProtocol::GyroUpdate& raw_data_update, long sensor_timestamp) {
        ahrs->raw_gyro_x     = raw_data_update.gyro_x;
        ahrs->raw_gyro_y     = raw_data_update.gyro_y;
        ahrs->raw_gyro_z     = raw_data_update.gyro_z;
        ahrs->raw_accel_x    = raw_data_update.accel_x;
        ahrs->raw_accel_y    = raw_data_update.accel_y;
        ahrs->raw_accel_z    = raw_data_update.accel_z;
        ahrs->cal_mag_x      = raw_data_update.mag_x;
        ahrs->cal_mag_y      = raw_data_update.mag_y;
        ahrs->cal_mag_z      = raw_data_update.mag_z;
        ahrs->mpu_temp_c     = raw_data_update.temp_c;
        ahrs->last_sensor_timestamp	= sensor_timestamp;
    }

    void SetAHRSData(AHRSProtocol::AHRSUpdate& ahrs_update, long sensor_timestamp) {
        /* Update base IMU class variables */

        ahrs->yaw                    = ahrs_update.yaw;
        ahrs->pitch                  = ahrs_update.pitch;
        ahrs->roll                   = ahrs_update.roll;
        ahrs->compass_heading        = ahrs_update.compass_heading;
        ahrs->yaw_offset_tracker->UpdateHistory(ahrs_update.yaw);

        /* Update AHRS class variables */

        // 9-axis data
        ahrs->fused_heading          = ahrs_update.fused_heading;

        // Gravity-corrected linear acceleration (world-frame)
        ahrs->world_linear_accel_x   = ahrs_update.linear_accel_x;
        ahrs->world_linear_accel_y   = ahrs_update.linear_accel_y;
        ahrs->world_linear_accel_z   = ahrs_update.linear_accel_z;

        // Gyro/Accelerometer Die Temperature
        ahrs->mpu_temp_c             = ahrs_update.mpu_temp;

        // Barometric Pressure/Altitude
        ahrs->altitude               = ahrs_update.altitude;
        ahrs->baro_pressure          = ahrs_update.barometric_pressure;

        // Magnetometer Data
        ahrs->cal_mag_x              = ahrs_update.cal_mag_x;
        ahrs->cal_mag_y              = ahrs_update.cal_mag_y;
        ahrs->cal_mag_z              = ahrs_update.cal_mag_z;

        // Status/Motion Detection
        ahrs->is_moving              =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_MOVING) != 0)
                        ? true : false);
        ahrs->is_rotating                =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_YAW_STABLE) != 0)
                        ? false : true);
        ahrs->altitude_valid             =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_ALTITUDE_VALID) != 0)
                        ? true : false);
        ahrs->is_magnetometer_calibrated =
                (((ahrs_update.cal_status &
                        NAVX_CAL_STATUS_MAG_CAL_COMPLETE) != 0)
                        ? true : false);
        ahrs->magnetic_disturbance       =
                (((ahrs_update.sensor_status &
                        NAVX_SENSOR_STATUS_MAG_DISTURBANCE) != 0)
                        ? true : false);

        ahrs->quaternionW                = ahrs_update.quat_w;
        ahrs->quaternionX                = ahrs_update.quat_x;
        ahrs->quaternionY                = ahrs_update.quat_y;
        ahrs->quaternionZ                = ahrs_update.quat_z;

        ahrs->last_sensor_timestamp	= sensor_timestamp;

        /* Notify external data arrival subscribers, if any. */
        for (int i = 0; i < MAX_NUM_CALLBACKS; i++) {
            ITimestampedDataSubscriber *callback = ahrs->callbacks[i];
            if (callback != NULL) {
            	long system_timestamp = (long)(Timer::GetFPGATimestamp() * 1000);
                callback->timestampedDataReceived(system_timestamp,
                		sensor_timestamp,
                		ahrs_update,
                		ahrs->callback_contexts[i]);
            }
        }

        ahrs->UpdateDisplacement( ahrs->world_linear_accel_x,
                ahrs->world_linear_accel_y,
                ahrs->update_rate_hz,
                ahrs->is_moving);

        ahrs->yaw_angle_tracker->NextAngle(ahrs->GetYaw());
    }

    void SetBoardID(AHRSProtocol::BoardID& board_id) {
        ahrs->board_type = board_id.type;
        ahrs->hw_rev = board_id.hw_rev;
        ahrs->fw_ver_major = board_id.fw_ver_major;
        ahrs->fw_ver_minor = board_id.fw_ver_minor;
    }

    void SetBoardState(IIOCompleteNotification::BoardState& board_state) {
        ahrs->update_rate_hz = board_state.update_rate_hz;
        ahrs->accel_fsr_g = board_state.accel_fsr_g;
        ahrs->gyro_fsr_dps = board_state.gyro_fsr_dps;
        ahrs->capability_flags = board_state.capability_flags;
        ahrs->op_status = board_state.op_status;
        ahrs->sensor_status = board_state.sensor_status;
        ahrs->cal_status = board_state.cal_status;
        ahrs->selftest_status = board_state.selftest_status;
    }

	void YawResetComplete() {
		ahrs->yaw_angle_tracker->Reset();
	}

    /***********************************************************/
    /* IBoardCapabilities Interface Implementation        */
    /***********************************************************/
    bool IsOmniMountSupported()
    {
       return (((ahrs->capability_flags & NAVX_CAPABILITY_FLAG_OMNIMOUNT) !=0) ? true : false);
    }

    bool IsBoardYawResetSupported()
    {
        return (((ahrs->capability_flags & NAVX_CAPABILITY_FLAG_YAW_RESET) != 0) ? true : false);
    }

    bool IsDisplacementSupported()
    {
        return (((ahrs->capability_flags & NAVX_CAPABILITY_FLAG_VEL_AND_DISP) != 0) ? true : false);
    }

    bool IsAHRSPosTimestampSupported()
    {
    	return (((ahrs->capability_flags & NAVX_CAPABILITY_FLAG_AHRSPOS_TS) != 0) ? true : false);
    }
};

/**
 * The AHRS class provides an interface to AHRS capabilities
 * of the KauaiLabs navX Robotics Navigation Sensor via SPI, I2C and
 * Serial (TTL UART and USB) communications interfaces on the RoboRIO.
 *
 * The AHRS class enables access to basic connectivity and state information,
 * as well as key 6-axis and 9-axis orientation information (yaw, pitch, roll,
 * compass heading, fused (9-axis) heading and magnetic disturbance detection.
 *
 * Additionally, the ARHS class also provides access to extended information
 * including linear acceleration, motion detection, rotation detection and sensor
 * temperature.
 *
 * If used with the navX Aero, the AHRS class also provides access to
 * altitude, barometric pressure and pressure sensor temperature data
 * @author Scott
 */

AHRS::AHRS(SPI::Port spi_port_id, uint8_t update_rate_hz) {
    SPIInit(spi_port_id, DEFAULT_SPI_BITRATE, update_rate_hz);
}

/**
 * The AHRS class provides an interface to AHRS capabilities
 * of the KauaiLabs navX Robotics Navigation Sensor via SPI, I2C and
 * Serial (TTL UART and USB) communications interfaces on the RoboRIO.
 *
 * The AHRS class enables access to basic connectivity and state information,
 * as well as key 6-axis and 9-axis orientation information (yaw, pitch, roll,
 * compass heading, fused (9-axis) heading and magnetic disturbance detection.
 *
 * Additionally, the ARHS class also provides access to extended information
 * including linear acceleration, motion detection, rotation detection and sensor
 * temperature.
 *
 * If used with the navX Aero, the AHRS class also provides access to
 * altitude, barometric pressure and pressure sensor temperature data
 *
 * This constructor allows the specification of a custom SPI bitrate, in bits/second.
 *
 * @author Scott
 */

AHRS::AHRS(SPI::Port spi_port_id, uint32_t spi_bitrate, uint8_t update_rate_hz) {
    SPIInit(spi_port_id, spi_bitrate, update_rate_hz);
}

/**
 * Constructs the AHRS class using I2C communication, overriding the
 * default update rate with a custom rate which may be from 4 to 200,
 * representing the number of updates per second sent by the sensor.
 *<p>
 * This constructor should be used if communicating via I2C.
 *<p>
 * Note that increasing the update rate may increase the
 * CPU utilization.
 *<p>
 * @param i2c_port_id I2C Port to use
 * @param update_rate_hz Custom Update Rate (Hz)
 */
AHRS::AHRS(I2C::Port i2c_port_id, uint8_t update_rate_hz) {
    I2CInit(i2c_port_id, update_rate_hz);
}

    /**
     * Constructs the AHRS class using serial communication, overriding the
     * default update rate with a custom rate which may be from 4 to 200,
     * representing the number of updates per second sent by the sensor.
     *<p>
     * This constructor should be used if communicating via either
     * TTL UART or USB Serial interface.
     *<p>
     * Note that the serial interfaces can communicate either
     * processed data, or raw data, but not both simultaneously.
     * If simultaneous processed and raw data are needed, use
     * one of the register-based interfaces (SPI or I2C).
     *<p>
     * Note that increasing the update rate may increase the
     * CPU utilization.
     *<p>
     * @param serial_port_id SerialPort to use
     * @param data_type either kProcessedData or kRawData
     * @param update_rate_hz Custom Update Rate (Hz)
     */
AHRS::AHRS(SerialPort::Port serial_port_id, AHRS::SerialDataType data_type, uint8_t update_rate_hz) {
    SerialInit(serial_port_id, data_type, update_rate_hz);
}

/**
 * Constructs the AHRS class using SPI communication and the default update rate.
 *<p>
 * This constructor should be used if communicating via SPI.
 *<p>
 * @param spi_port_id SPI port to use.
 */
AHRS::AHRS(SPI::Port spi_port_id) {
    SPIInit(spi_port_id, DEFAULT_SPI_BITRATE, NAVX_DEFAULT_UPDATE_RATE_HZ);
}


/**
 * Constructs the AHRS class using I2C communication and the default update rate.
 *<p>
 * This constructor should be used if communicating via I2C.
 *<p>
 * @param i2c_port_id I2C port to use
 */
AHRS::AHRS(I2C::Port i2c_port_id) {
    I2CInit(i2c_port_id, NAVX_DEFAULT_UPDATE_RATE_HZ);
}


/**
 * Constructs the AHRS class using serial communication and the default update rate,
 * and returning processed (rather than raw) data.
 *<p>
 * This constructor should be used if communicating via either
 * TTL UART or USB Serial interface.
 *<p>
 * @param serial_port_id SerialPort to use
 */
AHRS::AHRS(SerialPort::Port serial_port_id) {
    SerialInit(serial_port_id, SerialDataType::kProcessedData, NAVX_DEFAULT_UPDATE_RATE_HZ);
}

/**
 * Returns the current pitch value (in degrees, from -180 to 180)
 * reported by the sensor.  Pitch is a measure of rotation around
 * the X Axis.
 * @return The current pitch value in degrees (-180 to 180).
 */
float AHRS::GetPitch() {
    return pitch;
}

/**
 * Returns the current roll value (in degrees, from -180 to 180)
 * reported by the sensor.  Roll is a measure of rotation around
 * the X Axis.
 * @return The current roll value in degrees (-180 to 180).
 */
float AHRS::GetRoll() {
    return roll;
}

/**
 * Returns the current yaw value (in degrees, from -180 to 180)
 * reported by the sensor.  Yaw is a measure of rotation around
 * the Z Axis (which is perpendicular to the earth).
 *<p>
 * Note that the returned yaw value will be offset by a user-specified
 * offset value; this user-specified offset value is set by
 * invoking the zeroYaw() method.
 * @return The current yaw value in degrees (-180 to 180).
 */
float AHRS::GetYaw() {
    if ( ahrs_internal->IsBoardYawResetSupported() ) {
        return this->yaw;
    } else {
        return (float) yaw_offset_tracker->ApplyOffset(this->yaw);
    }
}

/**
 * Returns the current tilt-compensated compass heading
 * value (in degrees, from 0 to 360) reported by the sensor.
 *<p>
 * Note that this value is sensed by a magnetometer,
 * which can be affected by nearby magnetic fields (e.g., the
 * magnetic fields generated by nearby motors).
 *<p>
 * Before using this value, ensure that (a) the magnetometer
 * has been calibrated and (b) that a magnetic disturbance is
 * not taking place at the instant when the compass heading
 * was generated.
 * @return The current tilt-compensated compass heading, in degrees (0-360).
 */
float AHRS::GetCompassHeading() {
    return compass_heading;
}

/**
 * Sets the user-specified yaw offset to the current
 * yaw value reported by the sensor.
 *<p>
 * This user-specified yaw offset is automatically
 * subtracted from subsequent yaw values reported by
 * the getYaw() method.
 */
void AHRS::ZeroYaw() {
    if ( ahrs_internal->IsBoardYawResetSupported() ) {
        io->ZeroYaw();
        /* Notification is deferred until action is complete. */
    } else {
		yaw_offset_tracker->SetOffset();
		/* Notification occurs immediately. */
		ahrs_internal->YawResetComplete();
    }
}

/**
 * Returns true if the sensor is currently performing automatic
 * gyro/accelerometer calibration.  Automatic calibration occurs
 * when the sensor is initially powered on, during which time the
 * sensor should be held still, with the Z-axis pointing up
 * (perpendicular to the earth).
 *<p>
 * NOTE:  During this automatic calibration, the yaw, pitch and roll
 * values returned may not be accurate.
 *<p>
 * Once calibration is complete, the sensor will automatically remove
 * an internal yaw offset value from all reported values.
 *<p>
 * @return Returns true if the sensor is currently automatically
 * calibrating the gyro and accelerometer sensors.
 */
bool AHRS::IsCalibrating() {
    return !((cal_status &
                NAVX_CAL_STATUS_IMU_CAL_STATE_MASK) ==
                    NAVX_CAL_STATUS_IMU_CAL_COMPLETE);
}

/**
 * Indicates whether the sensor is currently connected
 * to the host computer.  A connection is considered established
 * whenever communication with the sensor has occurred recently.
 *<p>
 * @return Returns true if a valid update has been recently received
 * from the sensor.
 */
bool AHRS::IsConnected() {
    return io->IsConnected();
}

/**
 * Returns the count in bytes of data received from the
 * sensor.  This could can be useful for diagnosing
 * connectivity issues.
 *<p>
 * If the byte count is increasing, but the update count
 * (see getUpdateCount()) is not, this indicates a software
 * misconfiguration.
 * @return The number of bytes received from the sensor.
 */
double AHRS::GetByteCount() {
    return io->GetByteCount();
}

/**
 * Returns the count of valid updates which have
 * been received from the sensor.  This count should increase
 * at the same rate indicated by the configured update rate.
 * @return The number of valid updates received from the sensor.
 */
double AHRS::GetUpdateCount() {
    return io->GetUpdateCount();
}

/**
 * Returns the sensor timestamp corresponding to the
 * last sample retrieved from the sensor.  Note that this
 * sensor timestamp is only provided when the Register-based
 * IO methods (SPI, I2C) are used; sensor timestamps are not
 * provided when Serial-based IO methods (TTL UART, USB)
 * are used.
 * @return The sensor timestamp corresponding to the current AHRS sensor data.
 */
long AHRS::GetLastSensorTimestamp() {
	return this->last_sensor_timestamp;
}

/**
 * Returns the current linear acceleration in the X-axis (in G).
 *<p>
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the x-axis of the
 * body (e.g., the robot) on which the sensor is mounted.
 *<p>
 * @return Current world linear acceleration in the X-axis (in G).
 */
float AHRS::GetWorldLinearAccelX()
{
    return this->world_linear_accel_x;
}

/**
 * Returns the current linear acceleration in the Y-axis (in G).
 *<p>
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the Y-axis of the
 * body (e.g., the robot) on which the sensor is mounted.
 *<p>
 * @return Current world linear acceleration in the Y-axis (in G).
 */
float AHRS::GetWorldLinearAccelY()
{
    return this->world_linear_accel_y;
}

/**
 * Returns the current linear acceleration in the Z-axis (in G).
 *<p>
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the Z-axis of the
 * body (e.g., the robot) on which the sensor is mounted.
 *<p>
 * @return Current world linear acceleration in the Z-axis (in G).
 */
float AHRS::GetWorldLinearAccelZ()
{
    return this->world_linear_accel_z;
}

/**
 * Indicates if the sensor is currently detecting motion,
 * based upon the X and Y-axis world linear acceleration values.
 * If the sum of the absolute values of the X and Y axis exceed
 * a "motion threshold", the motion state is indicated.
 *<p>
 * @return Returns true if the sensor is currently detecting motion.
 */
bool AHRS::IsMoving()
{
    return is_moving;
}

/**
 * Indicates if the sensor is currently detecting yaw rotation,
 * based upon whether the change in yaw over the last second
 * exceeds the "Rotation Threshold."
 *<p>
 * Yaw Rotation can occur either when the sensor is rotating, or
 * when the sensor is not rotating AND the current gyro calibration
 * is insufficiently calibrated to yield the standard yaw drift rate.
 *<p>
 * @return Returns true if the sensor is currently detecting motion.
 */
bool AHRS::IsRotating()
{
    return is_rotating;
}

/**
 * Returns the current barometric pressure, based upon calibrated readings
 * from the onboard pressure sensor.  This value is in units of millibar.
 *<p>
 * NOTE:  This value is only valid for a navX Aero.  To determine
 * whether this value is valid, see isAltitudeValid().
 * @return Returns current barometric pressure (navX Aero only).
 */
float AHRS::GetBarometricPressure()
{
    return baro_pressure;
}

/**
 * Returns the current altitude, based upon calibrated readings
 * from a barometric pressure sensor, and the currently-configured
 * sea-level barometric pressure [navX Aero only].  This value is in units of meters.
 *<p>
 * NOTE:  This value is only valid sensors including a pressure
 * sensor.  To determine whether this value is valid, see
 * isAltitudeValid().
 *<p>
 * @return Returns current altitude in meters (as long as the sensor includes
 * an installed on-board pressure sensor).
 */
float AHRS::GetAltitude()
{
    return altitude;
}

/**
 * Indicates whether the current altitude (and barometric pressure) data is
 * valid. This value will only be true for a sensor with an onboard
 * pressure sensor installed.
 *<p>
 * If this value is false for a board with an installed pressure sensor,
 * this indicates a malfunction of the onboard pressure sensor.
 *<p>
 * @return Returns true if a working pressure sensor is installed.
 */
bool AHRS::IsAltitudeValid()
{
    return this->altitude_valid;
}

/**
 * Returns the "fused" (9-axis) heading.
 *<p>
 * The 9-axis heading is the fusion of the yaw angle, the tilt-corrected
 * compass heading, and magnetic disturbance detection.  Note that the
 * magnetometer calibration procedure is required in order to
 * achieve valid 9-axis headings.
 *<p>
 * The 9-axis Heading represents the sensor's best estimate of current heading,
 * based upon the last known valid Compass Angle, and updated by the change in the
 * Yaw Angle since the last known valid Compass Angle.  The last known valid Compass
 * Angle is updated whenever a Calibrated Compass Angle is read and the sensor
 * has recently rotated less than the Compass Noise Bandwidth (~2 degrees).
 * @return Fused Heading in Degrees (range 0-360)
 */
float AHRS::GetFusedHeading()
{
    return fused_heading;
}

/**
 * Indicates whether the current magnetic field strength diverges from the
 * calibrated value for the earth's magnetic field by more than the currently-
 * configured Magnetic Disturbance Ratio.
 *<p>
 * This function will always return false if the sensor's magnetometer has
 * not yet been calibrated; see isMagnetometerCalibrated().
 * @return true if a magnetic disturbance is detected (or the magnetometer is uncalibrated).
 */
bool AHRS::IsMagneticDisturbance()
{
    return magnetic_disturbance;
}

/**
 * Indicates whether the magnetometer has been calibrated.
 *<p>
 * Magnetometer Calibration must be performed by the user.
 *<p>
 * Note that if this function does indicate the magnetometer is calibrated,
 * this does not necessarily mean that the calibration quality is sufficient
 * to yield valid compass headings.
 *<p>
 * @return Returns true if magnetometer calibration has been performed.
 */
bool AHRS::IsMagnetometerCalibrated()
{
    return is_magnetometer_calibrated;
}

/* Unit Quaternions */

/**
 * Returns the imaginary portion (W) of the Orientation Quaternion which
 * fully describes the current sensor orientation with respect to the
 * reference angle defined as the angle at which the yaw was last "zeroed".
 *<p>
 * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
 * to 2.  This total range (4) can be associated with a unit circle, since
 * each circle is comprised of 4 PI Radians.
 * <p>
 * For more information on Quaternions and their use, please see this <a href=https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>definition</a>.
 * @return Returns the imaginary portion (W) of the quaternion.
 */
float AHRS::GetQuaternionW() {
    return quaternionW;
}
/**
 * Returns the real portion (X axis) of the Orientation Quaternion which
 * fully describes the current sensor orientation with respect to the
 * reference angle defined as the angle at which the yaw was last "zeroed".
 * <p>
 * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
 * to 2.  This total range (4) can be associated with a unit circle, since
 * each circle is comprised of 4 PI Radians.
 * <p>
 * For more information on Quaternions and their use, please see this <a href=https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>description</a>.
 * @return Returns the real portion (X) of the quaternion.
 */
float AHRS::GetQuaternionX() {
    return quaternionX;
}
/**
 * Returns the real portion (X axis) of the Orientation Quaternion which
 * fully describes the current sensor orientation with respect to the
 * reference angle defined as the angle at which the yaw was last "zeroed".
 *
 * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
 * to 2.  This total range (4) can be associated with a unit circle, since
 * each circle is comprised of 4 PI Radians.
 *
 * For more information on Quaternions and their use, please see:
 *
 *   https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
 *
 * @return Returns the real portion (X) of the quaternion.
 */
float AHRS::GetQuaternionY() {
    return quaternionY;
}
/**
 * Returns the real portion (X axis) of the Orientation Quaternion which
 * fully describes the current sensor orientation with respect to the
 * reference angle defined as the angle at which the yaw was last "zeroed".
 *
 * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
 * to 2.  This total range (4) can be associated with a unit circle, since
 * each circle is comprised of 4 PI Radians.
 *
 * For more information on Quaternions and their use, please see:
 *
 *   https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
 *
 * @return Returns the real portion (X) of the quaternion.
 */
float AHRS::GetQuaternionZ() {
    return quaternionZ;
}

/**
 * Zeros the displacement integration variables.   Invoke this at the moment when
 * integration begins.
 */
void AHRS::ResetDisplacement() {
    if (ahrs_internal->IsDisplacementSupported() ) {
        io->ZeroDisplacement();
    }
    else {
        integrator->ResetDisplacement();
    }
}

/**
 * Each time new linear acceleration samples are received, this function should be invoked.
 * This function transforms acceleration in G to meters/sec^2, then converts this value to
 * Velocity in meters/sec (based upon velocity in the previous sample).  Finally, this value
 * is converted to displacement in meters, and integrated.
 * @return none.
 */

void AHRS::UpdateDisplacement( float accel_x_g, float accel_y_g,
                                    int update_rate_hz, bool is_moving ) {
    integrator->UpdateDisplacement(accel_x_g, accel_y_g, update_rate_hz, is_moving);
}

/**
 * Returns the velocity (in meters/sec) of the X axis [Experimental].
 *
 * NOTE:  This feature is experimental.  Velocity measures rely on integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting velocities are not known to be very accurate.
 * @return Current Velocity (in meters/squared).
 */
float AHRS::GetVelocityX() {
    return (ahrs_internal->IsDisplacementSupported() ? velocity[0] : integrator->GetVelocityX());
}

/**
 * Returns the velocity (in meters/sec) of the Y axis [Experimental].
 *
 * NOTE:  This feature is experimental.  Velocity measures rely on integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting velocities are not known to be very accurate.
 * @return Current Velocity (in meters/squared).
 */
float AHRS::GetVelocityY() {
    return (ahrs_internal->IsDisplacementSupported() ? velocity[1] : integrator->GetVelocityY());
}

/**
 * Returns the velocity (in meters/sec) of the Z axis [Experimental].
 *
 * NOTE:  This feature is experimental.  Velocity measures rely on integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting velocities are not known to be very accurate.
 * @return Current Velocity (in meters/squared).
 */
float AHRS::GetVelocityZ() {
    return (ahrs_internal->IsDisplacementSupported() ? velocity[2] : 0.f);
}

/**
 * Returns the displacement (in meters) of the X axis since resetDisplacement()
 * was last invoked [Experimental].
 *
 * NOTE:  This feature is experimental.  Displacement measures rely on double-integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting displacement are not known to be very accurate, and the amount of error
 * increases quickly as time progresses.
 * @return Displacement since last reset (in meters).
 */
float AHRS::GetDisplacementX() {
    return (ahrs_internal->IsDisplacementSupported() ? displacement[0] : integrator->GetVelocityX());
}

/**
 * Returns the displacement (in meters) of the Y axis since resetDisplacement()
 * was last invoked [Experimental].
 *
 * NOTE:  This feature is experimental.  Displacement measures rely on double-integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting displacement are not known to be very accurate, and the amount of error
 * increases quickly as time progresses.
 * @return Displacement since last reset (in meters).
 */
float AHRS::GetDisplacementY() {
    return (ahrs_internal->IsDisplacementSupported() ? displacement[1] : integrator->GetVelocityY());
}

/**
 * Returns the displacement (in meters) of the Z axis since resetDisplacement()
 * was last invoked [Experimental].
 *
 * NOTE:  This feature is experimental.  Displacement measures rely on double-integration
 * of acceleration values from MEMS accelerometers which yield "noisy" values.  The
 * resulting displacement are not known to be very accurate, and the amount of error
 * increases quickly as time progresses.
 * @return Displacement since last reset (in meters).
 */
float AHRS::GetDisplacementZ() {
    return (ahrs_internal->IsDisplacementSupported() ? displacement[2] : 0.f);
}

/**
 * Enables or disables logging (via Console I/O) of AHRS library internal
 * behaviors, including events such as transient communication errors.
 * @param enable
 */
void AHRS::EnableLogging(bool enable) {
	if ( this->io != NULL) {
		io->EnableLogging(enable);
	}
}

int16_t AHRS::GetGyroFullScaleRangeDPS() {
	return gyro_fsr_dps;
}

int16_t AHRS::GetAccelFullScaleRangeG() {
	return accel_fsr_g;
}

#define NAVX_IO_THREAD_NAME "navXIOThread"

void AHRS::SPIInit( SPI::Port spi_port_id, uint32_t bitrate, uint8_t update_rate_hz ) {
    commonInit( update_rate_hz );
    io = new RegisterIO(new RegisterIO_SPI(new SPI(spi_port_id), bitrate), update_rate_hz, ahrs_internal, ahrs_internal);
    task = new std::thread(AHRS::ThreadFunc, io);
}

void AHRS::I2CInit( I2C::Port i2c_port_id, uint8_t update_rate_hz ) {
    commonInit(update_rate_hz);
    io = new RegisterIO(new RegisterIO_I2C(new I2C(i2c_port_id, NAVX_MXP_I2C_ADDRESS)), update_rate_hz, ahrs_internal, ahrs_internal);
    task = new std::thread(AHRS::ThreadFunc, io);
}

void AHRS::SerialInit(SerialPort::Port serial_port_id, AHRS::SerialDataType data_type, uint8_t update_rate_hz) {
    commonInit(update_rate_hz);
    bool processed_data = (data_type == SerialDataType::kProcessedData);
    io = new SerialIO(serial_port_id, update_rate_hz, processed_data, ahrs_internal, ahrs_internal);
    task = new std::thread(AHRS::ThreadFunc, io);
}

void AHRS::commonInit( uint8_t update_rate_hz ) {

    ahrs_internal = new AHRSInternal(this);
    this->update_rate_hz = update_rate_hz;

    /* Processed Data */

    yaw_offset_tracker = new OffsetTracker(YAW_HISTORY_LENGTH);
    integrator = new InertialDataIntegrator();
    yaw_angle_tracker = new ContinuousAngleTracker();

    yaw =
            pitch =
                    roll =
                            compass_heading = 0.0f;
    world_linear_accel_x =
            world_linear_accel_y =
                    world_linear_accel_z = 0.0f;
    mpu_temp_c = 0.0f;
    fused_heading = 0.0f;
    altitude = 0.0f;
    baro_pressure = 0.0f;
    is_moving = false;
    is_rotating = false;
    baro_sensor_temp_c = 0.0f;
    altitude_valid = false;
    is_magnetometer_calibrated = false;
    magnetic_disturbance = false;
    quaternionW =
            quaternionX =
                    quaternionY =
                            quaternionZ = 0.0f;

    /* Integrated Data */

    for ( int i = 0; i < 3; i++ ) {
        velocity[i] = 0.0f;
        displacement[i] = 0.0f;
    }


    /* Raw Data */
    raw_gyro_x =
            raw_gyro_y =
                    raw_gyro_z = 0.0f;
    raw_accel_x =
            raw_accel_y =
                    raw_accel_z = 0.0f;
    cal_mag_x =
            cal_mag_y =
                    cal_mag_z = 0.0f;

    /* Configuration/Status */
    update_rate_hz = 0;
    accel_fsr_g = DEFAULT_ACCEL_FSR_G;
    gyro_fsr_dps = DEFAULT_GYRO_FSR_DPS;
    capability_flags = 0;
    op_status =
            sensor_status =
                    cal_status =
                            selftest_status = 0;
    /* Board ID */
    board_type =
            hw_rev =
                    fw_ver_major =
                            fw_ver_minor = 0;
    last_sensor_timestamp = 0;
    last_update_time = 0;

    io = 0;

    for ( int i = 0; i < MAX_NUM_CALLBACKS; i++) {
    	callbacks[i] = NULL;
    	callback_contexts[i] = NULL;
    }
}

/**
 * Returns the total accumulated yaw angle (Z Axis, in degrees)
 * reported by the sensor.
 *<p>
 * NOTE: The angle is continuous, meaning it's range is beyond 360 degrees.
 * This ensures that algorithms that wouldn't want to see a discontinuity
 * in the gyro output as it sweeps past 0 on the second time around.
 *<p>
 * Note that the returned yaw value will be offset by a user-specified
 * offset value; this user-specified offset value is set by
 * invoking the zeroYaw() method.
 *<p>
 * @return The current total accumulated yaw angle (Z axis) of the robot
 * in degrees. This heading is based on integration of the returned rate
 * from the Z-axis (yaw) gyro.
 */

double AHRS::GetAngle() {
    return yaw_angle_tracker->GetAngle();
}

/**
 * Return the rate of rotation of the yaw (Z-axis) gyro, in degrees per second.
 *<p>
 * The rate is based on the most recent reading of the yaw gyro angle.
 *<p>
 * @return The current rate of change in yaw angle (in degrees per second)
 */

double AHRS::GetRate() {
    return yaw_angle_tracker->GetRate();
}

/**
 * Sets an amount of angle to be automatically added before returning a
 * angle from the getAngle() method.  This allows users of the getAngle() method
 * to logically rotate the sensor by a given amount of degrees.
 * <p>
 * NOTE 1:  The adjustment angle is <b>only</b> applied to the value returned
 * from getAngle() - it does not adjust the value returned from getYaw(), nor
 * any of the quaternion values.
 * <p>
 * NOTE 2:  The adjustment angle is <b>not</b>automatically cleared whenever the
 * sensor yaw angle is reset.
 * <p>
 * If not set, the default adjustment angle is 0 degrees (no adjustment).
 * @param adjustment, in degrees (range:  -360 to 360)
 */
void AHRS::SetAngleAdjustment(double adjustment) {
	yaw_angle_tracker->SetAngleAdjustment(adjustment);
}

/**
 * Returns the currently configured adjustment angle.  See
 * setAngleAdjustment() for more details.
 *
 * If this method returns 0 degrees, no adjustment to the value returned
 * via getAngle() will occur.
 * @param adjustment, in degrees (range:  -360 to 360)
 */
double AHRS::GetAngleAdjustment() {
	return yaw_angle_tracker->GetAngleAdjustment();
}

/**
 * Reset the Yaw gyro.
 *<p>
 * Resets the Gyro Z (Yaw) axis to a heading of zero. This can be used if
 * there is significant drift in the gyro and it needs to be recalibrated
 * after it has been running.
 */
void AHRS::Reset() {
    ZeroYaw();
}

static const float DEV_UNITS_MAX = 32768.0f;

/**
 * Returns the current raw (unprocessed) X-axis gyro rotation rate (in degrees/sec).  NOTE:  this
 * value is un-processed, and should only be accessed by advanced users.
 * Typically, rotation about the X Axis is referred to as "Pitch".  Calibrated
 * and Integrated Pitch data is accessible via the {@link #GetPitch()} method.
 *<p>
 * @return Returns the current rotation rate (in degrees/sec).
 */
float AHRS::GetRawGyroX() {
    return this->raw_gyro_x / (DEV_UNITS_MAX / (float)gyro_fsr_dps);
}

/**
 * Returns the current raw (unprocessed) Y-axis gyro rotation rate (in degrees/sec).  NOTE:  this
 * value is un-processed, and should only be accessed by advanced users.
 * Typically, rotation about the T Axis is referred to as "Roll".  Calibrated
 * and Integrated Pitch data is accessible via the {@link #GetRoll()} method.
 *<p>
 * @return Returns the current rotation rate (in degrees/sec).
 */
float AHRS::GetRawGyroY() {
    return this->raw_gyro_y / (DEV_UNITS_MAX / (float)gyro_fsr_dps);
}

/**
 * Returns the current raw (unprocessed) Z-axis gyro rotation rate (in degrees/sec).  NOTE:  this
 * value is un-processed, and should only be accessed by advanced users.
 * Typically, rotation about the T Axis is referred to as "Yaw".  Calibrated
 * and Integrated Pitch data is accessible via the {@link #GetYaw()} method.
 *<p>
 * @return Returns the current rotation rate (in degrees/sec).
 */
float AHRS::GetRawGyroZ() {
    return this->raw_gyro_z / (DEV_UNITS_MAX / (float)gyro_fsr_dps);
}

/**
 * Returns the current raw (unprocessed) X-axis acceleration rate (in G).  NOTE:  this
 * value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not had acceleration due to gravity removed from it, and has not been rotated to
 * the world reference frame.  Gravity-corrected, world reference frame-corrected
 * X axis acceleration data is accessible via the {@link #GetWorldLinearAccelX()} method.
 *<p>
 * @return Returns the current acceleration rate (in G).
 */
float AHRS::GetRawAccelX() {
    return this->raw_accel_x / (DEV_UNITS_MAX / (float)accel_fsr_g);
}

/**
 * Returns the current raw (unprocessed) Y-axis acceleration rate (in G).  NOTE:  this
 * value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not had acceleration due to gravity removed from it, and has not been rotated to
 * the world reference frame.  Gravity-corrected, world reference frame-corrected
 * Y axis acceleration data is accessible via the {@link #GetWorldLinearAccelY()} method.
 *<p>
 * @return Returns the current acceleration rate (in G).
 */
float AHRS::GetRawAccelY() {
    return this->raw_accel_y / (DEV_UNITS_MAX / (float)accel_fsr_g);
}

/**
 * Returns the current raw (unprocessed) Z-axis acceleration rate (in G).  NOTE:  this
 * value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not had acceleration due to gravity removed from it, and has not been rotated to
 * the world reference frame.  Gravity-corrected, world reference frame-corrected
 * Z axis acceleration data is accessible via the {@link #GetWorldLinearAccelZ()} method.
 *<p>
 * @return Returns the current acceleration rate (in G).
 */
float AHRS::GetRawAccelZ() {
    return this->raw_accel_z / (DEV_UNITS_MAX / (float)accel_fsr_g);
}

static const float UTESLA_PER_DEV_UNIT = 0.15f;

/**
 * Returns the current raw (unprocessed) X-axis magnetometer reading (in uTesla).  NOTE:
 * this value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not been tilt-corrected, and has not been combined with the other magnetometer axis
 * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
 * via the {@link #GetCompassHeading()} method.
 *<p>
 * @return Returns the mag field strength (in uTesla).
 */
float AHRS::GetRawMagX() {
    return this->cal_mag_x / UTESLA_PER_DEV_UNIT;
}

/**
 * Returns the current raw (unprocessed) Y-axis magnetometer reading (in uTesla).  NOTE:
 * this value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not been tilt-corrected, and has not been combined with the other magnetometer axis
 * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
 * via the {@link #GetCompassHeading()} method.
 *<p>
 * @return Returns the mag field strength (in uTesla).
 */
float AHRS::GetRawMagY() {
    return this->cal_mag_y / UTESLA_PER_DEV_UNIT;
}

/**
 * Returns the current raw (unprocessed) Z-axis magnetometer reading (in uTesla).  NOTE:
 * this value is unprocessed, and should only be accessed by advanced users.  This raw value
 * has not been tilt-corrected, and has not been combined with the other magnetometer axis
 * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
 * via the {@link #GetCompassHeading()} method.
 *<p>
 * @return Returns the mag field strength (in uTesla).
 */
float AHRS::GetRawMagZ() {
    return this->cal_mag_z / UTESLA_PER_DEV_UNIT;
 }

/**
 * Returns the current barometric pressure (in millibar) [navX Aero only].
 *<p>
 *This value is valid only if a barometric pressure sensor is onboard.
 *
 * @return Returns the current barometric pressure (in millibar).
 */
float AHRS::GetPressure() {
    // TODO implement for navX-Aero.
    return 0;
}


/**
 * Returns the current temperature (in degrees centigrade) reported by
 * the sensor's gyro/accelerometer circuit.
 *<p>
 * This value may be useful in order to perform advanced temperature-
 * correction of raw gyroscope and accelerometer values.
 *<p>
 * @return The current temperature (in degrees centigrade).
 */
float AHRS::GetTempC()
{
    return this->mpu_temp_c;
}

/**
 * Returns information regarding which sensor board axis (X,Y or Z) and
 * direction (up/down) is currently configured to report Yaw (Z) angle
 * values.   NOTE:  If the board firmware supports Omnimount, the board yaw
 * axis/direction are configurable.
 *<p>
 * For more information on Omnimount, please see:
 *<p>
 * http://navx-mxp.kauailabs.com/navx-mxp/installation/omnimount/
 *<p>
 * @return The currently-configured board yaw axis/direction.
 */
AHRS::BoardYawAxis AHRS::GetBoardYawAxis() {
    BoardYawAxis yaw_axis;
    short yaw_axis_info = (short)(capability_flags >> 3);
    yaw_axis_info &= 7;
    if ( yaw_axis_info == OMNIMOUNT_DEFAULT) {
        yaw_axis.up = true;
        yaw_axis.board_axis = BoardAxis::kBoardAxisZ;
    } else {
        yaw_axis.up = (((yaw_axis_info & 0x01) != 0) ? true : false);
        yaw_axis_info >>= 1;
        switch ( yaw_axis_info ) {
        case 0:
            yaw_axis.board_axis = BoardAxis::kBoardAxisX;
            break;
        case 1:
            yaw_axis.board_axis = BoardAxis::kBoardAxisY;
            break;
        case 2:
        default:
            yaw_axis.board_axis = BoardAxis::kBoardAxisZ;
            break;
        }
    }
    return yaw_axis;
}

/**
 * Returns the version number of the firmware currently executing
 * on the sensor.
 *<p>
 * To update the firmware to the latest version, please see:
 *<p>
 *   http://navx-mxp.kauailabs.com/navx-mxp/support/updating-firmware/
 *<p>
 * @return The firmware version in the format [MajorVersion].[MinorVersion]
 */
std::string AHRS::GetFirmwareVersion() {
    std::ostringstream os;
    os << (int)fw_ver_major << "." << (int)fw_ver_minor;
    std::string fw_version = os.str();
    return fw_version;
}

    /***********************************************************/
    /* SendableBase Interface Implementation                   */
    /***********************************************************/

void AHRS::InitSendable(SendableBuilder& builder) {
	builder.SetSmartDashboardType("Gyro");
	builder.AddDoubleProperty("Value", [=]() { return GetYaw(); }, nullptr);
}

/***********************************************************/
/* PIDSource Interface Implementation                      */
/***********************************************************/

/**
 * Returns the current yaw value reported by the sensor.  This
 * yaw value is useful for implementing features including "auto rotate
 * to a known angle".
 * @return The current yaw angle in degrees (-180 to 180).
 */
double AHRS::PIDGet() {
    return GetYaw();
}

int AHRS::ThreadFunc(IIOProvider *io_provider) {
    io_provider->Run();
    return 0;
}

/**
 * Registers a callback interface.  This interface
 * will be called back when new data is available,
 * based upon a change in the sensor timestamp.
 *<p>
 * Note that this callback will occur within the context of the
 * device IO thread, which is not the same thread context the
 * caller typically executes in.
 */
bool AHRS::RegisterCallback( ITimestampedDataSubscriber *callback, void *callback_context) {
    bool registered = false;
    for ( int i = 0; i < MAX_NUM_CALLBACKS; i++ ) {
        if (callbacks[i] == NULL) {
            callbacks[i] = callback;
            callback_contexts[i] = callback_context;
            registered = true;
            break;
        }
    }
    return registered;
}

/**
 * Deregisters a previously registered callback interface.
 *
 * Be sure to deregister any callback which have been
 * previously registered, to ensure that the object
 * implementing the callback interface does not continue
 * to be accessed when no longer necessary.
 */
bool AHRS::DeregisterCallback( ITimestampedDataSubscriber *callback ) {
    bool deregistered = false;
    for ( int i = 0; i < MAX_NUM_CALLBACKS; i++ ) {
        if (callbacks[i] == callback) {
            callbacks[i] = NULL;
            deregistered = true;
            break;
        }
    }
    return deregistered;
}

/**
 * Returns the navX-Model device's currently configured update
 * rate.  Note that the update rate that can actually be realized
 * is a value evenly divisible by the navX-Model device's internal
 * motion processor sample clock (200Hz).  Therefore, the rate that
 * is returned may be lower than the requested sample rate.
 *
 * The actual sample rate is rounded down to the nearest integer
 * that is divisible by the number of Digital Motion Processor clock
 * ticks.  For instance, a request for 58 Hertz will result in
 * an actual rate of 66Hz (200 / (200 / 58), using integer
 * math.
 *
 * @return Returns the current actual update rate in Hz
 * (cycles per second).
 */

int AHRS::GetActualUpdateRate() {
    uint8_t actual_update_rate = GetActualUpdateRateInternal(GetRequestedUpdateRate());
    return (int)actual_update_rate;
}

uint8_t AHRS::GetActualUpdateRateInternal(uint8_t update_rate) {
#define NAVX_MOTION_PROCESSOR_UPDATE_RATE_HZ 200
    int integer_update_rate = (int)update_rate;
    int realized_update_rate = NAVX_MOTION_PROCESSOR_UPDATE_RATE_HZ /
            (NAVX_MOTION_PROCESSOR_UPDATE_RATE_HZ / integer_update_rate);
    return (uint8_t)realized_update_rate;
}

/**
 * Returns the currently requested update rate.
 * rate.  Note that not every update rate can actually be realized,
 * since the actual update rate must be a value evenly divisible by
 * the navX-Model device's internal motion processor sample clock (200Hz).
 *
 * To determine the actual update rate, use the
 * {@link #getActualUpdateRate()} method.
 *
 * @return Returns the requested update rate in Hz
 * (cycles per second).
 */

int AHRS::GetRequestedUpdateRate() {
    return (int)update_rate_hz;
}


