/*
 * InertialDataIntegrator.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include "InertialDataIntegrator.h"

InertialDataIntegrator::InertialDataIntegrator() {
    ResetDisplacement();
}

void InertialDataIntegrator::UpdateDisplacement( float accel_x_g, float accel_y_g,
        int update_rate_hz, bool is_moving ) {
    if ( is_moving ) {
        float accel_g[2];
        float accel_m_s2[2];
        float curr_velocity_m_s[2];
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

void InertialDataIntegrator::ResetDisplacement() {
    for ( int i = 0; i < 2; i++ ) {
        last_velocity[i] = 0.0f;
        displacement[i] = 0.0f;
    }
}

float InertialDataIntegrator::GetVelocityX() {
    return last_velocity[0];
}

float InertialDataIntegrator::GetVelocityY() {
    return last_velocity[1];
}

float InertialDataIntegrator::GetVelocityZ() {
    return 0;
}

float InertialDataIntegrator::GetDisplacementX() {
    return displacement[0];
}

float InertialDataIntegrator::GetDisplacementY() {
    return displacement[1];
}

float InertialDataIntegrator::GetDisplacementZ() {
     return 0;
}

