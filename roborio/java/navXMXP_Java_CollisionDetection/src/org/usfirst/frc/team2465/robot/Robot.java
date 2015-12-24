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
 * a collision detection feature, which can be used to detect events
 * while driving a robot, such as bumping into a wall or being hit
 * by another robot.
 *
 * The basic principle used within the Collision Detection example 
 * is the calculation of Jerk (which is defined as the change in 
 * acceleration).  In the sample code shown below, both the X axis and 
 * the Y axis jerk are calculated, and if either exceeds a threshold, 
 * then a collision has occurred.
 * 
 * The 'collision threshold' used in these samples will likely need to 
 * be tuned for your robot, since the amount of jerk which constitutes 
 * a collision will be dependent upon the robot mass and expected 
 * maximum velocity of either the robot, or any object which may strike 
 * the robot.
 */

public class Robot extends SampleRobot {
  AHRS ahrs;
  RobotDrive myRobot;
  Joystick stick;
  double last_world_linear_accel_x;
  double last_world_linear_accel_y;

  final static double kCollisionThreshold_DeltaG = 0.5f;

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

          boolean collisionDetected = false;
          
          double curr_world_linear_accel_x = ahrs.getWorldLinearAccelX();
          double currentJerkX = curr_world_linear_accel_x - last_world_linear_accel_x;
          last_world_linear_accel_x = curr_world_linear_accel_x;
          double curr_world_linear_accel_y = ahrs.getWorldLinearAccelY();
          double currentJerkY = curr_world_linear_accel_y - last_world_linear_accel_y;
          last_world_linear_accel_y = curr_world_linear_accel_y;
          
          if ( ( Math.abs(currentJerkX) > kCollisionThreshold_DeltaG ) ||
               ( Math.abs(currentJerkY) > kCollisionThreshold_DeltaG) ) {
              collisionDetected = true;
          }
          SmartDashboard.putBoolean(  "CollisionDetected", collisionDetected);
          
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
