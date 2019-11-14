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
 * an automatic balance feature, which can be used to help avoid
 * a robot tipping over when driving..
 *
 * The basic principle shown in the example is measurement of the
 * Pitch (rotation about the X axis) and Roll (rotation about the
 * Y axis) angles.  When these angles exceed the "off balance"
 * threshold and until these angles fall below the "on balance"
 * threshold, the drive system is automatically driven in the
 * opposite direction at a magnitude proportional to the
 * Pitch or Roll angle.
 *
 * Note that this is just a starting point for automatic balancing,
 * and will likely require a reasonable amount of tuning in order
 * to work well with your robot.
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
  double xAxisRate = stick->GetX();
  double yAxisRate = stick->GetY();
  double pitchAngleDegrees = ahrs->GetPitch();
  double rollAngleDegrees = ahrs->GetRoll();

  if (!autoBalanceXMode &&
      (fabs(pitchAngleDegrees) >=
       fabs(kOffBalanceThresholdDegrees)))
  {
    autoBalanceXMode = true;
  }
  else if (autoBalanceXMode &&
           (fabs(pitchAngleDegrees) <=
            fabs(kOnBalanceThresholdDegrees)))
  {
    autoBalanceXMode = false;
  }
  if (!autoBalanceYMode &&
      (fabs(pitchAngleDegrees) >=
       fabs(kOffBalanceThresholdDegrees)))
  {
    autoBalanceYMode = true;
  }
  else if (autoBalanceYMode &&
           (fabs(pitchAngleDegrees) <=
            fabs(kOnBalanceThresholdDegrees)))
  {
    autoBalanceYMode = false;
  }

  // Control drive system automatically,
  // driving in reverse direction of pitch/roll angle,
  // with a magnitude based upon the angle

  if (autoBalanceXMode)
  {
    double pitchAngleRadians = pitchAngleDegrees * (M_PI / 180.0);
    xAxisRate = sin(pitchAngleRadians) * -1;
  }
  if (autoBalanceYMode)
  {
    double rollAngleRadians = rollAngleDegrees * (M_PI / 180.0);
    yAxisRate = sin(rollAngleRadians) * -1;
  }

  try
  {
    // Use the joystick X axis for lateral movement, Y axis for forward movement, and Z axis for rotation.
    robotDrive->DriveCartesian(xAxisRate, yAxisRate, stick->GetZ());
  }
  catch (exception &ex)
  {
    string err_string = "Drive system error:  ";
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
