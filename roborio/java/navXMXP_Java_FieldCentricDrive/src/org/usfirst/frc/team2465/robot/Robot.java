package org.usfirst.frc.team2465.robot;

import com.kauailabs.navx.frc.AHRS;

import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.RobotDrive;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.Timer;

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
        myRobot.drive(-0.5, 0.0);	 // drive forwards half speed
        Timer.delay(2.0);		     //  for 2 seconds
        myRobot.drive(0.0, 0.0);	 // stop robot
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
                myRobot.mecanumDrive_Cartesian(stick.getX(), stick.getY(), 
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
