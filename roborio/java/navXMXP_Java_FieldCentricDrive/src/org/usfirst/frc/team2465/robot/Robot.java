package org.usfirst.frc.team2465.robot;

import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.Spark;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.drive.MecanumDrive;

/**
 * This is a demo program showing the use of the navX MXP to implement
 * field-centric ("field-oriented") drive system control of a Mecanum-
 * based drive system.  Note that field-centric drive can also be used
 * with other Holonomic drive systems (e.g., OmniWheel, Swerve).
 *
 * This example also includes a feature allowing the driver to "reset"
 * the "yaw" angle.  When the reset occurs, the new gyro angle will be
 * 0 degrees.  This can be useful in cases when the gyro drifts, which
 * doesn't typically happen during a FRC match, but can occur during
 * long practice sessions.
 */

public class Robot extends SampleRobot {
    AHRS ahrs;
    MecanumDrive myRobot;
    Joystick stick;

    // Channels for the wheels
    final static int frontLeftChannel	= 2;
    final static int rearLeftChannel	= 3;
    final static int frontRightChannel	= 1;
    final static int rearRightChannel	= 0;
    
    Spark frontLeft;
    Spark rearLeft;
    Spark frontRight;
    Spark rearRight;
            
    public Robot() {
    	frontLeft = new Spark(frontLeftChannel);
    	rearLeft = new Spark(rearLeftChannel);
    	frontRight = new Spark(frontRightChannel);
    	rearRight = new Spark(rearRightChannel);
        myRobot = new MecanumDrive(frontLeft, rearLeft, 
  				 frontRight, rearRight);
        myRobot.setExpiration(0.1);
        stick = new Joystick(0);
        try {
			/***********************************************************************
			 * navX-MXP:
			 * - Communication via RoboRIO MXP (SPI, I2C, TTL UART) and USB.            
			 * - See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface.
			 * 
			 * navX-Micro:
			 * - Communication via I2C (RoboRIO MXP or Onboard) and USB.
			 * - See http://navx-micro.kauailabs.com/guidance/selecting-an-interface.
			 * 
			 * Multiple navX-model devices on a single robot are supported.
			 ************************************************************************/
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
        myRobot.driveCartesian(0.0, -0.5, 0.0);	 // drive forwards half speed
        Timer.delay(2.0);		     			 //  for 2 seconds
        myRobot.driveCartesian(0.0, 0.0, 0.0);	 // stop robot
    }

    /**
     * Runs the motors with arcade steering.
     */
    public void operatorControl() {
        myRobot.setSafetyEnabled(true);
        while (isOperatorControl() && isEnabled()) {
            if ( stick.getRawButton(1)) {
                ahrs.reset();
            }
            try {
                /* Use the joystick X axis for lateral movement,            */
                /* Y axis for forward movement, and Z axis for rotation.    */
                /* Use navX MXP yaw angle to define Field-centric transform */
                myRobot.driveCartesian(stick.getX(), stick.getY(), 
                                       stick.getTwist(), ahrs.getAngle());
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
}
