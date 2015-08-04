
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

public class Robot extends SampleRobot {
    AHRS ahrs;
    RobotDrive myRobot;
    Joystick stick;
    boolean autoBalanceXMode;
    boolean autoBalanceYMode;

    public Robot() {
        myRobot = new RobotDrive(0, 1);
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

    static final double kOffBalanceAngleThresholdDegrees = 10;
    static final double kOonBalanceAngleThresholdDegrees  = 5;

    
    public void operatorControl() {
        myRobot.setSafetyEnabled(true);
        while (isOperatorControl() && isEnabled()) {

            double xAxisRate = stick.getX();
            double yAxisRate = stick.getY();
            double pitchAngleDegrees = ahrs.getPitch();
            double rollAngleDegrees = ahrs.getRoll();
            if ( !autoBalanceXMode && ( pitchAngleDegrees >= kOffBalanceAngleThresholdDegrees ) ) {
                autoBalanceXMode = true;
            }
            else if ( autoBalanceXMode && ( pitchAngleDegrees <= (-kOonBalanceAngleThresholdDegrees))) {
                autoBalanceXMode = false;
            }
            if ( !autoBalanceYMode && ( pitchAngleDegrees >= kOffBalanceAngleThresholdDegrees ) ) {
                autoBalanceYMode = true;
            }
            else if ( autoBalanceYMode && ( pitchAngleDegrees <= (-kOonBalanceAngleThresholdDegrees))) {
                autoBalanceYMode = false;
            }
            
            // Control drive system automatically, 
            // driving in reverse direction of pitch/roll angle,
            // with a magnitude based upon the angle
            
            if ( autoBalanceXMode ) {
                double pitchAngleRadians = pitchAngleDegrees * (Math.PI / 180.0);
                xAxisRate = Math.sin(pitchAngleRadians) * -1;
            }
            if ( autoBalanceYMode ) {
                double rollAngleRadians = rollAngleDegrees * (Math.PI / 180.0);
                yAxisRate = Math.sin(rollAngleRadians) * -1;
            }
            
            try {
                myRobot.mecanumDrive_Cartesian(xAxisRate, yAxisRate, stick.getTwist(), ahrs.getAngle());
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
