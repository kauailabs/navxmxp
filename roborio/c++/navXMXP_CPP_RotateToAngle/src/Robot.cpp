#include "WPILib.h"
#include "AHRS.h"

/* The following PID Controller coefficients will need to be tuned */
/* to match the dynamics of your drive system.  Note that the      */
/* SmartDashboard in Test mode has support for helping you tune    */
/* controllers by displaying a form where you can enter new P, I,  */
/* and D constants and test the mechanism.                         */

const static double kP = 0.03f;
const static double kI = 0.00f;
const static double kD = 0.00f;
const static double kF = 0.00f;

/* This tuning parameter indicates how close to "on target" the    */
/* PID Controller will attempt to get.                             */

const static double kToleranceDegrees = 2.0f;

/**
 * This is a demo program showing the use of the navX MXP to implement
 * a "rotate to angle" feature.
 *
 * This example will automatically rotate the robot to one of four
 * angles (0, 90, 180 and 270 degrees).
 *
 * This rotation can occur when the robot is still, but can also occur
 * when the robot is driving.  When using field-oriented control, this
 * will cause the robot to drive in a straight line, in whatever direction
 * is selected.
 *
 * This example also includes a feature allowing the driver to "reset"
 * the "yaw" angle.  When the reset occurs, the new gyro angle will be
 * 0 degrees.  This can be useful in cases when the gyro drifts, which
 * doesn't typically happen during a FRC match, but can occur during
 * long practice sessions.
 *
 * Note that the PID Controller coefficients defined below will need to
 * be tuned for your drive system.
 */

class Robot: public SampleRobot, public PIDOutput
{

    // Channels for the wheels
    const static int frontLeftChannel	= 2;
    const static int rearLeftChannel	= 3;
    const static int frontRightChannel	= 1;
    const static int rearRightChannel	= 0;

    const static int joystickChannel	= 0;

    RobotDrive robotDrive;	            // Robot drive system
    Joystick stick;			            // Driver Joystick
    AHRS *ahrs;                         // navX MXP
    PIDController *turnController;      // PID Controller
    double rotateToAngleRate;           // Current rotation rate

public:

    /* This function is invoked periodically by the PID Controller, */
    /* based upon navX MXP yaw angle input and PID Coefficients.    */
    virtual void PIDWrite(double output) {
        this->rotateToAngleRate = output;
    }

	Robot() :
            robotDrive(frontLeftChannel, rearLeftChannel,
                       frontRightChannel, rearRightChannel), // initialize variables in same
            stick(joystickChannel)                           // order as declared above.
    {
	    rotateToAngleRate = 0.0f;
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
                /* Use the joystick X axis for lateral movement,          */
                /* Y axis for forward movement, and the current           */
                /* calculated rotation rate (or joystick Z axis),         */
                /* depending upon whether "rotate to angle" is active.    */
                robotDrive.MecanumDrive_Cartesian(stick.GetX(), stick.GetY(),
                                                  currentRotationRate ,ahrs->GetAngle());
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
