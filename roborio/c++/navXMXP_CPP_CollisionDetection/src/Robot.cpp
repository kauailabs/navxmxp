#include "WPILib.h"
#include "AHRS.h"

/**
 * This is a demo program showing the use of the navX MXP to implement
 * a collision detection feature, which can be used to detect events
 * while driving a robot, such as bumping into a wall or being hit
 * by another robot.
 *
 * The basic principle used within the Collision Detection example
 * is the calculation of Jerk (which is defined as the change in
 * acceleration).  In the sample code shown below, both the X axis and
 * the Y axis jerk are calculated, and if either exceeds a threshold,
 * then a collision has occurred.
 *
 * The 'collision threshold' used in these samples will likely need to
 * be tuned for your robot, since the amount of jerk which constitutes
 * a collision will be dependent upon the robot mass and expected
 * maximum velocity of either the robot, or any object which may strike
 * the robot.
 */

#define COLLISION_THRESHOLD_DELTA_G 0.5f

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
    double last_world_linear_accel_x;
    double last_world_linear_accel_y;

public:
    Robot() :
            robotDrive(frontLeftChannel, rearLeftChannel,
                       frontRightChannel, rearRightChannel),	// initialize variables in
            stick(joystickChannel)								// same order declared above
    {
        last_world_linear_accel_x = 0.0f;
        last_world_linear_accel_y = 0.0f;
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
            bool collisionDetected = false;

            double curr_world_linear_accel_x = ahrs->GetWorldLinearAccelX();
            double currentJerkX = curr_world_linear_accel_x - last_world_linear_accel_x;
            last_world_linear_accel_x = curr_world_linear_accel_x;
            double curr_world_linear_accel_y = ahrs->GetWorldLinearAccelY();
            double currentJerkY = curr_world_linear_accel_y - last_world_linear_accel_y;
            last_world_linear_accel_y = curr_world_linear_accel_y;

            if ( ( fabs(currentJerkX) > COLLISION_THRESHOLD_DELTA_G ) ||
                 ( fabs(currentJerkY) > COLLISION_THRESHOLD_DELTA_G) ) {
                collisionDetected = true;
            }
            SmartDashboard::PutBoolean(  "CollisionDetected", collisionDetected);

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
