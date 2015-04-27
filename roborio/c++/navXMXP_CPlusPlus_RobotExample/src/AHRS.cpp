/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2015. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/

#include "AHRS.h"
#include "AHRSProtocol.h"

/**
 * The AHRS class provides an interface to AHRS capabilities
 * of the KauaiLabs navX MXP Robotics Navigation Sensor.
 * 
 * The AHRS class enables access to basic connectivity and state information, 
 * as well as key 6-axis and 9-axis orientation information (yaw, pitch, roll, 
 * compass heading, fused (9-axis) heading and magnetic disturbance detection.
 * Additionally, the ARHS class also provides access to extended information
 * including linear acceleration, motion detection, rotation detection and sensor 
 * temperature.
 * 
 * If used with the navX MXP Aero, the AHRS class also provides access to 
 * altitude, barometric pressure and pressure sensor temperature data
 * @author Scott
 */

#define NAVX_MXP_DEFAULT_UPDATE_RATE_HZ 60

    /**
     * Constructs the AHRS class, overriding the default update rate
     * with a custom rate which may be from 4 to 60, representing
     * the number of updates per second sent by the navX MXP.  
     * 
     * Note that increasing the update rate may increase the 
     * CPU utilization.
     * @param serial_port SerialPort object to use
     * @param update_rate_hz Custom Update Rate (Hz)
     */
AHRS::AHRS(SerialPort *pport, uint8_t update_rate_hz) : IMU(pport,update_rate_hz)
{
		this->current_stream_type = MSGID_AHRS_UPDATE;
		world_linear_accel_x =
			world_linear_accel_y =
			world_linear_accel_z =
			mpu_temp_c =
			fused_heading =
			altitude =
			barometric_pressure =
			baro_sensor_temp_c =
			mag_field_norm_ratio = 0.0f;
        cal_mag_x = 
        		cal_mag_y =
        		cal_mag_z = 0;
        is_moving =
        		is_rotating =
        		altitude_valid =
        		is_magnetometer_calibrated =
        		magnetic_disturbance = false;
        ResetDisplacement();
}

AHRS::~AHRS()
{
}

int AHRS::DecodePacketHandler( char *received_data, int bytes_remaining )
{
	float yaw, pitch, roll, compass_heading;
	float altitude, fused_heading, linear_accel_x, linear_accel_y, linear_accel_z;
	float mpu_temp;
	int16_t raw_mag_x, raw_mag_y, raw_mag_z;
	int16_t cal_mag_x, cal_mag_y, cal_mag_z;
	float mag_norm_ratio, mag_norm_scalar;
	int16_t quat_w, quat_x, quat_y, quat_z;
	float baro_pressure, baro_temp;
	uint8_t op_status, sensor_status;
	uint8_t cal_status, selftest_status;

	int packet_length = AHRSProtocol::decodeAHRSUpdate(	received_data, bytes_remaining,
														yaw,
														pitch,
														roll,
														compass_heading,
														altitude,
														fused_heading,
														linear_accel_x,
														linear_accel_y,
														linear_accel_z,
														mpu_temp,
														raw_mag_x,
														raw_mag_y,
														raw_mag_z,
														cal_mag_x,
														cal_mag_y,
														cal_mag_z,
														mag_norm_ratio,
														mag_norm_scalar,
														quat_w,
														quat_x,
														quat_y,
														quat_z,
														baro_pressure,
														baro_temp,
														op_status,
														sensor_status,
														cal_status,
														selftest_status);
	if (packet_length > 0) {
		/* Update base IMU class variables */

		this->yaw = yaw;
		this->pitch = pitch;
		this->roll = roll;
		this->compass_heading = compass_heading;
		UpdateYawHistory(yaw);

		/* Update AHRS class variables */

		// 9-axis data
		this->fused_heading			= fused_heading;

		// Gravity-corrected linear acceleration (world-frame)
		this->world_linear_accel_x	= linear_accel_x;
		this->world_linear_accel_y	= linear_accel_y;
		this->world_linear_accel_z	= linear_accel_z;

		// Gyro/Accelerometer Die Temperature
		this->mpu_temp_c			= mpu_temp;

		// Barometric Pressure/Altitude
		this->altitude				= altitude;
		this->barometric_pressure	= baro_pressure;
		this->baro_sensor_temp_c	= baro_temp;

		// Magnetometer Data
		this->cal_mag_x				= cal_mag_x;
		this->cal_mag_y				= cal_mag_y;
		this->cal_mag_z				= cal_mag_z;
		this->mag_field_norm_ratio	= mag_field_norm_ratio;

		// Status/Motion Detection
		this->is_moving						=
				(((sensor_status &
						NAVX_SENSOR_STATUS_MOVING) != 0)
						? true : false);
		this->is_rotating					=
				(((sensor_status &
						NAVX_SENSOR_STATUS_YAW_STABLE) != 0)
						? true : false);
		this->altitude_valid				=
				(((sensor_status &
						NAVX_SENSOR_STATUS_ALTITUDE_VALID) != 0)
						? true : false);
		this->is_magnetometer_calibrated	=
				(((cal_status &
						NAVX_CAL_STATUS_MAG_CAL_COMPLETE) != 0)
						? true : false);
		this->magnetic_disturbance			=
				(((sensor_status &
						NAVX_SENSOR_STATUS_MAG_DISTURBANCE) != 0)
						? true : false);

		UpdateDisplacement( this->world_linear_accel_x,
				this->world_linear_accel_y,
				update_rate_hz,
				this->is_moving);
	}
	return packet_length;
}
        
/**
 * Returns the current linear acceleration in the x-axis (in g).
 *
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the x-axis of the
 * body (e.g., the robot) on which the nav6 IMU is mounted.
 *
 * @return Current world linear acceleration in the x-axis (in g).
 */
float AHRS::GetWorldLinearAccelX()
{
	return world_linear_accel_x;
}

/**
 * Returns the current linear acceleration in the y-axis (in g).
 *
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the y-axis of the
 * body (e.g., the robot) on which the nav6 IMU is mounted.
 *
 * @return Current world linear acceleration in the y-axis (in g).
 */
float AHRS::GetWorldLinearAccelY()
{
	return world_linear_accel_y;
}

/**
 * Returns the current linear acceleration in the z-axis (in g).
 *
 * World linear acceleration refers to raw acceleration data, which
 * has had the gravity component removed, and which has been rotated to
 * the same reference frame as the current yaw value.  The resulting
 * value represents the current acceleration in the z-axis of the
 * body (e.g., the robot) on which the nav6 IMU is mounted.
 *
 * @return Current world linear acceleration in the z-axis (in g).
 */
float AHRS::GetWorldLinearAccelZ()
{
	return world_linear_accel_z;
}

/**
 * Indicates if the navX MXP is currently detecting motion,
 * based upon the x and y-axis world linear acceleration values.
 * If the sum of the absolute values of the x and y axis exceed,
 * 0.01g, the motion state is indicated.
 * @return Returns true if the navX MXP is currently detecting motion.
 */
bool AHRS::IsMoving()
{
	return is_moving;
}

/**
 * Indicates if the navX MXP is currently detecting yaw rotation,
 * based upon whether the change in yaw over the last second
 * exceeds the "Rotation Threshold."
 *
 * Yaw Rotation can occur either when the navX MXP is rotating, or
 * when the navX MXP is not rotating, but the current gyro calibration
 * is insufficiently calibrated to yield the standard yaw drift rate.
 * @return Returns true if the navX MXP is currently detecting motion.
 */
bool AHRS::IsRotating()
{
	return is_rotating;
}

/**
 * Returns the current barometric pressure, based upon calibrated readings
 * from the onboard pressure sensor.  This value is in units of millibar.
 *
 * NOTE:  This value is only valid for a navX MXP Aero.  To determine
 * whether this value is valid, see isAltitudeValid().
 * @return Returns current barometric pressure (navX MXP Aero only).
 */
float AHRS::GetBarometricPressure()
{
	return barometric_pressure;
}

/**
 * Returns the current altitude, based upon calibrated readings
 * from the onboard pressure sensor, and the currently-configured
 * sea-level barometric pressure.  This value is in units of meters.
 *
 * NOTE:  This value is only valid for a navX MXP Aero.  To determine
 * whether this value is valid, see isAltitudeValid().
 * @return Returns current altitude (navX MXP Aero only).
 */
float AHRS::GetAltitude()
{
	return altitude;
}

/**
 * Indicates whether the current altitude (and barometric pressure) data is
 * valid. This value will only be true for a navX MXP Aero.
 *
 * If this value is false for a navX MXP Aero, this indicates a malfunction
 * of the onboard pressure sensor.
 * @return Returns current altitude (navX MXP Aero only).
 */
bool AHRS::IsAltitudeValid()
{
	return this->altitude_valid;
}

/**
 * Returns the "fused" (9-axis) heading.
 *
 * The 9-axis heading is the fusion of the yaw angle, the tilt-corrected
 * compass heading, and magnetic disturbance detection.  Note that the
 * navX MXP magnetometer calibration procedure is required in order to
 * achieve valid 9-axis headings.
 *
 * The 9-axis Heading represents the navX MXP's best estimate of current heading,
 * based upon the last known valid Compass Angle, and updated by the change in the
 * Yaw Angle since the last known valid Compass Angle.  The last known valid Compass
 * Angle is updated whenever a Calibrated Compass Angle is read and the navX MXP
 * has recently rotated less than the Compass Noise Bandwidth (~2 degrees).
 * @return
 */
float AHRS::GetFusedHeading()
{
	return fused_heading;
}

/**
 * Indicates whether the current magnetic field strength diverges from the
 * calibrated value for the earth's magnetic field by more than the currently-
 * configured Magnetic Disturbance Ratio.
 *
 * This function will always return false if the navX MXP magnetometer has
 * not yet been calibrated; see isMagnetometerCalibrated().
 * @return
 */
bool AHRS::IsMagneticDisturbance()
{
	return magnetic_disturbance;
}

/**
 * Indicates whether the navX MXP magnetometer has been calibrated.
 *
 * Magnetometer Calibration must be performed by the user.
 *
 * Note that if this function does indicate the magnetometer is calibrated,
 * this does not necessarily mean that the calibration quality is sufficient
 * to yield valid compass headings.
 * @return
 */
bool AHRS::IsMagnetometerCalibrated()
{
	return is_magnetometer_calibrated;
}

/**
 * Returns the current temperature (in degrees centigrade) reported by
 * the nav6 gyro/accelerometer circuit.
 *
 * This value may be useful in order to perform advanced temperature-
 * dependent calibration.
 * @return The current temperature (in degrees centigrade).
 */
float AHRS::GetTempC()
{
	return this->mpu_temp_c;
}

/**
 * Returns the current calibrated magnetometer sensor reading.
 * @return the current calibrated magnetometer reading, in device units.
 */
short AHRS::GetCalibratedMagnetometerX()
{
	return this->cal_mag_x;
}

/**
 * Returns the current calibrated magnetometer sensor reading.
 * @return the current calibrated magnetometer reading, in device units.
 */
short AHRS::GetCalibratedMagnetometerY()
{
	return this->cal_mag_y;
}

/**
 * Returns the current calibrated magnetometer sensor reading.
 * @return the current calibrated magnetometer reading, in device units.
 */
short AHRS::GetCalibratedMagnetometerZ()
{
	return this->cal_mag_z;
}

/**
 * Zeros the displacement integration variables.   Invoke this at the moment when
 * integration begins.
 * @return none.
 */
void AHRS::ResetDisplacement()
{
	for ( int i = 0; i < 2; i++ ) {
		last_velocity[i] = 0.0f;
		displacement[i] = 0.0f;
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
								int update_rate_hz, bool is_moving )
{
	if ( is_moving ) {
		float accel_g[2];
		float accel_m_s2[2];
		float curr_velocity_m_s[2];
		float sample_time = (1.0f / update_rate_hz);
		accel_g[0] = accel_x_g;
		accel_g[1] = accel_y_g;
		for ( int i = 0; i < 2; i++ ) {
			accel_m_s2[i] = accel_g[i] * 9.80665f;
			curr_velocity_m_s[i] = last_velocity[i] + (accel_m_s2[i] * sample_time);
			displacement[i] += last_velocity[i] + (0.5f * accel_m_s2[i] * sample_time * sample_time);
			last_velocity[i] = curr_velocity_m_s[i];
		}
	} else {
		last_velocity[0] = 0.0f;
		last_velocity[1] = 0.0f;
	}
}

/**
 * Returns the velocity (in meters/sec) of the X axis.
 * @return none.
 */
float AHRS::GetVelocityX()
{
	return last_velocity[0];
}

/**
 * Returns the velocity (in meters/sec) of the Y axis.
 * @return none.
 */
float AHRS::GetVelocityY()
{
	return last_velocity[1];
}

/**
 * Returns the displacement (in meters) of the X axis since resetDisplacement()
 * was invoked.
 * @return none.
 */
float AHRS::GetDisplacementX()
{
	return displacement[0];
}

/**
 * Returns the displacement (in meters) of the Y axis since resetDisplacement()
 * was invoked.
 * @return none.
 */
float AHRS::GetDisplacementY()
{
	return displacement[1];
}


