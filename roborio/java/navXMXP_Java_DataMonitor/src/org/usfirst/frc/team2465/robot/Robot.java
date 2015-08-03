
package org.usfirst.frc.team2465.robot;


import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.I2C;
import edu.wpi.first.wpilibj.SerialPort;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * This is a demo program showing the use of the navX MXP Robotics Navigation Sensor.
 * 
 * The Robot constructor instantiates the "AHRS" class (Attitude/Heading Reference System).
 * 
 * Several constructors exist, one for each of the communication types (SPI, I2C and Serial).
 * Additionally, the sensor update rate can be customized if the default (50Hz) is not
 * desired.
 * 
 * Once instantiated, the AHRS class will periodically retrieve sensor data from the navX
 * sensor.
 * 
 * In the operatorControl() method, all data from the navX sensor is retrieved and output
 * to the SmartDashboard.
 * 
 * Additionally, the joystick trigger button is used to initiate "zeroing" (resetting)
 * the "Yaw" (Z axis) angle.  This technique is often used in Field-Centric (Field-Oriented)
 * drive systems to allow the driver to deal with drift, which may accumulate over long
 * periods of time (e.g., in a practice match).
 * 
 * In addition to the "processed data" (yaw, pitch, roll, compass heading, fused heading),
 * this sample demonstrates access to advanced features including Quaternion data,
 * Velocity/Displacement Estimates, and Raw Sensor Data.
 * 
 * As well, Board Information is also retrieved; this can be useful for debugging connectivity
 * issues after initial installation of the navX sensor.
 */
public class Robot extends SampleRobot {
    AHRS ahrs;
    Joystick stick;
 
    public Robot() {
        stick = new Joystick(0);
        byte update_rate_hz = 60;
        //ahrs = new AHRS(SPI.Port.kMXP,update_rate_hz);
        //ahrs = new AHRS(I2C.Port.kMXP,update_rate_hz);
        //ahrs = new AHRS(SerialPort.Port.kMXP,AHRS.SerialDataType.kProcessedData, update_rate_hz);
        //ahrs = new AHRS(SerialPort.Port.kMXP,AHRS.SerialDataType.kRawData, update_rate_hz);
        ahrs = new AHRS(SerialPort.Port.kUSB,AHRS.SerialDataType.kProcessedData,update_rate_hz);
    }

    /**
     * Runs during autonomous mode
     */
    public void autonomous() {
        Timer.delay(2.0);		//    for 2 seconds
    }

    /**
     * Display navX MXP Sensor Data on Smart Dashboard
     */
    public void operatorControl() {
        while (isOperatorControl() && isEnabled()) {
            
            Timer.delay(0.020);		/* wait for one motor update time period (50Hz)     */
            
            boolean zero_yaw_pressed = stick.getTrigger();
            if ( zero_yaw_pressed ) {
                ahrs.zeroYaw();
            }

            /* Display 6-axis Processed Angle Data                                      */
            SmartDashboard.putBoolean(  "IMU_Connected",        ahrs.isConnected());
            SmartDashboard.putBoolean(  "IMU_IsCalibrating",    ahrs.isCalibrating());
            SmartDashboard.putNumber(   "IMU_Yaw",              ahrs.getYaw());
            SmartDashboard.putNumber(   "IMU_Pitch",            ahrs.getPitch());
            SmartDashboard.putNumber(   "IMU_Roll",             ahrs.getRoll());
            
            /* Display tilt-corrected, Magnetometer-based heading (requires             */
            /* magnetometer calibration to be useful)                                   */
            
            SmartDashboard.putNumber(   "IMU_CompassHeading",   ahrs.getCompassHeading());
            
            /* Display 9-axis Heading (requires magnetometer calibration to be useful)  */
            SmartDashboard.putNumber(   "IMU_FusedHeading",     ahrs.getFusedHeading());

            /* These functions are compatible w/the WPI Gyro Class, providing a simple  */
            /* path for upgrading from the Kit-of-Parts gyro to the navx MXP            */
            
            SmartDashboard.putNumber(   "IMU_TotalYaw",         ahrs.getAngle());
            SmartDashboard.putNumber(   "IMU_YawRateDPS",       ahrs.getRate());

            /* Display Processed Acceleration Data (Linear Acceleration, Motion Detect) */
            
            SmartDashboard.putNumber(   "IMU_Accel_X",          ahrs.getWorldLinearAccelX());
            SmartDashboard.putNumber(   "IMU_Accel_Y",          ahrs.getWorldLinearAccelY());
            SmartDashboard.putBoolean(  "IMU_IsMoving",         ahrs.isMoving());
            SmartDashboard.putBoolean(  "IMU_IsRotating",       ahrs.isRotating());

            /* Display estimates of velocity/displacement.  Note that these values are  */
            /* not expected to be accurate enough for estimating robot position on a    */
            /* FIRST FRC Robotics Field, due to accelerometer noise and the compounding */
            /* of these errors due to single (velocity) integration and especially      */
            /* double (displacement) integration.                                       */
            
            SmartDashboard.putNumber(   "Velocity_X",           ahrs.getVelocityX());
            SmartDashboard.putNumber(   "Velocity_Y",           ahrs.getVelocityY());
            SmartDashboard.putNumber(   "Displacement_X",       ahrs.getDisplacementX());
            SmartDashboard.putNumber(   "Displacement_Y",       ahrs.getDisplacementY());
            
            /* Display Raw Gyro/Accelerometer/Magnetometer Values                       */
            /* NOTE:  These values are not normally necessary, but are made available   */
            /* for advanced users.  Before using this data, please consider whether     */
            /* the processed data (see above) will suit your needs.                     */
            
            SmartDashboard.putNumber(   "RawGyro_X",            ahrs.getRawGyroX());
            SmartDashboard.putNumber(   "RawGyro_Y",            ahrs.getRawGyroY());
            SmartDashboard.putNumber(   "RawGyro_Z",            ahrs.getRawGyroZ());
            SmartDashboard.putNumber(   "RawAccel_X",           ahrs.getRawAccelX());
            SmartDashboard.putNumber(   "RawAccel_Y",           ahrs.getRawAccelY());
            SmartDashboard.putNumber(   "RawAccel_Z",           ahrs.getRawAccelZ());
            SmartDashboard.putNumber(   "RawMag_X",             ahrs.getRawMagX());
            SmartDashboard.putNumber(   "RawMag_Y",             ahrs.getRawMagY());
            SmartDashboard.putNumber(   "RawMag_Z",             ahrs.getRawMagZ());
            SmartDashboard.putNumber(   "IMU_Temp_C",           ahrs.getTempC());
            
            /* Omnimount Yaw Axis Information                                           */
            /* For more info, see http://navx-mxp.kauailabs.com/installation/omnimount  */
            AHRS.BoardYawAxis yaw_axis = ahrs.getBoardYawAxis();
            SmartDashboard.putString(   "YawAxisDirection",     yaw_axis.up ? "Up" : "Down" );
            SmartDashboard.putNumber(   "YawAxis",              yaw_axis.board_axis.getValue() );
            
            /* Sensor Board Information                                                 */
            SmartDashboard.putString(   "FirmwareVersion",      ahrs.getFirmwareVersion());
            
            /* Quaternion Data                                                          */
            /* Quaternions are fascinating, and are the most compact representation of  */
            /* orientation data.  All of the Yaw, Pitch and Roll Values can be derived  */
            /* from the Quaternions.  If interested in motion processing, knowledge of  */
            /* Quaternions is highly recommended.                                       */
            SmartDashboard.putNumber(   "QuaternionW",          ahrs.getQuaternionW());
            SmartDashboard.putNumber(   "QuaternionX",          ahrs.getQuaternionX());
            SmartDashboard.putNumber(   "QuaternionY",          ahrs.getQuaternionY());
            SmartDashboard.putNumber(   "QuaternionZ",          ahrs.getQuaternionZ());
            
            /* Connectivity Debugging Support                                           */
            SmartDashboard.putNumber(   "IMU_Byte_Count",       ahrs.getByteCount());
            SmartDashboard.putNumber(   "IMU_Update_Count",     ahrs.getUpdateCount());
        
            /* Derived Motion Vector                                                    */
/*            double motion_vector_direction_degrees = 0.0f;
            double motion_vector_magnitude_g = 0.0f;
            if ( ahrs.isMoving() ) {
                double motion_vector_direction_radians = Math.atan2(ahrs.getVelocityX(), ahrs.getVelocityY());
                motion_vector_direction_degrees = motion_vector_direction_radians * (180.0 / Math.PI);
                motion_vector_magnitude_g = ((ahrs.getVelocityX() + ahrs.getVelocityY()) / 2);
            }
            SmartDashboard.putNumber(  "Motion_Direction", motion_vector_direction_degrees);
            SmartDashboard.putNumber(  "Motion Magnitude", motion_vector_magnitude_g );
*/
            }
    }

    /**
     * Runs during test mode
     */
    public void test() {
    }
}
