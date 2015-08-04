#include "WPILib.h"
#include "AHRS.h"

const static double kP = 0.03f;
const static double kI = 0.00f;
const static double kD = 0.00f;
const static double kF = 0.00f;

const static double kToleranceDegrees = 2.0f;

/**
 * This is a demo program showing how to use Mecanum control with the RobotDrive class.
 */
class Robot: public SampleRobot, public PIDOutput
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
    PIDController *turnController;
    double rotateToAngleRate;

    /* The following PID Controller coefficients will need to be tuned */
    /* to match the dynamics of your drive system.  Note that the      */
    /* SmartDashboard in Test mode has support for helping you tune    */
    /* controllers by displaying a form where you can enter new P, I,  */
    /* and D constants and test the mechanism.                         */

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
        turnController = new PIDController(kP, kI, kD, kF, ahrs, this);
        turnController->SetInputRange(-180.0f,  180.0f);
        turnController->SetOutputRange(-1.0, 1.0);
        turnController->SetAbsoluteTolerance(kToleranceDegrees);
        turnController->SetContinuous(true);

        /* Add the PID Controller to the Test-mode dashboard, allowing manual  */
        /* tuning of the Turn Controller's P, I and D coefficients.            */
        /* Typically, only the P value needs to be modified.                   */
        LiveWindow::GetInstance()->AddActuator("DriveSystem", "RotateController", turnController);
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
	        bool rotateToAngle = false;
            if ( stick.GetRawButton(2)) {
                turnController->SetSetpoint(0.0f);
                rotateToAngle = true;
            } else if ( stick.GetRawButton(3)) {
                turnController->SetSetpoint(90.0f);
                rotateToAngle = true;
            } else if ( stick.GetRawButton(4)) {
                turnController->SetSetpoint(179.9f);
                rotateToAngle = true;
            } else if ( stick.GetRawButton(5)) {
                turnController->SetSetpoint(-90.0f);
                rotateToAngle = true;
            }
            double currentRotationRate;
            if ( rotateToAngle ) {
                turnController->Enable();
                currentRotationRate = rotateToAngleRate;
            } else {
                turnController->Disable();
                currentRotationRate = stick.GetTwist();
            }
            try {
                // Use the joystick X axis for lateral movement, Y axis for forward movement, and Z axis for rotation.
                robotDrive.MecanumDrive_Cartesian(stick.GetX(), stick.GetY(), currentRotationRate ,ahrs->GetAngle());
            } catch (std::exception ex ) {
                std::string err_string = "Error communicating with Drive System:  ";
                err_string += ex.what();
                DriverStation::ReportError(err_string.c_str());
            }
			Wait(0.005); // wait 5ms to avoid hogging CPU cycles
		}
	}
    void PIDWrite(float output) {
        rotateToAngleRate = output;
    }

};

START_ROBOT_CLASS(Robot);
