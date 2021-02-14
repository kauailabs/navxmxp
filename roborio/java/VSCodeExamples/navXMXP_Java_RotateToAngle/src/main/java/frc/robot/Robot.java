/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package frc.robot;

import edu.wpi.first.wpilibj.TimedRobot;
import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.controller.PIDController;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.Spark;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.drive.MecanumDrive;
import edu.wpi.first.wpiutil.math.MathUtil;

/**
 * This is a demo program showing the use of the navX MXP to implement a "rotate
 * to angle" feature.
 *
 * This example will automatically rotate the robot to one of four angles (0,
 * 90, 180 and 270 degrees).
 *
 * This rotation can occur when the robot is still, but can also occur when the
 * robot is driving. When using field-oriented control, this will cause the
 * robot to drive in a straight line, in whathever direction is selected.
 *
 * This example also includes a feature allowing the driver to "reset" the "yaw"
 * angle. When the reset occurs, the new gyro angle will be 0 degrees. This can
 * be useful in cases when the gyro drifts, which doesn't typically happen
 * during a FRC match, but can occur during long practice sessions.
 *
 * Note that the PID Controller coefficients defined below will need to be tuned
 * for your drive system.
 */

public class Robot extends TimedRobot {
  AHRS ahrs;
  MecanumDrive myRobot;
  Joystick stick;
  PIDController turnController;
  double rotateToAngleRate;

  /* The following PID Controller coefficients will need to be tuned */
  /* to match the dynamics of your drive system. Note that the */
  /* SmartDashboard in Test mode has support for helping you tune */
  /* controllers by displaying a form where you can enter new P, I, */
  /* and D constants and test the mechanism. */

  static final double kP = 0.03;
  static final double kI = 0.00;
  static final double kD = 0.00;
  static final double kF = 0.00;

  static final double kToleranceDegrees = 2.0f;

  // Channels for the wheels
  final static int frontLeftChannel = 2;
  final static int rearLeftChannel = 3;
  final static int frontRightChannel = 1;
  final static int rearRightChannel = 0;

  Spark frontLeft;
  Spark rearLeft;
  Spark frontRight;
  Spark rearRight;

  /**
   * This function is run when the robot is first started up and should be used
   * for any initialization code.
   */
  @Override
  public void robotInit() {
    frontLeft = new Spark(frontLeftChannel);
    rearLeft = new Spark(rearLeftChannel);
    frontRight = new Spark(frontRightChannel);
    rearRight = new Spark(rearRightChannel);
    myRobot = new MecanumDrive(frontLeft, rearLeft, frontRight, rearRight);
    myRobot.setExpiration(0.1);
    stick = new Joystick(0);
    try {
      /***********************************************************************
       * navX-MXP: - Communication via RoboRIO MXP (SPI, I2C) and USB. - See
       * http://navx-mxp.kauailabs.com/guidance/selecting-an-interface.
       * 
       * navX-Micro: - Communication via I2C (RoboRIO MXP or Onboard) and USB. - See
       * http://navx-micro.kauailabs.com/guidance/selecting-an-interface.
       * 
       * VMX-pi: - Communication via USB. - See
       * https://vmx-pi.kauailabs.com/installation/roborio-installation/
       * 
       * Multiple navX-model devices on a single robot are supported.
       ************************************************************************/
      ahrs = new AHRS(SPI.Port.kMXP);
    } catch (RuntimeException ex) {
      DriverStation.reportError("Error instantiating navX MXP:  " + ex.getMessage(), true);
    }
    turnController = new PIDController(kP, kI, kD);
    turnController.enableContinuousInput(-180.0f, 180.0f);

    /* Note that the PIDController GUI should be added automatically to */
    /* the Test-mode dashboard, allowing manual tuning of the Turn */
    /* Controller's P, I and D coefficients. */
    /* Typically, only the P value needs to be modified. */
    }

  /**
   * This function is called every robot packet, no matter the mode. Use this for
   * items like diagnostics that you want ran during disabled, autonomous,
   * teleoperated and test.
   *
   * <p>
   * This runs after the mode specific periodic functions, but before LiveWindow
   * and SmartDashboard integrated updating.
   */
  @Override
  public void robotPeriodic() {
  }

  /**
   * This autonomous (along with the chooser code above) shows how to select
   * between different autonomous modes using the dashboard. The sendable chooser
   * code works with the Java SmartDashboard. If you prefer the LabVIEW Dashboard,
   * remove all of the chooser code and uncomment the getString line to get the
   * auto name from the text box below the Gyro
   *
   * <p>
   * You can add additional auto modes by adding additional comparisons to the
   * switch structure below with additional strings. If using the SendableChooser
   * make sure to add them to the chooser code above as well.
   */
  @Override
  public void autonomousInit() {
  }

  /**
   * This function is called periodically during autonomous.
   */
  @Override
  public void autonomousPeriodic() {
  }

  /**
   * This function is called periodically during operator control.
   */
  @Override
  public void teleopPeriodic() {
    boolean rotateToAngle = false;
    if (stick.getRawButton(1)) {
      ahrs.reset();
    }
    if (stick.getRawButton(2)) {
      turnController.setSetpoint(0.0f);
      rotateToAngle = true;
    } else if (stick.getRawButton(3)) {
      turnController.setSetpoint(90.0f);
      rotateToAngle = true;
    } else if (stick.getRawButton(4)) {
      turnController.setSetpoint(179.9f);
      rotateToAngle = true;
    } else if (stick.getRawButton(5)) {
      turnController.setSetpoint(-90.0f);
      rotateToAngle = true;
    }
    double currentRotationRate;
    if (rotateToAngle) {
      currentRotationRate = MathUtil.clamp(turnController.calculate(ahrs.getAngle()), -1.0, 1.0);
    } else {
      currentRotationRate = stick.getTwist();
    }
    try {
      /* Use the joystick X axis for lateral movement, */
      /* Y axis for forward movement, and the current */
      /* calculated rotation rate (or joystick Z axis), */
      /* depending upon whether "rotate to angle" is active. */
      myRobot.driveCartesian(stick.getX(), stick.getY(), currentRotationRate, ahrs.getAngle());
    } catch (RuntimeException ex) {
      DriverStation.reportError("Error communicating with drive system:  " + ex.getMessage(), true);
    }
  }

  /**
   * This function is called periodically during test mode.
   */
  @Override
  public void testPeriodic() {
  }
}
