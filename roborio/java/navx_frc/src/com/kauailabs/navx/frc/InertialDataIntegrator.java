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

class InertialDataIntegrator {

    private float last_velocity[] = new float[2];
    private float displacement[] = new float[2];

    public InertialDataIntegrator() {
        resetDisplacement();
    }
    
    public void updateDisplacement( float accel_x_g, float accel_y_g, 
            int update_rate_hz, boolean is_moving ) {
        if ( is_moving ) {
            float accel_g[] = new float[2];
            float accel_m_s2[] = new float[2];
            float curr_velocity_m_s[] = new float[2];
            float sample_time = (1.0f / update_rate_hz);
            accel_g[0] = accel_x_g;
            accel_g[1] = accel_y_g;
            for ( int i = 0; i < 2; i++ ) {
                accel_m_s2[i] = accel_g[i] * 9.80665f;
                curr_velocity_m_s[i] = last_velocity[i] + (accel_m_s2[i] * sample_time);
                displacement[i] += last_velocity[i] + (0.5f * accel_m_s2[i] * sample_time * sample_time);
                last_velocity[i] = curr_velocity_m_s[i];
            }
        } else {
            last_velocity[0] = 0.0f;
            last_velocity[1] = 0.0f;
        }
     }
    
    public void resetDisplacement() {
        for ( int i = 0; i < 2; i++ ) {
            last_velocity[i] = 0.0f;
            displacement[i] = 0.0f;
        }        
    }

    public float getVelocityX() {
        return last_velocity[0];
    }

    public float getVelocityY() {
        return last_velocity[1];
    }

    public float getVelocityZ() {
        return 0;
    }

    public float getDisplacementX() {
        return displacement[0];
    }

    public float getDisplacementY() {
        return displacement[1];
    }

    public float getDisplacementZ() {
         return 0;
    }
}
