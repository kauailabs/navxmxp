
package org.usfirst.frc.team2465.robot;


import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.PIDController;
import edu.wpi.first.wpilibj.PIDOutput;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.RobotDrive;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.livewindow.LiveWindow;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * This is a demo program showing the use of the RobotDrive class.
 * The SampleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 *
 * The VM is configured to automatically run this class, and to call the
 * functions corresponding to each mode, as described in the SampleRobot
 * documentation. If you change the name of this class or the package after
 * creating this project, you must also update the manifest file in the resource
 * directory.
 *
 * WARNING: While it may look like a good choice to use for your code if you're inexperienced,
 * don't. Unless you know what you are doing, complex code will be much more difficult under
 * this system. Use IterativeRobot or Command-Based instead if you're new.
 */
public class Robot extends SampleRobot implements PIDOutput {
    AHRS ahrs;
    RobotDrive myRobot;
    Joystick stick;
    PIDController turnController;
    double rotateToAngleRate;
    
    /* The following PID Controller coefficients will need to be tuned */
    /* to match the dynamics of your drive system.  Note that the      */
    /* SmartDashboard in Test mode has support for helping you tune    */
    /* controllers by displaying a form where you can enter new P, I,  */
    /* and D constants and test the mechanism.                         */
    
    static final double kP = 0.03;
    static final double kI = 0.00;
    static final double kD = 0.00;
    static final double kF = 0.00;
    
    static final double kToleranceDegrees = 2.0f;
    
    public Robot() {
        myRobot = new RobotDrive(0, 1);
        myRobot.setExpiration(0.1);
        stick = new Joystick(0);
        try {
            ahrs = new AHRS(SPI.Port.kMXP);
        } catch (RuntimeException ex ) {
            DriverStation.reportError("Error instantiating navX MXP:  " + ex.getMessage(), true);
        }
        turnController = new PIDController(kP, kI, kD, kF, ahrs, this);
        turnController.setInputRange(-180.0f,  180.0f);
        turnController.setOutputRange(-1.0, 1.0);
        turnController.setAbsoluteTolerance(kToleranceDegrees);
        turnController.setContinuous(true);
        
        /* Add the PID Controller to the Test-mode dashboard, allowing manual  */
        /* tuning of the Turn Controller's P, I and D coefficients.            */
        /* Typically, only the P value needs to be modified.                   */
        LiveWindow.addActuator("DriveSystem", "RotateController", turnController);
    }

    /**
     * Drive left & right motors for 2 seconds then stop
     */
    public void autonomous() {
        myRobot.setSafetyEnabled(false);
        myRobot.drive(0.0, 0.0);    // stop robot
        Timer.delay(2.0);		//    for 2 seconds
        myRobot.drive(0.0, 0.0);	// stop robot
    }

    /**
     * Runs the motors with onnidirectional drive steering.
     * 
     * Implements Field-centric drive control.
     * 
     * Also implements "rotate to angle", where the angle
     * being rotated to is defined by one of four buttons.
     * 
     * Note that this "rotate to angle" approach can also 
     * be used while driving to implement "straight-line
     * driving".
     */
    public void operatorControl() {
        myRobot.setSafetyEnabled(true);
        while (isOperatorControl() && isEnabled()) {
            boolean rotateToAngle = false;
            if ( stick.getRawButton(1)) {
                ahrs.reset();
            }
            if ( stick.getRawButton(2)) {
                turnController.setSetpoint(0.0f);
                rotateToAngle = true;
            } else if ( stick.getRawButton(3)) {
                turnController.setSetpoint(90.0f);
                rotateToAngle = true;
            } else if ( stick.getRawButton(4)) {
                turnController.setSetpoint(179.9f);
                rotateToAngle = true;
            } else if ( stick.getRawButton(5)) {
                turnController.setSetpoint(-90.0f);
                rotateToAngle = true;
            }
            double currentRotationRate;
            if ( rotateToAngle ) {
                turnController.enable();
                currentRotationRate = rotateToAngleRate;
            } else {
                turnController.disable();
                currentRotationRate = stick.getTwist();
            }
            try {
                myRobot.mecanumDrive_Cartesian(stick.getX(), stick.getY(), currentRotationRate, ahrs.getAngle());
            } catch( RuntimeException ex ) {
                DriverStation.reportError("Error communicating with drive system:  " + ex.getMessage(), true);
            }
            Timer.delay(0.005);		// wait for a motor update time
        }
    }

    /**
     * Runs during test mode
     */
    public void test() {
    }

    @Override
    public void pidWrite(double output) {
        rotateToAngleRate = output;
    }
}
