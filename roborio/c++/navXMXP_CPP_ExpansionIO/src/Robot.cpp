#include "WPILib.h"

/**
 * This example program demonstrates the use of the MXP
 * Expansion IO capabilities of the navX MXP, including
 * the following capabilities:
 *
 * -- DIGITAL I/O --
 *
 * - Pulse-Width Modulation [PWM] (e.g., Motor Control)
 * - Digital Inputs               (e.g., Contact Switch closure)
 * - Digital Outputs              (e.g., Relay control)
 * - Quadrature Encoder           (e.g., Wheel Encoder)
 *
 * -- ANALOG I/O --
 *
 * - Analog Inputs                (e.g., Ultrasonic Sensor)
 * - Analog Input Trigger         (e.g., Proximity Sensor trigger)
 * - Analog Trigger Counter
 * - Analog Output                (e.g., Constant-current LED, Sound)
 *
 * This example also demonstrates a simple method for calculating the
 * 'RoboRIO Channel Number' which corresponds to a given navX MXP IO
 * Pin number.
 *
 * For more details on navX MXP Expansion I/O, please see
 * http://navx-mxp.kauailabs.com/installation/io-expansion/
 */

class Robot: public SampleRobot
{

    const static int joystickChannel	= 0;

    Joystick stick;			// Driver Joystick

    /* Digital IO */
    Victor *pwm_out_9;          /* E.g., PWM out to motor controller  */
    Jaguar *pwm_out_8;          /* E.g., PWM out to motor controller  */
    DigitalInput *dig_in_7;     /* E.g., input from contact switch    */
    DigitalInput *dig_in_6;     /* E.g., input from contact switch    */
    DigitalOutput *dig_out_5;   /* E.g., output to relay or LED       */
    DigitalOutput *dig_out_4;   /* E.g., output to relay or LED       */
    Encoder *enc_3and2;         /* E.g., Wheel Encoder                */
    Encoder *enc_1and0;         /* E.g., Wheel Encoder                */

    /* Analog Inputs */
    AnalogInput *an_in_1;       /* E.g., Ultrasonic Sensor            */
    AnalogTrigger *an_trig_0;   /* E.g., Proximity Sensor Threshold   */
    Counter *an_trig_0_counter; /* E.g., Count of an_trig_0 events    */

    /* Analog Outputs */
    AnalogOutput *an_out_1;     /* E.g., Constant-current LED output  */
    AnalogOutput *an_out_0;     /* E.g., Speaker output               */

    enum PinType { DigitalIO, PWM, AnalogIn, AnalogOut };

#define MXP_IO_VOLTAGE (double)3.3f /* Alternately, 5.0f   */
#define MIN_AN_TRIGGER_VOLTAGE (double)0.76f
#define MAX_AN_TRIGGER_VOLTAGE MXP_IO_VOLTAGE - (double)2.0f

public:
    Robot() :
            stick(joystickChannel)
    {
        try {
            /* Construct Digital I/O Objects */
            pwm_out_9 = new Victor(        GetChannelFromPin( PinType::PWM,       9 ));
            pwm_out_8 = new Jaguar(        GetChannelFromPin( PinType::PWM,       8 ));
            dig_in_7  = new DigitalInput(  GetChannelFromPin( PinType::DigitalIO, 7 ));
            dig_in_6  = new DigitalInput(  GetChannelFromPin( PinType::DigitalIO, 6 ));
            dig_out_5 = new DigitalOutput( GetChannelFromPin( PinType::DigitalIO, 5 ));
            dig_out_4 = new DigitalOutput( GetChannelFromPin( PinType::DigitalIO, 4 ));
            enc_3and2 = new Encoder(       GetChannelFromPin( PinType::DigitalIO, 3 ),
                                           GetChannelFromPin( PinType::DigitalIO, 2 ));
            enc_1and0 = new Encoder(       GetChannelFromPin( PinType::DigitalIO, 1 ),
                                           GetChannelFromPin( PinType::DigitalIO, 0 ));

            /* Construct Analog I/O Objects */
            /* NOTE:  Due to a board layout issue on the navX MXP board revision */
            /* 3.3, there is noticeable crosstalk between AN IN pins 3, 2 and 1. */
            /* For that reason, use of pin 3 and pin 2 is NOT RECOMMENDED.       */
            an_in_1   = new AnalogInput(   GetChannelFromPin( PinType::AnalogIn,  1 ));
            an_trig_0 = new AnalogTrigger( GetChannelFromPin( PinType::AnalogIn,  0 ));
            an_trig_0_counter = new Counter( *an_trig_0 );

            an_out_1  = new AnalogOutput(  GetChannelFromPin( PinType::AnalogOut, 1 ));
            an_out_0  = new AnalogOutput(  GetChannelFromPin( PinType::AnalogOut, 0 ));

            /* Configure I/O Objects */
            pwm_out_9->SetSafetyEnabled(false); /* Disable Motor Safety for Demo */
            pwm_out_8->SetSafetyEnabled(false); /* Disable Motor Safety for Demo */

            /* Configure Analog Trigger */
            an_trig_0->SetAveraged(true);
            an_trig_0->SetLimitsVoltage(MIN_AN_TRIGGER_VOLTAGE, MAX_AN_TRIGGER_VOLTAGE);
        } catch (std::exception& ex ) {
            std::string err_string = "Error instantiating MXP pin on navX MXP:  ";
            err_string += ex.what();
            DriverStation::ReportError(err_string.c_str());
        }
	}

    static const int MAX_NAVX_MXP_DIGIO_PIN_NUMBER      = 9;
    static const int MAX_NAVX_MXP_ANALOGIN_PIN_NUMBER   = 3;
    static const int MAX_NAVX_MXP_ANALOGOUT_PIN_NUMBER  = 1;
    static const int NUM_ROBORIO_ONBOARD_DIGIO_PINS     = 10;
    static const int NUM_ROBORIO_ONBOARD_PWM_PINS       = 10;
    static const int NUM_ROBORIO_ONBOARD_ANALOGIN_PINS  = 4;

/* GetChannelFromPin( PinType, int ) - converts from a navX MXP */
/* Pin type and number to the corresponding RoboRIO Channel     */
/* Number, which is used by the WPI Library functions.          */

int GetChannelFromPin( PinType type, int io_pin_number ) {
    int roborio_channel = 0;
    if ( io_pin_number < 0 ) {
        throw std::runtime_error("Error:  navX MXP I/O Pin #");
    }
    switch ( type ) {
    case DigitalIO:
        if ( io_pin_number > MAX_NAVX_MXP_DIGIO_PIN_NUMBER ) {
            throw std::runtime_error("Error:  Invalid navX MXP Digital I/O Pin #");
        }
        roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_DIGIO_PINS +
                          (io_pin_number > 3 ? 4 : 0);
        break;
    case PWM:
        if ( io_pin_number > MAX_NAVX_MXP_DIGIO_PIN_NUMBER ) {
            throw std::runtime_error("Error:  Invalid navX MXP Digital I/O Pin #");
        }
        roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_PWM_PINS;
        break;
    case AnalogIn:
        if ( io_pin_number > MAX_NAVX_MXP_ANALOGIN_PIN_NUMBER ) {
            throw new std::runtime_error("Error:  Invalid navX MXP Analog Input Pin #");
        }
        roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_ANALOGIN_PINS;
        break;
    case AnalogOut:
        if ( io_pin_number > MAX_NAVX_MXP_ANALOGOUT_PIN_NUMBER ) {
            throw new std::runtime_error("Error:  Invalid navX MXP Analog Output Pin #");
        }
        roborio_channel = io_pin_number;
        break;
    }
    return roborio_channel;
}


    /**
     * Runs the motors with Mecanum drive.
     */
    void OperatorControl()
    {
        while (IsOperatorControl() && IsEnabled())
        {
            /* Process Digital I/O (Outputs controlled by joystick) */
            pwm_out_9->Set(stick.GetX());
            pwm_out_8->Set(stick.GetY());
            SmartDashboard::PutBoolean( "DigIn7", dig_in_7->Get());
            SmartDashboard::PutBoolean( "DigIn6", dig_in_6->Get());
            dig_out_5->Set(stick.GetRawButton(1));
            dig_out_4->Set(stick.GetRawButton(2));
            SmartDashboard::PutNumber(  "Enc3and2", enc_3and2->Get());
            SmartDashboard::PutNumber(  "Enc1and0", enc_1and0->Get());

            /* Process Analog Inputs */
            SmartDashboard::PutNumber(  "AnalogIn1",  an_in_1->GetAverageVoltage());
            SmartDashboard::PutBoolean( "AnalogTrigger0",  an_trig_0->GetTriggerState());
            SmartDashboard::PutNumber(  "AnalogTriggerCounter0",  an_trig_0_counter->Get());

            /* Process Analog Outputs (Outputs controlled by joystick) */
            an_out_1->SetVoltage(fabs(stick.GetX()) * MXP_IO_VOLTAGE);
            an_out_0->SetVoltage(fabs(stick.GetY()) * MXP_IO_VOLTAGE);

            Wait(0.005); // wait 5ms to avoid hogging CPU cycles
        }
    }

};

START_ROBOT_CLASS(Robot);
