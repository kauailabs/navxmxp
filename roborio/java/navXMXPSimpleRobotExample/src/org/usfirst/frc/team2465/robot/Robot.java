/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/
package org.usfirst.frc.team2465.robot;

import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.SerialPort;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.livewindow.LiveWindow;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * The VM is configured to automatically run this class, and to call the
 * functions corresponding to each mode, as described in the SimpleRobot
 * documentation. If you change the name of this class or the package after
 * creating this project, you must also update the manifest file in the resource
 * directory.
 */
public class Robot extends SampleRobot {

    AHRS imu;			// This class can only be used w/the navX MXP.
    boolean first_iteration;

    public Robot() {

        try {

            // Use SerialPort.Port.kMXP if connecting navX MXP to the RoboRio MXP port
            // Use SerialPort.Port.kUSB if connecting navX MXP to the RoboRio USB port

            // You can add a second parameter to modify the 
            // update rate (in hz).  The minimum is 4.      
            // The maximum (and the default) is 60 on a navX MXP.
            // If you need to minimize CPU load, you can set it to a
            // lower value, as shown here, depending upon your needs.
            // The recommended update rate is 50Hz

            // The AHRS class provides access to advanced features on 
            // a navX MXP, including 9-axis headings
            // and magnetic disturbance detection.  This class also offers
            // access to altitude/barometric pressure data from a
            // navX MXP Aero.

            byte update_rate_hz = 50;
            imu = new AHRS(SerialPort.Port.kMXP, update_rate_hz);
        } catch( Exception ex ) {
            ex.printStackTrace();
        }
        if ( imu != null ) {
            LiveWindow.addSensor("IMU", "Gyro", imu);
        }
        first_iteration = true;
    }

    /**
     * This function is called once each time the robot enters autonomous mode.
     */
    public void autonomous() {

    }

    /**
     * This function is called once each time the robot enters operator control.
     */
    public void operatorControl() {

        AHRS.BoardYawAxis yaw_axis = new AHRS.BoardYawAxis();
        while (isOperatorControl() && isEnabled()) {

            // When calibration has completed, zero the yaw
            // Calibration is complete approximately 20 seconds
            // after the robot is powered on.  During calibration,
            // the robot should be still.

            boolean is_calibrating = imu.isCalibrating();
            if ( first_iteration && !is_calibrating ) {
                Timer.delay( 0.3 );
                imu.zeroYaw();
                first_iteration = false;
            }

            // Update the dashboard with status and orientation
            // data from the navX MXP

            imu.getBoardYawAxis(yaw_axis);  

            SmartDashboard.putBoolean("Yaw_Axis_Up",        yaw_axis.up);
            SmartDashboard.putNumber( "Yaw_Axis",           yaw_axis.board_axis);            
            
            boolean reset_yaw_button_pressed = DriverStation.getInstance().getStickButton(0,(byte)1);
            if ( reset_yaw_button_pressed ) {
                imu.zeroYaw();
            }
            
            SmartDashboard.putBoolean(  "IMU_Connected",        imu.isConnected());
            SmartDashboard.putBoolean(  "IMU_IsCalibrating",    imu.isCalibrating());
            SmartDashboard.putNumber(   "IMU_Yaw",              imu.getYaw());
            SmartDashboard.putNumber(   "IMU_Pitch",            imu.getPitch());
            SmartDashboard.putNumber(   "IMU_Roll",             imu.getRoll());
            SmartDashboard.putNumber(   "IMU_CompassHeading",   imu.getCompassHeading());
            SmartDashboard.putNumber(   "IMU_Update_Count",     imu.getUpdateCount());
            SmartDashboard.putNumber(   "IMU_Byte_Count",       imu.getByteCount());

            /* These functions are compatible w/the WPI Gyro Class */
            SmartDashboard.putNumber(   "IMU_TotalYaw", 		imu.getAngle());
            SmartDashboard.putNumber(   "IMU_YawRateDPS",		imu.getRate());

            SmartDashboard.putNumber(   "IMU_Accel_X",          imu.getWorldLinearAccelX());
            SmartDashboard.putNumber(   "IMU_Accel_Y",          imu.getWorldLinearAccelY());
            SmartDashboard.putBoolean(  "IMU_IsMoving",         imu.isMoving());
            SmartDashboard.putNumber(   "IMU_Temp_C",           imu.getTempC());

            Timer.delay(0.1);
        }
    }

    /**
     * This function is called once each time the robot enters test mode.
     */
    public void test() {

    }
}
