/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015 Kauai Labs. All Rights Reserved.						  */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Based upon the Open Source WPI Library released by FIRST robotics.         */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef AHRS_H_
#define AHRS_H_

#include "IMU.h"

/**
 * Use the IMU to retrieve a Yaw/Pitch/Roll measurement.
 * 
 * This utilizes the Kauai Labs Nav6 IMU.
 * 
 * This IMU interfaces to the CRio processor via a Serial port.
 */

#define YAW_HISTORY_LENGTH 10

class AHRS : public IMU
{
protected:
	SerialPort *pserial_port;
	void InternalInit( SerialPort *pport, uint8_t update_rate_hz, char stream_type );
public:

	AHRS( SerialPort *pport, uint8_t update_rate_hz = 60 );
	virtual ~AHRS();

	float GetWorldLinearAccelX();
	float GetWorldLinearAccelY();
	float GetWorldLinearAccelZ();
	bool  IsMoving();
	bool  IsRotating();
	float GetBarometricPressure();
	float GetAltitude();
	bool  IsAltitudeValid();
	float GetFusedHeading();
	bool  IsMagneticDisturbance();
	bool  IsMagnetometerCalibrated();
	float GetTempC();
	short GetCalibratedMagnetometerX();
	short GetCalibratedMagnetometerY();
	short GetCalibratedMagnetometerZ();

	void  ResetDisplacement();
	float GetVelocityX();
	float GetVelocityY();
	float GetDisplacementX();
	float GetDisplacementY();

protected:

	virtual int DecodePacketHandler( char *received_data, int bytes_remaining );

    volatile float world_linear_accel_x;
    volatile float world_linear_accel_y;
    volatile float world_linear_accel_z;
    volatile float mpu_temp_c;
    volatile float fused_heading;
    volatile float altitude;
    volatile float barometric_pressure;
    volatile bool is_moving;
    volatile bool is_rotating;
    volatile float baro_sensor_temp_c;
    volatile short cal_mag_x;
    volatile short cal_mag_y;
    volatile short cal_mag_z;
    volatile float mag_field_norm_ratio;
    volatile bool altitude_valid;
    volatile bool is_magnetometer_calibrated;
    volatile bool magnetic_disturbance;

    void UpdateDisplacement( float accel_x_g, float accel_y_g,
    							int update_rate_hz, bool is_moving );

    float last_velocity[2];
    float displacement[2];

};
#endif
