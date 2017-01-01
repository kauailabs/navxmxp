/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2015. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
/* This code is based upon source code from Joe Ross, FRC Team 330 Beachbotics */
/* Source:  https://github.com/Beachbot330/Beachbot2016Java/blob/master/src/
 * org/usfirst/frc330/subsystems/Chassis.java#L310
 */
package com.kauailabs.navx.frc;

class ContinuousAngleTracker {

    private boolean fFirstUse;
    private double gyro_prevVal;
    private int ctrRollOver;
    float curr_yaw_angle;
    float last_yaw_angle;
    double angleAdjust;
    
    public ContinuousAngleTracker() {
    	init();
        angleAdjust = 0.0f;    	
    }    

    private void init() {
        gyro_prevVal = 0.0;
        ctrRollOver  = 0;
        fFirstUse = true;
        last_yaw_angle = 0.0f;        
        curr_yaw_angle = 0.0f;
    }
    
    public void nextAngle( float newAngle ) {
		synchronized(this){
			last_yaw_angle = curr_yaw_angle;
			curr_yaw_angle = newAngle;
		}
    }
    
    /* Invoked (internally) whenever yaw reset occurs. */
    public void reset() {
		synchronized(this){
			init();
		}
    }
    
    public double getAngle() {
		// First case
		// Old reading: +150 degrees
		// New reading: +170 degrees
		// Difference:  (170 - 150) = +20 degrees
		
		// Second case
		// Old reading: -20 degrees
		// New reading: -50 degrees
		// Difference : (-50 - -20) = -30 degrees 
		
		// Third case
		// Old reading: +179 degrees
		// New reading: -179 degrees
		// Difference:  (-179 - 179) = -358 degrees
		
		// Fourth case
		// Old reading: -179  degrees
		// New reading: +179 degrees
		// Difference:  (+179 - -179) = +358 degrees
		
		double difference;
		double gyroVal;
		double yawVal;
		
		synchronized(this) {
			yawVal = curr_yaw_angle;
			
			// Has gyro_prevVal been previously set?
			// If not, return do not calculate, return current value
			if( !fFirstUse )
			{
				// Determine count for rollover counter
				difference = yawVal - gyro_prevVal;

				/* Clockwise past +180 degrees
				 * If difference > 180*, increment rollover counter */
				if( difference < -180.0 ) {
					ctrRollOver++;

				/* Counter-clockwise past -180 degrees:
				 * If difference > 180*, decrement rollover counter */
				}
				else if ( difference > 180.0 ) {
					ctrRollOver--;
				}
			}
			
			// Mark gyro_prevVal as being used
			fFirstUse = false;
				
			// Calculate value to return back to calling function
			// e.g. +720 degrees or -360 degrees
			gyroVal = yawVal + (360.0 * ctrRollOver);
			gyro_prevVal = yawVal;
			
			return gyroVal + angleAdjust;
		}
    }
    
	public void setAngleAdjustment(double adjustment) {
		angleAdjust = adjustment;
	}
    
	public double getAngleAdjustment() {
		return angleAdjust;
	}  
    
    public double getRate() {
		float difference;
		synchronized(this) {
			difference = curr_yaw_angle - last_yaw_angle;
		}    	
		if ( difference > 180.0f) {
			/* Clockwise past +180 degrees */
			difference = 360.0f - difference;
		} else if ( difference < -180.0f) {
			/* Counter-clockwise past -180 degrees */
			difference = 360.0f + difference;
		}
		return difference;
    }
}
