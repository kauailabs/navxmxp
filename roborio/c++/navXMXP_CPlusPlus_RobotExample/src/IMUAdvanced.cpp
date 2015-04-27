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
#include "IMUAdvanced.h"
#include "IMUProtocol.h"
#include <math.h>
#include <string.h>

int IMUAdvanced::DecodePacketHandler( char *received_data, int bytes_remaining ) {
	
	int16_t q1, q2, q3, q4;
	int16_t accel_x, accel_y, accel_z;
	int16_t mag_x, mag_y, mag_z;
	float temp_c;

	int packet_length = IMUProtocol::decodeQuaternionUpdate( received_data, bytes_remaining, 
			q1,q2,q3,q4,accel_x,accel_y,accel_z,mag_x,mag_y,mag_z,temp_c ); 
	if ( packet_length > 0 ) {
		SetQuaternion(q1,q2,q3,q4,accel_x,accel_y,accel_z,mag_x,mag_y,mag_z,temp_c);
	}
	return packet_length;
}

IMUAdvanced::IMUAdvanced( SerialPort *pport, uint8_t update_rate_hz ) :
	IMU(pport,update_rate_hz,STREAM_CMD_STREAM_TYPE_QUATERNION)
{
	InitWorldLinearAccelHistory();
}

IMUAdvanced::~IMUAdvanced() {
	
}

void IMUAdvanced::InitWorldLinearAccelHistory()
{
	memset(world_linear_accel_history,0,sizeof(world_linear_accel_history));
	next_world_linear_accel_history_index = 0;
	world_linear_acceleration_recent_avg = 0.0;	
}

void IMUAdvanced::UpdateWorldLinearAccelHistory( float x, float y, float z )
{
	if ( next_world_linear_accel_history_index >= WORLD_LINEAR_ACCEL_HISTORY_LENGTH )
	{
		next_world_linear_accel_history_index = 0;
	}
	world_linear_accel_history[next_world_linear_accel_history_index] = fabs(x) + fabs(y);
	next_world_linear_accel_history_index++;
}

float IMUAdvanced::GetAverageFromWorldLinearAccelHistory()
{
	float world_linear_accel_history_avg = 0.0;
	for ( int i = 0; i < WORLD_LINEAR_ACCEL_HISTORY_LENGTH; i++ )
	{
		world_linear_accel_history_avg += world_linear_accel_history[i];
	}
	world_linear_accel_history_avg /= WORLD_LINEAR_ACCEL_HISTORY_LENGTH;
	return world_linear_accel_history_avg;
}

float IMUAdvanced::GetWorldLinearAccelX()
{
	return this->world_linear_accel_x;
}

float IMUAdvanced::GetWorldLinearAccelY()
{
	return this->world_linear_accel_y;	
}

float IMUAdvanced::GetWorldLinearAccelZ()
{
	return this->world_linear_accel_z;	
}

bool  IMUAdvanced::IsMoving()
{
	return (GetAverageFromWorldLinearAccelHistory() >= 0.01);
}

float IMUAdvanced::GetTempC()
{
	return this->temp_c;
}

void IMUAdvanced::SetQuaternion( int16_t quat1, int16_t quat2, int16_t quat3, int16_t quat4,
					int16_t accel_x, int16_t accel_y, int16_t accel_z,
					int16_t mag_x, int16_t mag_y, int16_t mag_z,
					float temp_c)
{
	{
		float q[4];				// Quaternion from IMU
		float gravity[3];		// Gravity Vector
		//float euler[3];			// Classic euler angle representation of quaternion
		float ypr[3];			// Angles in "Tait-Bryan" (yaw/pitch/roll) format
		float yaw_degrees;
		float pitch_degrees;
		float roll_degrees;
		float linear_acceleration_x;
		float linear_acceleration_y;
		float linear_acceleration_z;
        float q2[4];
        float q_product[4];
		float world_linear_acceleration_x;
		float world_linear_acceleration_y;
		float world_linear_acceleration_z;
        
		// Convert 15-bit signed quaternion integral values to floats
		q[0] = ((float)quat1) / 16384.0f;
		q[1] = ((float)quat2) / 16384.0f;
		q[2] = ((float)quat3) / 16384.0f;
        q[3] = ((float)quat4) / 16384.0f;
        
        // Fixup any quaternion values out of range
        for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i];

        // below calculations are necessary for calculation of yaw/pitch/roll, 
        // and tilt-compensated compass heading
        
        // calculate gravity vector
        gravity[0] = 2 * (q[1]*q[3] - q[0]*q[2]);
        gravity[1] = 2 * (q[0]*q[1] + q[2]*q[3]);
        gravity[2] = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
        
        // calculate Euler angles
        // This code is here for reference, and is commented out for performance reasons
        //euler[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
        //euler[1] = -asin(2*q[1]*q[3] + 2*q[0]*q[2]);
        //euler[2] = atan2(2*q[2]*q[3] - 2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3] - 1);

        // calculate yaw/pitch/roll angles
        ypr[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
        ypr[1] = atan(gravity[0] / sqrt(gravity[1]*gravity[1] + gravity[2]*gravity[2]));
        ypr[2] = atan(gravity[1] / sqrt(gravity[0]*gravity[0] + gravity[2]*gravity[2]));

        // Convert yaw/pitch/roll angles to degreess, and remove calibrated offset
        
        yaw_degrees = ypr[0] * (180.0/3.1415926); 
        pitch_degrees = ypr[1] * (180.0/3.1415926); 
        roll_degrees = ypr[2] * (180.0/3.1415926); 
         
        // Subtract offset, and handle potential 360 degree wrap-around
        yaw_degrees -= yaw_offset_degrees;
        if ( yaw_degrees < -180 ) yaw_degrees += 360;
        if ( yaw_degrees > 180 ) yaw_degrees -= 360;

        // calculate linear acceleration by 
        // removing the gravity component from raw acceleration values
        // Note that this code assumes the acceleration full scale range
        // is +/- 2 degrees
         
        linear_acceleration_x = (float)(((float)accel_x) / (32768.0 / (float)accel_fsr_g)) - gravity[0];
        linear_acceleration_y = (float)(((float)accel_y) / (32768.0 / (float)accel_fsr_g)) - gravity[1];
        linear_acceleration_z = (float)(((float)accel_z) / (32768.0 / (float)accel_fsr_g)) - gravity[2]; 
        
        // Calculate world-frame acceleration
        
        q2[0] = 0;
        q2[1] = linear_acceleration_x;
        q2[2] = linear_acceleration_y;
        q2[3] = linear_acceleration_z;
        
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
        
        q_product[0] = q[0]*q2[0] - q[1]*q2[1] - q[2]*q2[2] - q[3]*q2[3];  // new w
        q_product[1] = q[0]*q2[1] + q[1]*q2[0] + q[2]*q2[3] - q[3]*q2[2];  // new x
        q_product[2] = q[0]*q2[2] - q[1]*q2[3] + q[2]*q2[0] + q[3]*q2[1];  // new y 
        q_product[3] = q[0]*q2[3] + q[1]*q2[2] - q[2]*q2[1] + q[3]*q2[0];  // new z

        float q_conjugate[4];
        
        q_conjugate[0] = q[0];            
        q_conjugate[1] = -q[1];            
        q_conjugate[2] = -q[2];            
        q_conjugate[3] = -q[3];            

        float q_final[4];
        
        q_final[0] = q_product[0]*q_conjugate[0] - q_product[1]*q_conjugate[1] - q_product[2]*q_conjugate[2] - q_product[3]*q_conjugate[3];  // new w
        q_final[1] = q_product[0]*q_conjugate[1] + q_product[1]*q_conjugate[0] + q_product[2]*q_conjugate[3] - q_product[3]*q_conjugate[2];  // new x
        q_final[2] = q_product[0]*q_conjugate[2] - q_product[1]*q_conjugate[3] + q_product[2]*q_conjugate[0] + q_product[3]*q_conjugate[1];  // new y 
        q_final[3] = q_product[0]*q_conjugate[3] + q_product[1]*q_conjugate[2] - q_product[2]*q_conjugate[1] + q_product[3]*q_conjugate[0];  // new z

        world_linear_acceleration_x = q_final[1];
        world_linear_acceleration_y = q_final[2];
        world_linear_acceleration_z = q_final[3];
        
        // Calculate tilt-compensated compass heading
        
        float inverted_pitch = -ypr[1];
        float roll = ypr[2];
        
        float cos_roll = cos(roll);
        float sin_roll = sin(roll);
        float cos_pitch = cos(inverted_pitch);
        float sin_pitch = sin(inverted_pitch);
        
        float MAG_X = mag_x * cos_pitch + mag_z * sin_pitch;
        float MAG_Y = mag_x * sin_roll * sin_pitch + mag_y * cos_roll - mag_z * sin_roll * cos_pitch;
        float tilt_compensated_heading_radians = atan2(MAG_Y,MAG_X);
        float tilt_compensated_heading_degrees = tilt_compensated_heading_radians * (180.0 / 3.1415926);
        
        // Adjust compass for board orientation,
        // and modify range from -180-180 to
        // 0-360 degrees
      
        tilt_compensated_heading_degrees -= 90.0;
        if ( tilt_compensated_heading_degrees < 0 ) {
          tilt_compensated_heading_degrees += 360; 
        }

		this->yaw = yaw_degrees;
		this->pitch = pitch_degrees;
		this->roll = roll_degrees;
		this->compass_heading = tilt_compensated_heading_degrees;
        
		this->world_linear_accel_x = world_linear_acceleration_x;
		this->world_linear_accel_y = world_linear_acceleration_y;
		this->world_linear_accel_z = world_linear_acceleration_z;
		this->temp_c = temp_c;
		
		UpdateYawHistory(this->yaw);
		UpdateWorldLinearAccelHistory( world_linear_acceleration_x, world_linear_acceleration_y, world_linear_acceleration_z);
	}	
}
