#include "WPILib.h"
#include "AHRS.h"
#include <math.h>
/**
 * This is a demo program showing how to use Mecanum control with the RobotDrive class.
 */

static const double kOffBalanceAngleThresholdDegrees = 10;
static const double kOonBalanceAngleThresholdDegrees  = 5;

class Robot: public SampleRobot
{

    // Channels for the wheels
    const static int frontLeftChannel	= 2;
    const static int rearLeftChannel	= 3;
    const static int frontRightChannel	= 1;
    const static int rearRightChannel	= 0;

    const static int joystickChannel	= 0;

	RobotDrive robotDrive;	// robot drive system
	Joystick stick;			// only joystick
	AHRS *ahrs;
    bool autoBalanceXMode;
    bool autoBalanceYMode;

public:
	Robot() :
			robotDrive(frontLeftChannel, rearLeftChannel,
					   frontRightChannel, rearRightChannel),	// these must be initialized in the same order
			stick(joystickChannel)								// as they are declared above.
	{
		robotDrive.SetExpiration(0.1);
		robotDrive.SetInvertedMotor(RobotDrive::kFrontLeftMotor, true);	// invert the left side motors
		robotDrive.SetInvertedMotor(RobotDrive::kRearLeftMotor, true);	// you may need to change or remove this to match your robot
        try {
            /* Communicate w/navX MXP via the MXP SPI Bus.                                       */
            /* Alternatively:  I2C::Port::kMXP, SerialPort::Port::kMXP or SerialPort::Port::kUSB */
            /* See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface/ for details.   */
            ahrs = new AHRS(SPI::Port::kMXP);
        } catch (std::exception ex ) {
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
	        bool reset_yaw_button_pressed = stick.GetRawButton(1);
	        if ( reset_yaw_button_pressed ) {
	            ahrs->ZeroYaw();
	        }
            double xAxisRate = stick.GetX();
            double yAxisRate = stick.GetY();
            double pitchAngleDegrees = ahrs->GetPitch();
            double rollAngleDegrees = ahrs->GetRoll();
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
                double pitchAngleRadians = pitchAngleDegrees * (M_PI / 180.0);
                xAxisRate = sin(pitchAngleRadians) * -1;
            }
            if ( autoBalanceYMode ) {
                double rollAngleRadians = rollAngleDegrees * (M_PI / 180.0);
                yAxisRate = sin(rollAngleRadians) * -1;
            }

            try {
                // Use the joystick X axis for lateral movement, Y axis for forward movement, and Z axis for rotation.
                robotDrive.MecanumDrive_Cartesian(xAxisRate, yAxisRate, stick.GetZ(),ahrs->GetAngle());
            } catch (std::exception ex ) {
                std::string err_string = "Error communicating with Drive System:  ";
                err_string += ex.what();
                DriverStation::ReportError(err_string.c_str());
            }
			Wait(0.005); // wait 5ms to avoid hogging CPU cycles
		}
	}

};

START_ROBOT_CLASS(Robot);
