/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015 Kauai Labs. All Rights Reserved.						  */
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
#include "AHRSProtocol.h"
#include <string.h>
#include <exception>

static ReentrantSemaphore cIMUStateSemaphore;
static int update_count = 0;
static int byte_count = 0;

static bool stop = false;

/*** Internal task.
 * 
 * Task which retrieves yaw/pitch/roll updates from the IMU, via the
 * SerialPort.
 **/ 

static char protocol_buffer[1024];
static char additional_received_data[1024];
static bool signal_transmit_integration_control = false;
static uint8_t next_integration_control_action = 0;
static bool signal_retransmit_stream_config = false;

#define DEBUG_IMU_RX  // Comment out to disable receive debugging

static void imuTask(IMU *imu)
{
	char stream_type;
	uint16_t gyro_fsr_dps, accel_fsr, update_rate_hz;
	uint16_t q1_offset, q2_offset, q3_offset, q4_offset;
	float yaw_offset_degrees;
	uint16_t flags;
    bool stream_response_received = false;
    double last_valid_packet_time = 0.0;
    int partial_binary_packet_count = 0;
    int stream_config_request_count = 0;
    int stream_response_receive_count = 0;
    int stream_response_timeout_count = 0;
    int timeout_count = 0;
    int discarded_bytes_count = 0;
    int port_reset_count = 0;
    double last_stream_command_sent_timestamp = 0.0;
    int updates_in_last_second = 0;
    double last_second_start_time = 0;
    int integration_response_receive_count = 0;

	SerialPort *pport = imu->GetSerialPort();
    try {
		pport->SetReadBufferSize(512);
		pport->SetTimeout(1.0);
		pport->EnableTermination('\n');
		pport->Flush();
		pport->Reset();
    } catch(std::exception ex) {
    }

    int cmd_packet_length = IMUProtocol::encodeStreamCommand( protocol_buffer,
    							imu->current_stream_type, imu->update_rate_hz );

    try {
        pport->Reset();
        pport->Write( protocol_buffer, cmd_packet_length );
        pport->Flush();
#ifdef DEBUG_IMU_RX
        port_reset_count++;
        SmartDashboard::PutNumber("nav6_PortResets", (double)port_reset_count);
#endif
        last_stream_command_sent_timestamp = Timer::GetFPGATimestamp();
    } catch (std::exception ex) {
    }

    while (!stop) {
        try {

            // Wait, with delays to conserve CPU resources, until
            // bytes have arrived.
        	bool send_integration_control = false;
        	uint8_t integration_control_action;
        	{
        		Synchronized sync(cIMUStateSemaphore);
				if ( signal_transmit_integration_control ) {
					send_integration_control = signal_transmit_integration_control;
					integration_control_action = next_integration_control_action;
					signal_transmit_integration_control = false;
					next_integration_control_action = 0;
        		}
        	}

        	if ( send_integration_control ) {
                int cmd_packet_length = AHRSProtocol::encodeIntegrationControlCmd( protocol_buffer, integration_control_action, 0 );
                try {
                    pport->Write( protocol_buffer, cmd_packet_length );
                } catch (std::exception ex2) {
                }
        	}

            while ( !stop && ( pport->GetBytesReceived() < 1 ) ) {
            	delayMillis(1000/update_rate_hz);
            }

            int packets_received = 0;
    		uint32_t bytes_read = pport->Read( protocol_buffer, 256 );
            if (bytes_read > 0) {
                byte_count += bytes_read;
                uint32_t i = 0;
                // Scan the buffer looking for valid packets
                while (i < bytes_read) {

                    // Attempt to decode a packet

                    int bytes_remaining = bytes_read - i;

                	if ( protocol_buffer[i] != PACKET_START_CHAR ) {
                		/* Skip over received bytes until a packet start is detected. */
                		i++;
#ifdef DEBUG_IMU_RX
                		discarded_bytes_count++;
    			        SmartDashboard::PutNumber("nav6 Discarded Bytes", (double)discarded_bytes_count);
#endif
    			        continue;
                	} else {
                		if ( ( bytes_remaining > 2 ) &&
                			 ( protocol_buffer[i+1] == '#' ) ) {
                			/* Binary packet received; next byte is packet length-2 */
                			uint8_t total_expected_binary_data_bytes = protocol_buffer[i+2];
                			total_expected_binary_data_bytes += 2;
                			while ( bytes_remaining < total_expected_binary_data_bytes ) {
                 				/* This binary packet contains an embedded     */
                				/* end-of-line character.  Continue to receive */
                				/* more data until entire packet is received.  */
                				uint32_t additional_received_data_length = pport->Read(additional_received_data, 256);
                				if ( additional_received_data_length > 0 ) {
									byte_count += additional_received_data_length;
									memcpy(protocol_buffer + bytes_read, additional_received_data, additional_received_data_length);
									bytes_read += additional_received_data_length;
									bytes_remaining += additional_received_data_length;
                				} else {
                					/* Timeout waiting for remainder of binary packet */
                					i++;
                					bytes_remaining--;
#ifdef DEBUG_IMU_RX
                			        partial_binary_packet_count++;
                			        SmartDashboard::PutNumber("nav6 Partial Binary Packets", (double)partial_binary_packet_count);
#endif
                			        continue;
                				}
                			}
                		}
                	}

                    int packet_length = imu->DecodePacketHandler(&protocol_buffer[i],bytes_remaining);
                    if (packet_length > 0) {
                        packets_received++;
                        update_count++;
                        last_valid_packet_time = Timer::GetFPGATimestamp();
                        updates_in_last_second++;
                        if ((last_valid_packet_time - last_second_start_time ) > 1.0 ) {
#ifdef DEBUG_IMU_RX
                        	SmartDashboard::PutNumber("nav6 UpdatesPerSec", (double)updates_in_last_second);
                        	updates_in_last_second = 0;
#endif
                        	last_second_start_time = last_valid_packet_time;
                        }
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
#ifdef DEBUG_IMU_RX
                            stream_response_receive_count++;
                        	SmartDashboard::PutNumber("nav6 Stream Responses", (double)stream_response_receive_count);
#endif
                        }
                        else {
                        	uint8_t action;
                        	int32_t parameter;
        					packet_length = AHRSProtocol::decodeIntegrationControlResponse( &protocol_buffer[i], bytes_remaining,
        							  action, parameter );
        					if ( packet_length > 0 ) {
        						// Confirmation of integration control
        						integration_response_receive_count++;
#ifdef DEBUG_IMU_RX
        						SmartDashboard::PutNumber("IntegrationControlRxCount", integration_response_receive_count);
#endif
                                i += packet_length;
        					} else {
                        	// current index is not the start of a valid packet; increment
								i++;
#ifdef DEBUG_IMU_RX
								discarded_bytes_count++;
								SmartDashboard::PutNumber("nav6 Discarded Bytes", (double)discarded_bytes_count);
#endif
        					}
        				}
                    }
                }

                if ( ( packets_received == 0 ) && ( bytes_read == 256 ) ) {
                    // Workaround for issue found in SerialPort implementation:
                    // No packets received and 256 bytes received; this
                    // condition occurs in the SerialPort.  In this case,
                    // reset the serial port.
                    pport->Reset();
#ifdef DEBUG_IMU_RX
                    port_reset_count++;
    		        SmartDashboard::PutNumber("nav6_PortResets", (double)port_reset_count);
#endif
                }

                bool retransmit_stream_config = false;
                if ( signal_retransmit_stream_config ) {
                	signal_retransmit_stream_config = false;
                	retransmit_stream_config = true;
                }

                // If a stream configuration response has not been received within three seconds
                // of operation, (re)send a stream configuration request

                if ( retransmit_stream_config ||
                		(!stream_response_received && ((Timer::GetFPGATimestamp() - last_stream_command_sent_timestamp ) > 3.0 ) ) ) {
                    int cmd_packet_length = IMUProtocol::encodeStreamCommand( protocol_buffer, imu->current_stream_type, imu->update_rate_hz );
                    try {
                        last_stream_command_sent_timestamp = Timer::GetFPGATimestamp();
                        pport->Write( protocol_buffer, cmd_packet_length );
                        pport->Flush();
#ifdef DEBUG_IMU_RX
                        stream_config_request_count++;
                    	SmartDashboard::PutNumber("nav6 Stream Config Request sent ", (double)stream_config_request_count);
#endif
                    } catch (std::exception ex2) {
#ifdef DEBUG_IMU_RX
                    	stream_response_timeout_count++;
                    	SmartDashboard::PutNumber("nav6 Stream Response Timeouts", (double)stream_response_timeout_count);
#endif
                    }
                }
                else {
                    // If no bytes remain in the buffer, and not awaiting a response, sleep a bit
                    if ( stream_response_received && ( pport->GetBytesReceived() == 0 ) ) {
                    	delayMillis(1000/update_rate_hz);
                    }
                }

                /* If receiving data, but no valid packets have been received in the last second */
                /* the navX MXP may have been reset, but no exception has been detected.         */
                /* In this case , trigger transmission of a new stream_command, to ensure the    */
                /* streaming packet type is configured correctly.                                */

                if ( ( Timer::GetFPGATimestamp() - last_valid_packet_time ) > 1.0 ) {
                	last_stream_command_sent_timestamp = 0.0;
                	stream_response_received = false;
                }
            }
        } catch (std::exception ex) {
            // This exception typically indicates a Timeout
            stream_response_received = false;
#ifdef DEBUG_IMU_RX
	        timeout_count++;
	        SmartDashboard::PutNumber("nav6 Serial Port Timeouts", (double)timeout_count);
            SmartDashboard::PutString("LastNavException", ex.what());
#endif
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
    yaw_crossing_count = 0;
    yaw_last_direction = 0;
    last_yaw_rate = 0.0f;
	pserial_port = pport;
	pserial_port->Reset();
	InitIMU();
	m_task->Start((uint32_t)this);
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
	double time_since_last_update = Timer::GetFPGATimestamp() - this->last_update_time;
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
	if ( this->IsBoardYawResetSupported() ) {
		/* navX MXP supports on-board yaw offset reset */
		EnqueueIntegrationControlMessage( NAVX_INTEGRATION_CTL_RESET_YAW );
	} else {
		yaw_offset = GetAverageFromYawHistory();
	}
}

float IMU::GetYawUnsynchronized()
{
	double yaw = this->yaw;
	yaw -= yaw_offset;
	if ( yaw < -180 ) yaw += 360;
	if ( yaw > 180 ) yaw -= 360;
	return yaw;
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
	return GetYawUnsynchronized();
}

/**
 * Returns the total accumulated yaw angle (Z Axis, in degrees)
 * reported by the navX MXP.
 *
 * The angle is continuous, that is can go beyond 360 degrees. This make algorithms that wouldn't
 * want to see a discontinuity in the gyro output as it sweeps past 0 on the second time around.
 *
 * Note that the returned yaw value will be offset by a user-specified
 * offset value; this user-specified offset value is set by
 * invoking the zeroYaw() method.
 *
 * Returns the current heading of the robot in degrees. This heading is based on integration
 * of the returned rate from the gyro.
 */

double IMU::GetAngle()
{
	double accumulated_yaw_angle = (double)yaw_crossing_count * 360.0f;
	double curr_yaw = GetYaw();
	if ( curr_yaw < 0.0f ) {
		curr_yaw += 360.0f;
	}
	accumulated_yaw_angle += curr_yaw;
	return accumulated_yaw_angle;
}

/**
 * Return the rate of rotation of the gyro.
 *
 * The rate is based on the most recent reading of the gyro analog value.
 *
 * Returns the current rate in degrees per second
 */

double IMU::GetRate()
{
	return last_yaw_rate * (float)update_rate_hz;
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
		
    	float last_offset_corrected_yaw = GetYawUnsynchronized();

    	float last_yaw = this->yaw + 180.0f;
    	float curr_yaw = yaw + 180.0f;
        float delta_yaw = curr_yaw - last_yaw;
        this->last_yaw_rate = delta_yaw;

        yaw_last_direction = 0;
    	if ( curr_yaw < last_yaw ) {
    		if ( delta_yaw < 180.0f ) {
    			yaw_last_direction = -1;
    		} else {
    			yaw_last_direction = 1;
    		}
    	} else if ( curr_yaw > last_yaw ) {
    		if ( delta_yaw > 180.0f ) {
    			yaw_last_direction = -1;
    		} else {
    			yaw_last_direction = 1;
    		}
    	}

		this->yaw = yaw;
		this->pitch = pitch;
		this->roll = roll;
		this->compass_heading = compass_heading;

    	float curr_offset_corrected_yaw = GetYawUnsynchronized();

    	if ( yaw_last_direction < 0 ) {
			if ( ( curr_offset_corrected_yaw < 0.0f ) && ( last_offset_corrected_yaw >= 0.0f ) ) {
				yaw_crossing_count--;
			}
    	} else if ( yaw_last_direction > 0 ) {
			if ( ( curr_offset_corrected_yaw >= 0.0f ) && ( last_offset_corrected_yaw < 0.0f ) ) {
				yaw_crossing_count++;
			}
    	}

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
	last_update_time = Timer::GetFPGATimestamp();
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
        /* If AHRSPOS is update type is request, but board doesn't support it, */
        /* retransmit the stream config, falling back to AHRS Update mode.     */
        if ( current_stream_type == MSGID_AHRSPOS_UPDATE ) {
        	if ( !IsDisplacementSupported() ) {
        		current_stream_type = MSGID_AHRS_UPDATE;
        		signal_retransmit_stream_config = true;
        	}
        }

	}		
}

bool IMU::IsOmniMountSupported()
{
	return ((this->flags & NAVX_CAPABILITY_FLAG_OMNIMOUNT) ? true : false);
}

bool IMU::IsBoardYawResetSupported()
{
	return ((this->flags & NAVX_CAPABILITY_FLAG_YAW_RESET) ? true : false);
}

bool IMU::IsDisplacementSupported()
{
	return ((this->flags & NAVX_CAPABILITY_FLAG_VEL_AND_DISP) ? true : false);
}

void IMU::EnqueueIntegrationControlMessage(uint8_t action)
{
	Synchronized sync(cIMUStateSemaphore);
	signal_transmit_integration_control = true;
	next_integration_control_action = action;
}

uint8_t IMU::GetBoardYawAxis( bool& up)
{
	uint16_t yaw_axis_info = (flags >> 2) & 0x0007;
	uint8_t yaw_axis;
	if ( yaw_axis_info == OMNIMOUNT_DEFAULT) {
		yaw_axis = YAW_AXIS_Z;
		up = true;
	} else {
		yaw_axis = (uint8_t)yaw_axis_info;
		yaw_axis -= 1;
		up = ((yaw_axis & 0x01) ? false : true);
		yaw_axis >>= 1;
	}
	return yaw_axis;
}



