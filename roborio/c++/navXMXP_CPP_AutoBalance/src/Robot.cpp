#include <string>
#include "WPILib.h"
#include "AHRS.h"
#include <math.h>
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
static const double kOnBalanceThresholdDegrees  = 5.0f;

using namespace std;

class Robot: public SampleRobot
{

    // Channels for the wheels
    const static int frontLeftChannel	= 2;
    const static int rearLeftChannel	= 3;
    const static int frontRightChannel	= 1;
    const static int rearRightChannel	= 0;

    Spark frontLeft;
    Spark rearLeft;
    Spark frontRight;
    Spark rearRight;

    const static int joystickChannel	= 0;

    MecanumDrive robotDrive;	// robot drive system
    Joystick stick;			// only joystick
    AHRS *ahrs;
    bool autoBalanceXMode;
    bool autoBalanceYMode;

public:
    Robot() :
            frontLeft(frontLeftChannel),
			rearLeft(rearLeftChannel),
			frontRight(frontRightChannel),
			rearRight(rearRightChannel),
    		robotDrive(frontLeft,  rearLeft,
                       frontRight, rearRight),	// these must be initialized in the
            stick(joystickChannel)				// same order as they are declared above.
    {
        robotDrive.SetExpiration(0.1);
        frontLeft.SetInverted(true);  // invert the left side motors
        rearLeft.SetInverted(true);   // (remove/modify to match your robot)
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
            ahrs = new AHRS(SPI::Port::kMXP);
        } catch (exception& ex ) {
        	std::string what_string = ex.what();
        	std::string err_msg("Error instantiating navX MXP:  " + what_string);
        	const char *p_err_msg = err_msg.c_str();
            DriverStation::ReportError(p_err_msg);
        }
        autoBalanceXMode = false;
        autoBalanceYMode = false;
    }

    /**
     * Drive based upon joystick inputs, and automatically control
     * motors if the robot begins tipping.
     */
    void OperatorControl()
    {
        robotDrive.SetSafetyEnabled(false);
        while (IsOperatorControl() && IsEnabled()) {

            double xAxisRate = stick.GetX();
            double yAxisRate = stick.GetY();
            double pitchAngleDegrees = ahrs->GetPitch();
            double rollAngleDegrees = ahrs->GetRoll();

            if ( !autoBalanceXMode &&
                 (fabs(pitchAngleDegrees) >=
                  fabs(kOffBalanceThresholdDegrees))) {
                autoBalanceXMode = true;
            }
            else if ( autoBalanceXMode &&
                      (fabs(pitchAngleDegrees) <=
                       fabs(kOnBalanceThresholdDegrees))) {
                autoBalanceXMode = false;
            }
            if ( !autoBalanceYMode &&
                 (fabs(pitchAngleDegrees) >=
                  fabs(kOffBalanceThresholdDegrees))) {
                autoBalanceYMode = true;
            }
            else if ( autoBalanceYMode &&
                      (fabs(pitchAngleDegrees) <=
                       fabs(kOnBalanceThresholdDegrees))) {
                autoBalanceYMode = false;
            }

            // Control drive system automatically,
            // driving in reverse direction of pitch/roll angle,
            // with a magnitude based upon the angle

            if ( autoBalanceXMode ) {
                double pitchAngleRadians = pitchAngleDegrees * (M_PI / 180.0);
                xAxisRate = sin(pitchAngleRadians) * -1;
            }
            if ( autoBalanceYMode ) {
                double rollAngleRadians = rollAngleDegrees * (M_PI / 180.0);
                yAxisRate = sin(rollAngleRadians) * -1;
            }

            try {
                // Use the joystick X axis for lateral movement, Y axis for forward movement, and Z axis for rotation.
                robotDrive.DriveCartesian(xAxisRate, yAxisRate,stick.GetZ());
            } catch (exception& ex ) {
                string err_string = "Drive system error:  ";
                err_string += ex.what();
                DriverStation::ReportError(err_string.c_str());
            }
            Wait(0.005); // wait 5ms to avoid hogging CPU cycles
        }
    }

};

START_ROBOT_CLASS(Robot);
