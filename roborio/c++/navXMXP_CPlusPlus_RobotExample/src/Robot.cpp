#include "WPILib.h"
#include "IMU.h"
#include "IMUAdvanced.h"
#include "AHRS.h"

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */
class Robot : public SampleRobot
{
	NetworkTable *table;
	AHRS *imu;
	SerialPort *serial_port;
	bool first_iteration;

public:
	Robot()
	{
	}

	void RobotInit() {

		table = NetworkTable::GetTable("datatable");
		serial_port = new SerialPort(57600,SerialPort::kMXP);
        uint8_t update_rate_hz = 50;
        imu = new AHRS(serial_port,update_rate_hz);
        if ( imu ) {
        	LiveWindow::GetInstance()->AddSensor("IMU", "Gyro", imu);
        }
        first_iteration = true;
	}

	/**
	 *
	 * Wait for 10 seconds
	 */
	void Autonomous(void)
	{
		Wait(2.0); 				//    for 10 seconds
	}

	/**
	 * Runs the motors with arcade steering.
	 */
	void OperatorControl(void)
	{
		int reset_yaw_count = 0;
		int reset_displacement_count = 0;
		while (IsOperatorControl())
		{
			if ( first_iteration ) {
	            bool is_calibrating = imu->IsCalibrating();
	            if ( !is_calibrating ) {
	                Wait( 0.3 );
	                imu->ZeroYaw();
	                first_iteration = false;
	            }
			}
            bool yaw_axis_up;
            uint8_t yaw_axis = imu->GetBoardYawAxis(yaw_axis_up);

            bool reset_yaw_button_pressed = DriverStation::GetInstance()->GetStickButton(0,1);
            if ( reset_yaw_button_pressed ) {
            	imu->ZeroYaw();
            	reset_yaw_count++;
            }
            bool reset_displacement_button_pressed = DriverStation::GetInstance()->GetStickButton(0,2);
            if ( reset_displacement_button_pressed ) {
            	imu->ResetDisplacement();
            	reset_displacement_count++;
            }

            SmartDashboard::PutNumber("Reset_Yaw_Count", reset_yaw_count);
            SmartDashboard::PutNumber("Reset_Displacement_Count", reset_displacement_count);
            SmartDashboard::PutBoolean("Yaw_Axis_Up",       yaw_axis_up);
            SmartDashboard::PutNumber( "Yaw_Axis", 			yaw_axis);

			SmartDashboard::PutBoolean( "IMU_Connected", imu->IsConnected());
			SmartDashboard::PutNumber("IMU_Yaw", imu->GetYaw());
			SmartDashboard::PutNumber("IMU_Pitch", imu->GetPitch());
			SmartDashboard::PutNumber("IMU_Roll", imu->GetRoll());
			SmartDashboard::PutNumber("IMU_CompassHeading", imu->GetCompassHeading());
			SmartDashboard::PutNumber("IMU_Update_Count", imu->GetUpdateCount());
			SmartDashboard::PutNumber("IMU_Byte_Count", imu->GetByteCount());

		    /* These functions are compatible w/the WPI Gyro Class */
			SmartDashboard::PutNumber(   "IMU_TotalYaw", 		imu->GetAngle());
			SmartDashboard::PutNumber(   "IMU_YawRateDPS",		imu->GetRate());

	        SmartDashboard::PutNumber("IMU_Accel_X", imu->GetWorldLinearAccelX());
			SmartDashboard::PutNumber("IMU_Accel_Y", imu->GetWorldLinearAccelY());
			SmartDashboard::PutBoolean("IMU_IsMoving", imu->IsMoving());
			SmartDashboard::PutNumber("IMU_Temp_C", imu->GetTempC());
            SmartDashboard::PutBoolean("IMU_IsCalibrating", imu->IsCalibrating());

            SmartDashboard::PutNumber("Velocity_X",       	imu->GetVelocityX() );
            SmartDashboard::PutNumber("Velocity_Y",       	imu->GetVelocityY() );
            SmartDashboard::PutNumber("Displacement_X",     imu->GetDisplacementX() );
            SmartDashboard::PutNumber("Displacement_Y",     imu->GetDisplacementY() );

            Wait(0.1);				// wait for a while
		}
	}

	/**
	 * Runs during test mode
	 */
	void Test() {

	}

};

START_ROBOT_CLASS(Robot);


