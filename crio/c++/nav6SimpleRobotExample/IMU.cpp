/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs. All Rights Reserved.							  */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Based upon the Open Source WPI Library released by FIRST robotics.         */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "NetworkCommunication/UsageReporting.h"
#include "Timer.h"
#include "WPIErrors.h"
#include "LiveWindow/LiveWindow.h"
#include <time.h>
#include "IMU.h"
#include "IMUProtocol.h"
#include "Synchronized.h"

static SEM_ID cIMUStateSemaphore = semBCreate (SEM_Q_PRIORITY, SEM_FULL);   
static int update_count = 0;
static int byte_count = 0;

static bool stop = false;

/*** Internal task.
 * 
 * Task which retrieves yaw/pitch/roll updates from the IMU, via the
 * SerialPort.
 **/ 

static char protocol_buffer[1024];

static void imuTask(IMU *imu) 
{
    bool stream_response_received = false;
    double last_stream_command_sent_timestamp = 0.0;
	stop = false;
	SerialPort *pport = imu->GetSerialPort();
	pport->SetReadBufferSize(512);
	pport->SetTimeout(1.0);
	pport->EnableTermination('\n');
	pport->Flush();
	pport->Reset();

	char stream_type;
	uint16_t gyro_fsr_dps, accel_fsr, update_rate_hz;
	uint16_t q1_offset, q2_offset, q3_offset, q4_offset;
	float yaw_offset_degrees;
	uint16_t flags;
	
    // Give the nav6 circuit a few seconds to initialize, then send the stream configuration command.
    int cmd_packet_length = IMUProtocol::encodeStreamCommand( protocol_buffer, imu->current_stream_type, imu->update_rate_hz ); 
    pport->Reset();
    pport->Write( protocol_buffer, cmd_packet_length );
    pport->Flush();
    last_stream_command_sent_timestamp = Timer::GetFPGATimestamp();
		
	while (!stop)
	{ 
		UINT32 bytes_read = pport->Read( protocol_buffer, 256 );
		if ( bytes_read > 0 )
		{
			int packets_received = 0;
			byte_count += bytes_read;
			UINT32 i = 0;
			// Scan the buffer looking for valid packets
			while ( i < bytes_read )
			{
				int bytes_remaining = bytes_read - i;
				int packet_length = imu->DecodePacketHandler( &protocol_buffer[i], bytes_remaining ); 
				if ( packet_length > 0 )
				{
					packets_received++;
					update_count++;
					i += packet_length;
				}
				else 
				{
					packet_length = IMUProtocol::decodeStreamResponse( &protocol_buffer[i], bytes_remaining, stream_type,
							  gyro_fsr_dps, accel_fsr, update_rate_hz,
							  yaw_offset_degrees, 
							  q1_offset, q2_offset, q3_offset, q4_offset,
							  flags );
					if ( packet_length > 0 ) 
					{
						packets_received++;
						imu->SetStreamResponse( stream_type, 
								  gyro_fsr_dps, accel_fsr, update_rate_hz,
								  yaw_offset_degrees, 
								  q1_offset, q2_offset, q3_offset, q4_offset,
								  flags );
						stream_response_received = true;
						i += packet_length;
					}
					else // current index is not the start of a valid packet we're interested in; increment
					{
						i++;
					}
				}
			}
			if ( ( packets_received == 0 ) && ( bytes_read == 256 ) ) {
				// No packets received and 256 bytes received; this
				// condition occurs in the SerialPort.  In this case,
				// reset the serial port.
				pport->Reset();
			}
			
			if ( !stream_response_received && ((Timer::GetFPGATimestamp() - last_stream_command_sent_timestamp ) > 3.0 ) ) {
				cmd_packet_length = IMUProtocol::encodeStreamCommand( protocol_buffer, imu->current_stream_type, imu->update_rate_hz ); 
				last_stream_command_sent_timestamp = Timer::GetFPGATimestamp();
				pport->Write( protocol_buffer, cmd_packet_length );
				pport->Flush();
			}	            				
		}
		else {
			// This exception typically indicates a Timeout
			stream_response_received = false;
		}
	}
}

int IMU::DecodePacketHandler( char *received_data, int bytes_remaining ) {
	
	float yaw = 0.0;
	float pitch = 0.0;
	float roll = 0.0;
	float compass_heading = 0.0;		

	int packet_length = IMUProtocol::decodeYPRUpdate( received_data, bytes_remaining, yaw, pitch, roll, compass_heading ); 
	if ( packet_length > 0 )
	{
		SetYawPitchRoll(yaw,pitch,roll,compass_heading);
	}
	return packet_length;
}

void IMU::InternalInit( SerialPort *pport, uint8_t update_rate_hz, char stream_type ) {
	current_stream_type = stream_type;
	yaw_offset_degrees = 0;
	accel_fsr_g = 2;
	gyro_fsr_dps = 2000;
	flags = 0;
	this->update_rate_hz = update_rate_hz;
	m_task = new Task("IMU", (FUNCPTR)imuTask,Task::kDefaultPriority+1);
	yaw = 0.0;
	pitch = 0.0;
	roll = 0.0;
	pserial_port = pport;
	pserial_port->Reset();
	InitIMU();
	m_task->Start((UINT32)this);		
}

IMU::IMU( SerialPort *pport, uint8_t update_rate_hz, char stream_type ) {
	InternalInit(pport,update_rate_hz,stream_type);
}

IMU::IMU( SerialPort *pport, uint8_t update_rate_hz )
{
	InternalInit(pport,update_rate_hz,STREAM_CMD_STREAM_TYPE_YPR);
}

/**
 * Initialize the IMU.
 */
void IMU::InitIMU()
{
	// The IMU serial port configuration is 8 data bits, no parity, one stop bit. 
	// No flow control is used.
	// Conveniently, these are the defaults used by the WPILib's SerialPort class.
	//
	// In addition, the WPILib's SerialPort class also defaults to:
	//
	// Timeout period of 5 seconds
	// Termination ('\n' character)
	// Transmit immediately
	InitializeYawHistory();
	yaw_offset = 0;
	
	// set the nav6 into "YPR" update mode
	
	int packet_length = IMUProtocol::encodeStreamCommand( protocol_buffer, current_stream_type, update_rate_hz ); 
	pserial_port->Write( protocol_buffer, packet_length );	
}

/**
 * Delete the IMU.
 */
IMU::~IMU()
{
	m_task->Stop();
	delete m_task;
	m_task = 0;
}

void IMU::Restart()
{
	stop = true;
	pserial_port->Reset();
	m_task->Stop();
	
	pserial_port->Reset();
	InitializeYawHistory();
	update_count = 0;
	byte_count = 0;
	m_task->Restart();
}

bool IMU::IsConnected()
{
	double time_since_last_update = Timer::GetPPCTimestamp() - this->last_update_time;
	return time_since_last_update <= 1.0;
}

double IMU::GetByteCount()
{
	return byte_count;
}
double IMU::GetUpdateCount()
{
	return update_count;
}

bool IMU::IsCalibrating()
{
	Synchronized sync(cIMUStateSemaphore);
	uint16_t calibration_state = this->flags & NAV6_FLAG_MASK_CALIBRATION_STATE;
	return (calibration_state != NAV6_CALIBRATION_STATE_COMPLETE);
}

void IMU::ZeroYaw()
{
	yaw_offset = GetAverageFromYawHistory();
}


/**
 * Return the yaw angle in degrees.
 * 
 * This angle increases as the robot spins to the right.
 * 
 * This angle ranges from -180 to 180 degrees.
 */
float IMU::GetYaw( void )
{
	Synchronized sync(cIMUStateSemaphore);
	double yaw = this->yaw;
	yaw -= yaw_offset;
	if ( yaw < -180 ) yaw += 360;
	if ( yaw > 180 ) yaw -= 360;
	return yaw;
}

float IMU::GetPitch( void )
{
	Synchronized sync(cIMUStateSemaphore);
	return this->pitch;
}

float IMU::GetRoll( void )
{
	Synchronized sync(cIMUStateSemaphore);
	return this->roll;
}

float IMU::GetCompassHeading( void )
{
	Synchronized sync(cIMUStateSemaphore);
	return this->compass_heading;
}

/**
 * Get the angle in degrees for the PIDSource base object.
 * 
 * @return The angle in degrees.
 */
double IMU::PIDGet()
{
	return GetYaw();
}

void IMU::UpdateTable() {
	if (m_table != NULL) {
		m_table->PutNumber("Value", GetYaw());
	}
}

void IMU::StartLiveWindowMode() {
	
}

void IMU::StopLiveWindowMode() {
	
}

std::string IMU::GetSmartDashboardType() {
	return "Gyro";
}

void IMU::InitTable(ITable *subTable) {
	m_table = subTable;
	UpdateTable();
}

ITable * IMU::GetTable() {
	return m_table;
}

void IMU::SetYawPitchRoll(float yaw, float pitch, float roll, float compass_heading)
{
	{
		Synchronized sync(cIMUStateSemaphore);
		
		this->yaw = yaw;
		this->pitch = pitch;
		this->roll = roll;
		this->compass_heading = compass_heading;
	}
	UpdateYawHistory(this->yaw);
}

void IMU::InitializeYawHistory()
{
	memset(yaw_history,0,sizeof(yaw_history));
	next_yaw_history_index = 0;
	last_update_time = 0.0;
}

void IMU::UpdateYawHistory(float curr_yaw )
{
	if ( next_yaw_history_index >= YAW_HISTORY_LENGTH )
	{
		next_yaw_history_index = 0;
	}
	yaw_history[next_yaw_history_index] = curr_yaw;
	last_update_time = Timer::GetPPCTimestamp();
	next_yaw_history_index++;
}

double IMU::GetAverageFromYawHistory()
{
	Synchronized sync(cIMUStateSemaphore);
	double yaw_history_sum = 0.0;
	for ( int i = 0; i < YAW_HISTORY_LENGTH; i++ )
	{
		yaw_history_sum += yaw_history[i];
	}	
	double yaw_history_avg = yaw_history_sum / YAW_HISTORY_LENGTH;
	return yaw_history_avg;
}

void IMU::SetStreamResponse( char stream_type, 
								uint16_t gyro_fsr_dps, uint16_t accel_fsr, uint16_t update_rate_hz,
								float yaw_offset_degrees, 
								uint16_t q1_offset, uint16_t q2_offset, uint16_t q3_offset, uint16_t q4_offset,
								uint16_t flags )
{
	{
		Synchronized sync(cIMUStateSemaphore);
		this->yaw_offset_degrees = yaw_offset_degrees;
		this->flags = flags;
		this->accel_fsr_g = accel_fsr;
		this->gyro_fsr_dps = gyro_fsr_dps;
		this->update_rate_hz = update_rate_hz;
	}		
}


