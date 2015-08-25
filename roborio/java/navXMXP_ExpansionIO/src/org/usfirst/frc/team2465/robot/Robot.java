package org.usfirst.frc.team2465.robot;

import edu.wpi.first.wpilibj.AnalogInput;
import edu.wpi.first.wpilibj.AnalogOutput;
import edu.wpi.first.wpilibj.AnalogTrigger;
import edu.wpi.first.wpilibj.Counter;
import edu.wpi.first.wpilibj.DigitalInput;
import edu.wpi.first.wpilibj.DigitalOutput;
import edu.wpi.first.wpilibj.DriverStation;
import edu.wpi.first.wpilibj.Encoder;
import edu.wpi.first.wpilibj.Jaguar;
import edu.wpi.first.wpilibj.SampleRobot;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.Victor;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

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

public class Robot extends SampleRobot {
    Joystick stick;

    /* Digital IO */
    Victor pwm_out_9;           /* E.g., PWM out to motor controller  */
    Jaguar pwm_out_8;           /* E.g., PWM out to motor controller  */
    DigitalInput dig_in_7;      /* E.g., input from contact switch    */
    DigitalInput dig_in_6;      /* E.g., input from contact switch    */
    DigitalOutput dig_out_5;    /* E.g., output to relay or LED       */
    DigitalOutput dig_out_4;    /* E.g., output to relay or LED       */
    Encoder enc_3and2;          /* E.g., Wheel Encoder                */
    Encoder enc_1and0;          /* E.g., Wheel Encoder                */
    
    /* Analog Inputs */
    AnalogInput an_in_1;        /* E.g., Ultrasonic Sensor            */
    AnalogTrigger an_trig_0;    /* E.g., Proximity Sensor Threshold   */
    Counter an_trig_0_counter;  /* E.g., Count of an_trig_0 events    */
    
    /* Analog Outputs */
    AnalogOutput an_out_1;      /* E.g., Constant-current LED output  */
    AnalogOutput an_out_0;      /* E.g., Speaker output               */
    
    public final double MXP_IO_VOLTAGE = 3.3f; /* Alternately, 5.0f   */;
    public final double MIN_AN_TRIGGER_VOLTAGE = 0.76f;
    public final double MAX_AN_TRIGGER_VOLTAGE = MXP_IO_VOLTAGE - 2.0f;

    public Robot() {
        stick = new Joystick(0);
        try {
            /* Construct Digital I/O Objects */
            pwm_out_9 = new Victor(        getChannelFromPin( PinType.PWM,       9 ));
            pwm_out_8 = new Jaguar(        getChannelFromPin( PinType.PWM,       8 ));
            dig_in_7  = new DigitalInput(  getChannelFromPin( PinType.DigitalIO, 7 ));
            dig_in_6  = new DigitalInput(  getChannelFromPin( PinType.DigitalIO, 6 ));
            dig_out_5 = new DigitalOutput( getChannelFromPin( PinType.DigitalIO, 5 ));
            dig_out_4 = new DigitalOutput( getChannelFromPin( PinType.DigitalIO, 4 ));
            enc_3and2 = new Encoder(       getChannelFromPin( PinType.DigitalIO, 3 ),
                                           getChannelFromPin( PinType.DigitalIO, 2 ));
            enc_1and0 = new Encoder(       getChannelFromPin( PinType.DigitalIO, 1 ),
                                           getChannelFromPin( PinType.DigitalIO, 0 ));
            
            /* Construct Analog I/O Objects */
            /* NOTE:  Due to a board layout issue on the navX MXP board revision */
            /* 3.3, there is noticeable crosstalk between AN IN pins 3, 2 and 1. */
            /* For that reason, use of pin 3 and pin 2 is NOT RECOMMENDED.       */
            an_in_1   = new AnalogInput(   getChannelFromPin( PinType.AnalogIn,  1 ));
            an_trig_0 = new AnalogTrigger( getChannelFromPin( PinType.AnalogIn,  0 ));
            an_trig_0_counter = new Counter( an_trig_0 );
            
            an_out_1  = new AnalogOutput(  getChannelFromPin( PinType.AnalogOut, 1 ));
            an_out_0  = new AnalogOutput(  getChannelFromPin( PinType.AnalogOut, 0 ));
            
            /* Configure I/O Objects */
            pwm_out_9.setSafetyEnabled(false); /* Disable Motor Safety for Demo */
            pwm_out_8.setSafetyEnabled(false); /* Disable Motor Safety for Demo */
            
            /* Configure Analog Trigger */
            an_trig_0.setAveraged(true);
            an_trig_0.setLimitsVoltage(MIN_AN_TRIGGER_VOLTAGE, MAX_AN_TRIGGER_VOLTAGE);
        } catch (RuntimeException ex ) {
            DriverStation.reportError( "Error instantiating MXP pin on navX MXP:  " 
                                        + ex.getMessage(), true);
        }
    }

    /**
     * Wait for 2 seconds then stop
     */
    public void autonomous() {
        Timer.delay(2.0);		     //  for 2 seconds
    }

    public enum PinType { DigitalIO, PWM, AnalogIn, AnalogOut };
    
    public final int MAX_NAVX_MXP_DIGIO_PIN_NUMBER      = 9;
    public final int MAX_NAVX_MXP_ANALOGIN_PIN_NUMBER   = 3;
    public final int MAX_NAVX_MXP_ANALOGOUT_PIN_NUMBER  = 1;
    public final int NUM_ROBORIO_ONBOARD_DIGIO_PINS     = 10;
    public final int NUM_ROBORIO_ONBOARD_PWM_PINS       = 10;
    public final int NUM_ROBORIO_ONBOARD_ANALOGIN_PINS  = 4;
    
    /* getChannelFromPin( PinType, int ) - converts from a navX MXP */
    /* Pin type and number to the corresponding RoboRIO Channel     */
    /* Number, which is used by the WPI Library functions.          */
    
    public int getChannelFromPin( PinType type, int io_pin_number ) 
               throws IllegalArgumentException {
        int roborio_channel = 0;
        if ( io_pin_number < 0 ) {
            throw new IllegalArgumentException("Error:  navX MXP I/O Pin #");
        }
        switch ( type ) {
        case DigitalIO:
            if ( io_pin_number > MAX_NAVX_MXP_DIGIO_PIN_NUMBER ) {
                throw new IllegalArgumentException("Error:  Invalid navX MXP Digital I/O Pin #");
            }
            roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_DIGIO_PINS + 
                              (io_pin_number > 3 ? 4 : 0);
            break;
        case PWM:
            if ( io_pin_number > MAX_NAVX_MXP_DIGIO_PIN_NUMBER ) {
                throw new IllegalArgumentException("Error:  Invalid navX MXP Digital I/O Pin #");
            }
            roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_PWM_PINS;
            break;
        case AnalogIn:
            if ( io_pin_number > MAX_NAVX_MXP_ANALOGIN_PIN_NUMBER ) {
                throw new IllegalArgumentException("Error:  Invalid navX MXP Analog Input Pin #");
            }
            roborio_channel = io_pin_number + NUM_ROBORIO_ONBOARD_ANALOGIN_PINS;
            break;
        case AnalogOut:
            if ( io_pin_number > MAX_NAVX_MXP_ANALOGOUT_PIN_NUMBER ) {
                throw new IllegalArgumentException("Error:  Invalid navX MXP Analog Output Pin #");
            }
            roborio_channel = io_pin_number;            
            break;
        }
        return roborio_channel;
    }
    
    /**
     * Runs the motors with arcade steering.
     */
    public void operatorControl() {
        while (isOperatorControl() && isEnabled()) {
            
            /* Process Digital I/O (Outputs controlled by joystick) */
            pwm_out_9.set(stick.getX());
            pwm_out_8.set(stick.getY());
            SmartDashboard.putBoolean( "DigIn7", dig_in_7.get());
            SmartDashboard.putBoolean( "DigIn6", dig_in_6.get());
            dig_out_5.set(stick.getRawButton(1));
            dig_out_4.set(stick.getRawButton(2));
            SmartDashboard.putNumber(  "Enc3and2", enc_3and2.get());
            SmartDashboard.putNumber(  "Enc1and0", enc_1and0.get());
            
            /* Process Analog Inputs */
            SmartDashboard.putNumber(  "AnalogIn1",  an_in_1.getAverageVoltage());
            SmartDashboard.putBoolean( "AnalogTrigger0",  an_trig_0.getTriggerState());
            SmartDashboard.putNumber(  "AnalogTriggerCounter0",  an_trig_0_counter.get());
            
            /* Process Analog Outputs (Outputs controlled by joystick) */
            an_out_1.setVoltage(Math.abs(stick.getX()) * MXP_IO_VOLTAGE);
            an_out_0.setVoltage(Math.abs(stick.getY()) * MXP_IO_VOLTAGE);
            
            Timer.delay(0.005);		/* wait for a motor update time */
        }
    }

    /**
     * Runs during test mode
     */
    public void test() {
    }
}
