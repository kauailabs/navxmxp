/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2013. All Rights Reserved.                       */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the nav6_License.txt file*/ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/

package com.kauailabs.nav6.frc;
import java.util.Arrays;

import com.kauailabs.nav6.IMUProtocol;

import edu.wpi.first.wpilibj.SerialPort;

/**
 * The IMUAdvanced class provides a simplified interface to advanced capabilities
 * of the KauaiLabs nav6 IMU.
 * 
 * The IMUAdvanced class enables access to basic connectivity and state information, 
 * as well as key orientation information (yaw, pitch, roll, compass heading).
 * Additionally, the IMUAdvanced class also provides access to extended information
 * including linear acceleration, motion detection, and sensor temperature.
 * @author Scott
 */public class IMUAdvanced extends IMU {

    private IMUProtocol.QuaternionUpdate quaternion_update_data;    
    volatile float world_linear_accel_x;
    volatile float world_linear_accel_y;
    volatile float world_linear_accel_z;
    volatile float temp_c;
    float world_linear_accel_history[];
    int   next_world_linear_accel_history_index;
    float world_linear_acceleration_recent_avg;
    
    static final int WORLD_LINEAR_ACCEL_HISTORY_LENGTH = 10;

    /**
     * Constructs the IMUAdvanced class, overriding the default update rate
     * with a custom rate which may be from 4 to 100, representing
     * the number of updates per second sent by the nav6 IMU.  
     * 
     * Note that increasing the update rate may increase the 
     * CPU utilization.  Note that calculation of some 
     * advanced values utilizes additional cpu cycles, when compared
     * to the IMU class.
     * @param serial_port BufferingSerialPort object to use
     * @param update_rate_hz Custom Update Rate (Hz)
     */
    public IMUAdvanced(SerialPort serial_port, byte update_rate_hz) {
        super(serial_port,update_rate_hz);
        quaternion_update_data = new IMUProtocol.QuaternionUpdate();
        update_type = IMUProtocol.MSGID_QUATERNION_UPDATE;
    }
    
    /**
     * Constructs the IMUAdvanced class, using the default update rate.  
     * 
     * Note that calculation of some advanced values utilizes additional 
     * cpu cycles, when compared to the IMU class.
     * @param serial_port BufferingSerialPort object to use
     */
    public IMUAdvanced(SerialPort serial_port) {
        this(serial_port, DEFAULT_UPDATE_RATE_HZ);
    }

    //@Override
    protected int decodePacketHandler(byte[] received_data, int offset, int bytes_remaining) {
        
        int packet_length = IMUProtocol.decodeQuaternionUpdate(received_data, offset, bytes_remaining, quaternion_update_data);
        if (packet_length > 0) {
            setQuaternion(quaternion_update_data);
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
    public float getWorldLinearAccelX()
    {
        return this.world_linear_accel_x;
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
    public float getWorldLinearAccelY()
    {
        return this.world_linear_accel_y;
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
    public float getWorldLinearAccelZ()
    {
        return this.world_linear_accel_z;
    }

    /**
     * Indicates if the nav6 IMU is currently detection motion,
     * based upon the x and y-axis world linear acceleration values.
     * If the sum of the absolute values of the x and y axis exceed,
     * 0.01g, the motion state is indicated.
     * @return Returns true if the nav6 IMU is currently detecting motion.
     */
    public boolean isMoving()
    {
        return (getAverageFromWorldLinearAccelHistory() >= 0.01);
    }

    /**
     * Returns the current temperature (in degrees centigrade) reported by
     * the nav6 gyro/accelerometer circuit.
     * 
     * This value may be useful in order to perform advanced temperature-
     * dependent calibration.
     * @return The current temperature (in degrees centigrade).
     */
    public float getTempC()
    {
        return this.temp_c;
    }
    
    //@Override
    protected void initIMU() {
        super.initIMU();
        world_linear_accel_history = new float[WORLD_LINEAR_ACCEL_HISTORY_LENGTH];
        initWorldLinearAccelHistory();
    }

    private void initWorldLinearAccelHistory(){
        Arrays.fill(world_linear_accel_history,0);
        next_world_linear_accel_history_index = 0;
        world_linear_acceleration_recent_avg = (float) 0.0;
    }
    
    private void updateWorldLinearAccelHistory( float x, float y, float z ){
        if (next_world_linear_accel_history_index >= WORLD_LINEAR_ACCEL_HISTORY_LENGTH) {
            next_world_linear_accel_history_index = 0;
        }
        world_linear_accel_history[next_world_linear_accel_history_index] = Math.abs(x) + Math.abs(y);
        next_world_linear_accel_history_index++;
    }
    
    public float getAverageFromWorldLinearAccelHistory(){
        float world_linear_accel_history_sum = (float) 0.0;
        for (int i = 0; i < WORLD_LINEAR_ACCEL_HISTORY_LENGTH; i++) {
            world_linear_accel_history_sum += world_linear_accel_history[i];
        }
        return world_linear_accel_history_sum / WORLD_LINEAR_ACCEL_HISTORY_LENGTH;
    }

    private void setQuaternion(IMUProtocol.QuaternionUpdate raw_update) {
        synchronized (this) { // synchronized block
            
            float[] q = new float[4];
            float[] gravity = new float[3];
            //float[] euler = new float[3];
            float[] ypr = new float[3];
            float yaw_degrees;
            float pitch_degrees;
            float roll_degrees;
            float linear_acceleration_x;
            float linear_acceleration_y;
            float linear_acceleration_z;
            float q2[] = new float[4];
            float q_product[] = new float[4];
            float world_linear_acceleration_x;
            float world_linear_acceleration_y;
            float world_linear_acceleration_z;
                       
            q[0] = ((float)raw_update.q1) / 16384.0f;
            q[1] = ((float)raw_update.q2) / 16384.0f;
            q[2] = ((float)raw_update.q3) / 16384.0f;
            q[3] = ((float)raw_update.q4) / 16384.0f;
            for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i]; // Range-check quaterions
            
            // below calculations are necessary for calculation of yaw/pitch/roll, 
            // and tilt-compensated compass heading
            
            // calculate gravity vector
            gravity[0] = 2 * (q[1]*q[3] - q[0]*q[2]);
            gravity[1] = 2 * (q[0]*q[1] + q[2]*q[3]);
            gravity[2] = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
  
            // calculate Euler angles
            // This code is here for reference, and is commented out for performance reasons
           
            //euler[0] = (float) MathUtils.atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
            //euler[1] = (float) -MathUtils.asin(2*q[1]*q[3] + 2*q[0]*q[2]);
            //euler[2] = (float) MathUtils.atan2(2*q[2]*q[3] - 2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3] - 1);
  
            // calculate yaw/pitch/roll angles
            ypr[0] = (float) Math.atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
            ypr[1] = (float) Math.atan(gravity[0] / Math.sqrt(gravity[1]*gravity[1] + gravity[2]*gravity[2]));
            ypr[2] = (float) Math.atan(gravity[1] / Math.sqrt(gravity[0]*gravity[0] + gravity[2]*gravity[2]));
             
            yaw_degrees = (float) (ypr[0] * (180.0/Math.PI)); 
            pitch_degrees = (float) (ypr[1] * (180.0/Math.PI)); 
            roll_degrees = (float) (ypr[2] * (180.0/Math.PI)); 
             
            // Subtract nav6 offset, and handle potential 360 degree wrap-around
            yaw_degrees -= nav6_yaw_offset_degrees;
            if ( yaw_degrees < -180 ) yaw_degrees += 360;
            if ( yaw_degrees > 180 ) yaw_degrees -= 360;
             
            // calculate linear acceleration by 
            // removing the gravity component (+1g = +4096 in standard DMP FIFO packet)
             
            linear_acceleration_x = (float) ((((float)raw_update.accel_x) / (32768.0 / accel_fsr_g)) - gravity[0]);
            linear_acceleration_y = (float) ((((float)raw_update.accel_y) / (32768.0 / accel_fsr_g)) - gravity[1]);
            linear_acceleration_z = (float) ((((float)raw_update.accel_z) / (32768.0 / accel_fsr_g)) - gravity[2]); 
            
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

            float[] q_conjugate = new float[4];
            
            q_conjugate[0] = q[0];            
            q_conjugate[1] = -q[1];            
            q_conjugate[2] = -q[2];            
            q_conjugate[3] = -q[3];            

            float[] q_final = new float[4];
            
            q_final[0] = q_product[0]*q_conjugate[0] - q_product[1]*q_conjugate[1] - q_product[2]*q_conjugate[2] - q_product[3]*q_conjugate[3];  // new w
            q_final[1] = q_product[0]*q_conjugate[1] + q_product[1]*q_conjugate[0] + q_product[2]*q_conjugate[3] - q_product[3]*q_conjugate[2];  // new x
            q_final[2] = q_product[0]*q_conjugate[2] - q_product[1]*q_conjugate[3] + q_product[2]*q_conjugate[0] + q_product[3]*q_conjugate[1];  // new y 
            q_final[3] = q_product[0]*q_conjugate[3] + q_product[1]*q_conjugate[2] - q_product[2]*q_conjugate[1] + q_product[3]*q_conjugate[0];  // new z

            world_linear_acceleration_x = q_final[1];
            world_linear_acceleration_y = q_final[2];
            world_linear_acceleration_z = q_final[3];
             
            updateWorldLinearAccelHistory(world_linear_acceleration_x,world_linear_acceleration_y, world_linear_acceleration_z);
             
            // Calculate tilt-compensated compass heading
            
            float inverted_pitch = -ypr[1];
            float roll_radians = ypr[2];
            
            float cos_roll = (float) Math.cos(roll_radians);
            float sin_roll = (float) Math.sin(roll_radians);
            float cos_pitch = (float) Math.cos(inverted_pitch);
            float sin_pitch = (float) Math.sin(inverted_pitch);
            
            float MAG_X = raw_update.mag_x * cos_pitch + raw_update.mag_z * sin_pitch;
            float MAG_Y = raw_update.mag_x * sin_roll * sin_pitch + raw_update.mag_y * cos_roll - raw_update.mag_z * sin_roll * cos_pitch;
            float tilt_compensated_heading_radians = (float) Math.atan2(MAG_Y,MAG_X);
            float tilt_compensated_heading_degrees = (float) (tilt_compensated_heading_radians * (180.0 / Math.PI));
            
            // Adjust compass for board orientation,
            // and modify range from -180-180 to
            // 0-360 degrees
          
            tilt_compensated_heading_degrees -= 90.0;
            if ( tilt_compensated_heading_degrees < 0 ) {
              tilt_compensated_heading_degrees += 360; 
            }
            
            this.yaw = yaw_degrees;
            this.pitch = pitch_degrees;
            this.roll = roll_degrees;
            this.compass_heading = tilt_compensated_heading_degrees;
            
            this.world_linear_accel_x = world_linear_acceleration_x;
            this.world_linear_accel_y = world_linear_acceleration_y;
            this.world_linear_accel_z = world_linear_acceleration_z;
            this.temp_c = raw_update.temp_c;
            updateYawHistory(this.yaw);            
        }
    }
}
