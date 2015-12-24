package org.usfirst.frc.team2465.robot;

import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.RobotDrive;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * This is a demo program showing the use of the navX MXP to implement
 * a motion detection feature, which can be used to detect when your
 * robot is still and when it is not.
 *
 * The basic principle used within the Motion Detection example 
 * is to subtract the acceleration due to gravity from the raw 
 * acceleration values. The result value is known as 'world linear 
 * acceleration', representing the actual amount of acceleration 
 * due to motion, and is calculated automatically by the navX MXP 
 * motion processor.  Whenever the sum of the world linear 
 * acceleration in both the X and Y axes exceeds a 'motion 
 * threshold', motion is occurring.  The navX MXP makes it easy, 
 * simply invoke the 'isMoving()' function to determine if motion
 * is currently underway.
 */

public class Robot extends SampleRobot {
  AHRS ahrs;
  RobotDrive myRobot;
  Joystick stick;

  // Channels for the wheels
  final static int frontLeftChannel	= 2;
  final static int rearLeftChannel	= 3;
  final static int frontRightChannel	= 1;
  final static int rearRightChannel	= 0;
  
  public Robot() {
      myRobot = new RobotDrive(frontLeftChannel, rearLeftChannel,
      		frontRightChannel, rearRightChannel);
      myRobot.setExpiration(0.1);
      stick = new Joystick(0);
      try {
          /* Communicate w/navX MXP via the MXP SPI Bus.                                     */
          /* Alternatively:  I2C.Port.kMXP, SerialPort.Port.kMXP or SerialPort.Port.kUSB     */
          /* See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface/ for details. */
          ahrs = new AHRS(SPI.Port.kMXP); 
      } catch (RuntimeException ex ) {
          DriverStation.reportError("Error instantiating navX MXP:  " + ex.getMessage(), true);
      }
  }

  /**
   * Drive left & right motors for 2 seconds then stop
   */
  public void autonomous() {
      myRobot.setSafetyEnabled(false);
      myRobot.drive(-0.5, 0.0);	// drive forwards half speed
      Timer.delay(2.0);		//    for 2 seconds
      myRobot.drive(0.0, 0.0);	// stop robot
  }

  /**
   * Runs the motors with arcade steering.
   */

  public void operatorControl() {
      myRobot.setSafetyEnabled(true);
      while (isOperatorControl() && isEnabled()) {

          boolean motionDetected = ahrs.isMoving();
          SmartDashboard.putBoolean("MotionDetected", motionDetected);
          
          try {
              myRobot.mecanumDrive_Cartesian(stick.getX(), stick.getY(), stick.getTwist(),0);
          } catch( RuntimeException ex ) {
              String err_string = "Drive system error:  " + ex.getMessage();
              DriverStation.reportError(err_string, true);
          }
          Timer.delay(0.005);		// wait for a motor update time
      }
  }

  /**
   * Runs during test mode
   */
  public void test() {
  }
}
