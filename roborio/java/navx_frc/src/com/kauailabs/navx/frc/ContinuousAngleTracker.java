/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2015. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
package com.kauailabs.navx.frc;

import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

class ContinuousAngleTracker {

    private float last_angle;
    private double last_rate;
    private int zero_crossing_count;
    private boolean first_sample;
    
    public ContinuousAngleTracker() {
        last_angle = 0.0f;
        zero_crossing_count = 0;
        last_rate = 0;
        first_sample = true;
    }
    
    public void nextAngle( float newAngle ) {
        
    	/* If the first received sample is negative, 
    	 * ensure that the zero crossing count is
    	 * decremented.
    	 */
    	
    	if ( first_sample ) {
    		first_sample = false;
    		if ( newAngle < 0.0f ) {
    			zero_crossing_count--;
    		}
    	}
    	
    	/* Calculate delta angle, adjusting appropriately
    	 * if the current sample crossed the -180/180
    	 * point.
    	 */
    	
    	boolean bottom_crossing = false;
    	float delta_angle = newAngle - last_angle;
        /* Adjust for wraparound at -180/+180 point */
        if ( delta_angle >= 180.0f ){
        	delta_angle = 360.0f - delta_angle;
        	bottom_crossing = true;
        } else if ( delta_angle <= -180.0f ){
        	delta_angle = 360.0f + delta_angle;
        	bottom_crossing = true;
        }
        this.last_rate = delta_angle;

        /* If a zero crossing occurred, increment/decrement
         * the zero crossing count appropriately.
         */
        if ( !bottom_crossing ) {
	        if ( delta_angle < 0.0f ) {
	        	if ( (newAngle < 0.0f) && (last_angle >= 0.0f) ) {
	        		zero_crossing_count--;
	        	}
	        } else if ( delta_angle >= 0.0f ) {
	        	if ( (newAngle >= 0.0f) && (last_angle < 0.0f) ) {
	        		zero_crossing_count++;
	        	}
	        }
        }
        this.last_angle = newAngle;
    }
    
    public double getAngle() {
        double accumulated_angle = (double)zero_crossing_count * 360.0f;
        double curr_angle = (double)last_angle;
        if ( curr_angle < 0.0f ) {
            curr_angle += 360.0f;
        }
        accumulated_angle += curr_angle;
        return accumulated_angle;
    }
    
    public double getRate() {
        return last_rate;
    }
}
