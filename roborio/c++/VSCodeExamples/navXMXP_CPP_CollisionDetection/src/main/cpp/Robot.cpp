/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "Robot.h"

#include <iostream>

#include <frc/smartdashboard/SmartDashboard.h>

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

#define COLLISION_THRESHOLD_DELTA_G 0.5f

void Robot::RobotInit()
{
  frontLeft = new Spark(frontLeftChannel);
  rearLeft = new Spark(rearLeftChannel);
  frontRight = new Spark(frontRightChannel);
  rearRight = new Spark(rearRightChannel);
  robotDrive = new MecanumDrive(*frontLeft, *rearLeft,
                                *frontRight, *rearRight);
  stick = new Joystick(joystickChannel);

  last_world_linear_accel_x = 0.0f;
  last_world_linear_accel_y = 0.0f;

  robotDrive->SetExpiration(0.1);
  frontLeft->SetInverted(true); // invert the left side motors
  rearLeft->SetInverted(true);  // (remove/modify to match your robot)
  try
  {
    /***********************************************************************
     * navX-MXP:
     * - Communication via RoboRIO MXP (SPI, I2C) and USB.            
     * - See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface.
     * 
     * navX-Micro:
     * - Communication via I2C (RoboRIO MXP or Onboard) and USB.
     * - See http://navx-micro.kauailabs.com/guidance/selecting-an-interface.
     * 
     * VMX-pi:
     * - Communication via USB.
     * - See https://vmx-pi.kauailabs.com/installation/roborio-installation/
     * 
     * Multiple navX-model devices on a single robot are supported.
     ************************************************************************/
    ahrs = new AHRS(SPI::Port::kMXP);
  }
  catch (std::exception &ex)
  {
    std::string what_string = ex.what();
    std::string err_msg("Error instantiating navX MXP:  " + what_string);
    const char *p_err_msg = err_msg.c_str();
    DriverStation::ReportError(p_err_msg);
  }
}

/**
 * This function is called every robot packet, no matter the mode. Use
 * this for items like diagnostics that you want ran during disabled,
 * autonomous, teleoperated and test.
 *
 * <p> This runs after the mode specific periodic functions, but before
 * LiveWindow and SmartDashboard integrated updating.
 */
void Robot::RobotPeriodic() {}

/**
 * This autonomous (along with the chooser code above) shows how to select
 * between different autonomous modes using the dashboard. The sendable chooser
 * code works with the Java SmartDashboard. If you prefer the LabVIEW Dashboard,
 * remove all of the chooser code and uncomment the GetString line to get the
 * auto name from the text box below the Gyro.
 *
 * You can add additional auto modes by adding additional comparisons to the
 * if-else structure below with additional strings. If using the SendableChooser
 * make sure to add them to the chooser code above as well.
 */
void Robot::AutonomousInit()
{
}

void Robot::AutonomousPeriodic()
{
}

void Robot::TeleopInit() {}

void Robot::TeleopPeriodic()
{
  bool collisionDetected = false;

  double curr_world_linear_accel_x = ahrs->GetWorldLinearAccelX();
  double currentJerkX = curr_world_linear_accel_x - last_world_linear_accel_x;
  last_world_linear_accel_x = curr_world_linear_accel_x;
  double curr_world_linear_accel_y = ahrs->GetWorldLinearAccelY();
  double currentJerkY = curr_world_linear_accel_y - last_world_linear_accel_y;
  last_world_linear_accel_y = curr_world_linear_accel_y;

  if ((fabs(currentJerkX) > COLLISION_THRESHOLD_DELTA_G) ||
      (fabs(currentJerkY) > COLLISION_THRESHOLD_DELTA_G))
  {
    collisionDetected = true;
  }
  SmartDashboard::PutBoolean("CollisionDetected", collisionDetected);

  try
  {
    /* Use the joystick X axis for lateral movement,            */
    /* Y axis for forward movement, and Z axis for rotation.    */
    /* Use navX MXP yaw angle to define Field-centric transform */
    robotDrive->DriveCartesian(stick->GetX(), stick->GetY(),
                               stick->GetZ(), ahrs->GetAngle());
  }
  catch (std::exception &ex)
  {
    std::string what_string = ex.what();
    std::string err_msg("Error instantiating navX MXP:  " + what_string);
    const char *p_err_msg = err_msg.c_str();
    DriverStation::ReportError(p_err_msg);
  }
}

void Robot::TestPeriodic() {}

#ifndef RUNNING_FRC_TESTS
int main()
{
  return frc::StartRobot<Robot>();
}
#endif
