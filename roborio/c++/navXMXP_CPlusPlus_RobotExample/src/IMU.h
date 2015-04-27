/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs. All Rights Reserved.							  */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Based upon the Open Source WPI Library released by FIRST robotics.         */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef IMU_H_
#define IMU_H_

#include "SensorBase.h"
#include "PIDSource.h"
#include "LiveWindow/LiveWindowSendable.h"
#include "SerialPort.h"
#include "Task.h"
/**
 * Use the IMU to retrieve a Yaw/Pitch/Roll measurement.
 * 
 * This utilizes the Kauai Labs Nav6 IMU.
 * 
 * This IMU interfaces to the CRio processor via a Serial port.
 */

#define YAW_HISTORY_LENGTH 10

class IMU : public SensorBase, public PIDSource, public LiveWindowSendable
{
protected:
	SerialPort *pserial_port;
	IMU( SerialPort *pport, uint8_t update_rate_hz, char stream_type );
	void InternalInit( SerialPort *pport, uint8_t update_rate_hz, char stream_type );
public:

	IMU( SerialPort *pport, uint8_t update_rate_hz = 100 );
	virtual ~IMU();
	virtual float GetPitch();	// Pitch, in units of degrees (-180 to 180)
	virtual float GetRoll();	// Roll, in units of degrees (-180 to 180)
	virtual float GetYaw();		// Yaw, in units of degrees (-180 to 180)
	virtual float GetCompassHeading(); // CompassHeading, in units of degrees (0 to 360)
	
	bool IsConnected();
	void ZeroYaw();
	
	// PIDSource interface, returns yaw component in units of degrees (-180 to 180)
	double PIDGet();
	
	void UpdateTable();
	void StartLiveWindowMode();
	void StopLiveWindowMode();
	std::string GetSmartDashboardType();
	void InitTable(ITable *subTable);
	ITable * GetTable();

	SerialPort *GetSerialPort() { return pserial_port; }
	void SetYawPitchRoll(float yaw, float pitch, float roll, float compass_heading);
	void SetStreamResponse( char stream_type, 
							uint16_t gyro_fsr_dps, uint16_t accel_fsr_g, uint16_t update_rate_hz,
							float yaw_offset_degrees, 
							uint16_t q1_offset, uint16_t q2_offset, uint16_t q3_offset, uint16_t q4_offset,
							uint16_t flags );
	double GetYawOffset() { return yaw_offset; }
	double GetByteCount();
	double GetUpdateCount();
	void Restart();
	bool  IsCalibrating();

	uint8_t update_rate_hz;	
	char current_stream_type;
	virtual int DecodePacketHandler( char *received_data, int bytes_remaining );
	
private:
	void InitializeYawHistory();
	double GetAverageFromYawHistory();
	void InitIMU();
	

protected:

	void UpdateYawHistory(float curr_yaw );
	
	Task *	m_task;
	float 	yaw;
	float 	pitch; 
	float 	roll;
	float   compass_heading;
	float 	yaw_history[YAW_HISTORY_LENGTH];
	int 	next_yaw_history_index;
	double 	last_update_time;
	double 	yaw_offset;
	float   yaw_offset_degrees;
	uint16_t accel_fsr_g;
	uint16_t gyro_fsr_dps;
	uint16_t flags;
    
	ITable *m_table;
};
#endif
