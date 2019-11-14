/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "Robot.h"

#include <iostream>
#include <string>
#include <math.h>

#include <frc/smartdashboard/SmartDashboard.h>

/**
 * This is a demo program showing the use of the navX MXP to implement
 * a motion detection feature, which can be used to detect when your
 * robot is still and when it is not.
 *
 * The basic principle used within the Motion Detection example
 * is to subtract the acceleration due to gravity from the raw
 * acceleration values. The result value is known as “world linear
 * acceleration”, representing the actual amount of acceleration
 * due to motion, and is calculated automatically by navX MXP’s
 * motion processor.  Whenever the sum of the world linear
 * acceleration in both the X and Y axes exceeds a “motion
 * threshold”, motion is occurring.  The navX MXP makes it easy,
 * simply invoke the "isMoving()" function to determine if motion
 * is currently underway.
 */

static const double kOffBalanceThresholdDegrees = 10.0f;
static const double kOnBalanceThresholdDegrees = 5.0f;

using namespace std;

void Robot::RobotInit()
{
  frontLeft = new Spark(frontLeftChannel);
  rearLeft = new Spark(rearLeftChannel);
  frontRight = new Spark(frontRightChannel);
  rearRight = new Spark(rearRightChannel);
  robotDrive = new MecanumDrive(*frontLeft, *rearLeft,
                                *frontRight, *rearRight);
  stick = new Joystick(joystickChannel);

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
  catch (exception &ex)
  {
    std::string what_string = ex.what();
    std::string err_msg("Error instantiating navX MXP:  " + what_string);
    const char *p_err_msg = err_msg.c_str();
    DriverStation::ReportError(p_err_msg);
  }
  autoBalanceXMode = false;
  autoBalanceYMode = false;
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

/**
 * Drive based upon joystick inputs, and automatically control
 * motors if the robot begins tipping.
 */

void Robot::TeleopPeriodic()
{
  bool motionDetected = ahrs->IsMoving();
  SmartDashboard::PutBoolean("MotionDetected", motionDetected);

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
    std::string err_string = "Error communicating with Drive System:  ";
    err_string += ex.what();
    DriverStation::ReportError(err_string.c_str());
  }
}

void Robot::TestPeriodic() {}

#ifndef RUNNING_FRC_TESTS
int main()
{
  return frc::StartRobot<Robot>();
}
#endif
