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

class ContinuousAngleTracker {

    private float last_angle;
    private double last_rate;
    private int zero_crossing_count;
    
    public ContinuousAngleTracker() {
        last_angle = 0.0f;
        zero_crossing_count = 0;
        last_rate = 0;
    }
    
    public void nextAngle( float newAngle ) {
        
        int angle_last_direction;
        float adjusted_last_angle = ( last_angle < 0.0f ) ? last_angle + 360.0f : last_angle;
        float adjusted_curr_angle = ( newAngle < 0.0f ) ? newAngle + 360.0f : newAngle;
        float delta_angle = adjusted_curr_angle - adjusted_last_angle;
        this.last_rate = delta_angle;

        angle_last_direction = 0;
        if ( adjusted_curr_angle < adjusted_last_angle ) {
            if ( delta_angle < -180.0f ) {
                angle_last_direction = -1;
            } else {
                angle_last_direction = 1;
            }
        } else if ( adjusted_curr_angle > adjusted_last_angle ) {
            if ( delta_angle > 180.0f ) {
                angle_last_direction = -1;
            } else {
                angle_last_direction = 1;
            }
        }

        if ( angle_last_direction < 0 ) {
            if ( ( adjusted_curr_angle < 0.0f ) && ( adjusted_last_angle >= 0.0f ) ) {
                zero_crossing_count--;
            }           
        } else if ( angle_last_direction > 0 ) {
            if ( ( adjusted_curr_angle >= 0.0f ) && ( adjusted_last_angle < 0.0f ) ) {
                zero_crossing_count++;
            }           
        }
        last_angle = newAngle;
        
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
