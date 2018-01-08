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
    MecanumDrive myRobot;
    Joystick stick;
    boolean autoBalanceXMode;
    boolean autoBalanceYMode;

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
        myRobot.driveCartesian(0.0, -0.5, 0.0);	// drive forwards half speed
        Timer.delay(2.0);						//    for 2 seconds
        myRobot.driveCartesian(0.0, 0.0, 0.0);	// stop robot
    }

    /**
     * Runs the motors with arcade steering.
     */

    static final double kOffBalanceAngleThresholdDegrees = 10;
    static final double kOonBalanceAngleThresholdDegrees  = 5;

    
    public void operatorControl() {
        myRobot.setSafetyEnabled(true);
        while (isOperatorControl() && isEnabled()) {

            double xAxisRate            = stick.getX();
            double yAxisRate            = stick.getY();
            double pitchAngleDegrees    = ahrs.getPitch();
            double rollAngleDegrees     = ahrs.getRoll();
            
            if ( !autoBalanceXMode && 
                 (Math.abs(pitchAngleDegrees) >= 
                  Math.abs(kOffBalanceAngleThresholdDegrees))) {
                autoBalanceXMode = true;
            }
            else if ( autoBalanceXMode && 
                      (Math.abs(pitchAngleDegrees) <= 
                       Math.abs(kOonBalanceAngleThresholdDegrees))) {
                autoBalanceXMode = false;
            }
            if ( !autoBalanceYMode && 
                 (Math.abs(pitchAngleDegrees) >= 
                  Math.abs(kOffBalanceAngleThresholdDegrees))) {
                autoBalanceYMode = true;
            }
            else if ( autoBalanceYMode && 
                      (Math.abs(pitchAngleDegrees) <= 
                       Math.abs(kOonBalanceAngleThresholdDegrees))) {
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
                myRobot.driveCartesian(xAxisRate, yAxisRate, stick.getTwist(),0);
            } catch( RuntimeException ex ) {
                String err_string = "Drive system error:  " + ex.getMessage();
                DriverStation.reportError(err_string, true);
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