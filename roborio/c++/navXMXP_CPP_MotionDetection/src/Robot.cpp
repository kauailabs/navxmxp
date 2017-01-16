#include "WPILib.h"
#include "AHRS.h"

/**
 * This is a demo program showing the use of the navX MXP to implement
 * a motion detection feature, which can be used to detect when your
 * robot is still and when it is not.
 *
 * The basic principle used within the Motion Detection example
 * is to subtract the acceleration due to gravity from the raw
 * acceleration values. The result value is known as “world linear
 * acceleration”, representing the actual amount of acceleration
 * due to motion, and is calculated automatically by navX MXP’s
 * motion processor.  Whenever the sum of the world linear
 * acceleration in both the X and Y axes exceeds a “motion
 * threshold”, motion is occurring.  The navX MXP makes it easy,
 * simply invoke the "isMoving()" function to determine if motion
 * is currently underway.
 */

class Robot: public SampleRobot
{

    // Channels for the wheels
    const static int frontLeftChannel	= 2;
    const static int rearLeftChannel	= 3;
    const static int frontRightChannel	= 1;
    const static int rearRightChannel	= 0;

    const static int joystickChannel	= 0;

    RobotDrive robotDrive;	// Robot drive system
    Joystick stick;			// Driver Joystick
    AHRS *ahrs;             // navX MXP

public:
    Robot() :
            robotDrive(frontLeftChannel, rearLeftChannel,
                       frontRightChannel, rearRightChannel),	// initialize variables in
            stick(joystickChannel)								// same order declared above
    {
        robotDrive.SetExpiration(0.1);
        robotDrive.SetInvertedMotor(RobotDrive::kFrontLeftMotor, true);	// invert left side motors
        robotDrive.SetInvertedMotor(RobotDrive::kRearLeftMotor, true);	// change to match your robot
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
        } catch (std::exception& ex ) {
            std::string err_string = "Error instantiating navX MXP:  ";
            err_string += ex.what();
            DriverStation::ReportError(err_string.c_str());
        }
        if ( ahrs ) {
            LiveWindow::GetInstance()->AddSensor("IMU", "Gyro", ahrs);
        }
	}

    /**
     * Runs the motors with Mecanum drive.
     */
    void OperatorControl()
    {
        robotDrive.SetSafetyEnabled(false);
        while (IsOperatorControl() && IsEnabled())
        {
            bool motionDetected = ahrs->IsMoving();
            SmartDashboard::PutBoolean("MotionDetected", motionDetected);

            try {
                /* Use the joystick X axis for lateral movement,            */
                /* Y axis for forward movement, and Z axis for rotation.    */
                /* Use navX MXP yaw angle to define Field-centric transform */
                robotDrive.MecanumDrive_Cartesian(stick.GetX(), stick.GetY(),
                                                  stick.GetZ(),ahrs->GetAngle());
            } catch (std::exception& ex ) {
                std::string err_string = "Error communicating with Drive System:  ";
                err_string += ex.what();
                DriverStation::ReportError(err_string.c_str());
            }
            Wait(0.005); // wait 5ms to avoid hogging CPU cycles
        }
    }

};

START_ROBOT_CLASS(Robot);
