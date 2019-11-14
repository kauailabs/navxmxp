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
#include <frc/Joystick.h>
#include <frc/Victor.h>
#include <frc/Jaguar.h>
#include <frc/DigitalInput.h>
#include <frc/DigitalOutput.h>
#include <frc/Encoder.h>
#include <frc/AnalogInput.h>
#include <frc/AnalogOutput.h>
#include <frc/AnalogTrigger.h>
#include <frc/Counter.h>
#include <frc/Joystick.h>
#include <frc/DriverStation.h>

using namespace frc;

class Robot : public frc::TimedRobot {
 public:
  void RobotInit() override;
  void RobotPeriodic() override;
  void AutonomousInit() override;
  void AutonomousPeriodic() override;
  void TeleopInit() override;
  void TeleopPeriodic() override;
  void TestPeriodic() override;

  enum PinType { DigitalIO, PWM, AnalogIn, AnalogOut };
  int GetChannelFromPin( Robot::PinType type, int io_pin_number );

 private:
    const static int joystickChannel	= 0;

    Joystick *stick;

    /* Digital IO */
    Victor *pwm_out_9;          /* E.g., PWM out to motor controller  */
    Jaguar *pwm_out_8;          /* E.g., PWM out to motor controller  */
    DigitalInput *dig_in_7;     /* E.g., input from contact switch    */
    DigitalInput *dig_in_6;     /* E.g., input from contact switch    */
    DigitalOutput *dig_out_5;   /* E.g., output to relay or LED       */
    DigitalOutput *dig_out_4;   /* E.g., output to relay or LED       */
    Encoder *enc_3and2;         /* E.g., Wheel Encoder                */
    Encoder *enc_1and0;         /* E.g., Wheel Encoder                */

    /* Analog Inputs */
    AnalogInput *an_in_1;       /* E.g., Ultrasonic Sensor            */
    AnalogTrigger *an_trig_0;   /* E.g., Proximity Sensor Threshold   */
    Counter *an_trig_0_counter; /* E.g., Count of an_trig_0 events    */

    /* Analog Outputs */
    AnalogOutput *an_out_1;     /* E.g., Constant-current LED output  */
    AnalogOutput *an_out_0;     /* E.g., Speaker output               */
};
