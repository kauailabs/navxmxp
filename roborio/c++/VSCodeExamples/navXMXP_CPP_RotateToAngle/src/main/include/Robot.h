/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <string>

#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SendableChooser.h>
#include <frc/drive/MecanumDrive.h>
#include <frc/Joystick.h.>
#include <frc/Spark.h>
#include <frc/DriverStation.h>
#include <frc/PIDOutput.h>
#include <frc/PIDController.h>
#include "AHRS.h"

using namespace frc;

class Robot : public TimedRobot, public PIDOutput {
 public:
  void RobotInit() override;
  void RobotPeriodic() override;
  void AutonomousInit() override;
  void AutonomousPeriodic() override;
  void TeleopInit() override;
  void TeleopPeriodic() override;
  void TestPeriodic() override;

  void PIDWrite(double output) override;

 private:
    // Channels for the wheels
    const static int frontLeftChannel = 2;
    const static int rearLeftChannel = 3;
    const static int frontRightChannel = 1;
    const static int rearRightChannel = 0;

    Spark *frontLeft;
    Spark *rearLeft;
    Spark *frontRight;
    Spark *rearRight;

    const static int joystickChannel = 0;

    MecanumDrive *robotDrive; // robot drive system
    Joystick *stick;          // only joystick
    AHRS *ahrs;
    PIDController *turnController;      // PID Controller
    double rotateToAngleRate;           // Current rotation rate
};
