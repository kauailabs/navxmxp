/*
 * SimIO.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "SimIO.h"
#include "IMURegisters.h"
#include "delay.h"

using namespace frc;

SimIO::SimIO( uint8_t update_rate_hz,
                IIOCompleteNotification *notify_sink,
                hal::SimDevice *sim_device) {
    this->update_rate_hz        = update_rate_hz;
    this->notify_sink           = notify_sink;
    this->stop                  = false;
    this->start_seconds         = 0.0;
    this->is_connected          = false;

    raw_data_update = {};
    ahrs_update     = {};
    board_state     = {};
    board_id        = {};

    if (sim_device) {
            
        // Booleans
        
        this->simConnected  = sim_device->CreateBoolean("Connected", false, true);

        // Doubles
        this->simYaw                = sim_device->CreateDouble("Yaw", false, 0.0f);
        this->simPitch              = sim_device->CreateDouble("Pitch", false, 0.0f);
        this->simRoll               = sim_device->CreateDouble("Roll", false, 0.0f);
        this->simCompassHeading     = sim_device->CreateDouble("CompassHeading", false, 0.0f);
        this->simFusedHeading       = sim_device->CreateDouble("FusedHeading", false, 0.0f);                        

        this->simLinearWorldAccelX  = sim_device->CreateDouble("LinearWorldAccelX", false, 0.0f);
        this->simLinearWorldAccelY  = sim_device->CreateDouble("LinearWorldAccelY", false, 0.0f);
        this->simLinearWorldAccelZ  = sim_device->CreateDouble("LinearWorldAccelZ", false, 0.0f);

        board_id.fw_ver_major = 3;
        board_id.fw_ver_minor = 1;
        board_id.fw_revision = 400;
        board_id.type = 33; // navx-MXP type id        

        board_state.selftest_status = 
            NAVX_SELFTEST_STATUS_COMPLETE |
            NAVX_SELFTEST_RESULT_GYRO_PASSED |
            NAVX_SELFTEST_RESULT_ACCEL_PASSED |
            NAVX_SELFTEST_RESULT_MAG_PASSED; // BARO Passed is NOT set 
        board_state.sensor_status =
            // NAVX_SENSOR_STATUS_MOVING |              // NOTE:  Updated by Sim Variable
            NAVX_SENSOR_STATUS_YAW_STABLE |
            // NAVX_SENSOR_STATUS_MAG_DISTURBANCE |     // NOTE:  Always false
            // NAVX_SENSOR_STATUS_ALTITUDE_VALID |      // NOTE:  Always false
            // NAVX_SENSOR_STATUS_SEALEVEL_PRESS_SET |  // NOTE:  Always false
            NAVX_SENSOR_STATUS_FUSED_HEADING_VALID;            
        board_state.op_status =
            NAVX_OP_STATUS_NORMAL;
        board_state.cal_status = 
            NAV6_CALIBRATION_STATE_COMPLETE;
        board_state.capability_flags =
                // Note:  Configure capabilities to NOT include VEL_AND_DISP,
                // this forces the internal InertialDataIntegrator to be used.
                // NAVX_CAPABILITY_FLAG_VEL_AND_DISP |
                NAVX_CAPABILITY_FLAG_OMNIMOUNT |
                NAVX_CAPABILITY_FLAG_AHRSPOS_TS |
                // Note:  Configure capabilities to NOT include HW-based YAW RESET;
                // This causes software-based yaw reset to be used
                // NAVX_CAPABILITY_FLAG_YAW_RESET |                
                OMNIMOUNT_DEFAULT;        
        board_state.accel_fsr_g = 2;
        board_state.gyro_fsr_dps = 2000;                
        board_state.update_rate_hz = this->update_rate_hz;    

        /////////////////////////        
        // AHRSUpdate initialization
        ///////////////////////// 
        
        ahrs_update.mpu_temp = 35.0f;

        // Set Quaternion values to default (identity)        
        ahrs_update.quat_w = 0.0f;
        ahrs_update.quat_x = 0.0f;
        ahrs_update.quat_y = 0.0f;
        ahrs_update.quat_z = 1.0f;
        
        ahrs_update.vel_x = 0.0f;
        ahrs_update.vel_y = 0.0f;
        ahrs_update.vel_z = 0.0f;

        ahrs_update.disp_x = 0.0f;
        ahrs_update.disp_y = 0.0f;
        ahrs_update.disp_z = 0.0f;

        ahrs_update.altitude = 50.0f;   // Average surface altitude, planet earth
        ahrs_update.barometric_pressure = 1013.25f; // Nominal pressure at earth's surface

        // Synchronize AHRS Update and Board State values

        ahrs_update.cal_status = board_state.cal_status;
        ahrs_update.op_status = board_state.op_status;
        ahrs_update.selftest_status = board_state.selftest_status;          
        ahrs_update.sensor_status = board_state.sensor_status;

        // The following values are updated by Sim Variables later; set to reasonable defaults now

        ahrs_update.linear_accel_x = 0.0f; 
        ahrs_update.linear_accel_y = 0.0f;
        ahrs_update.linear_accel_z = 0.0f;        
        ahrs_update.yaw = 0.0f;
        ahrs_update.pitch = 0.0f;
        ahrs_update.roll = 0.0f;
        ahrs_update.compass_heading = 0.0f;
        ahrs_update.fused_heading = 0.0f;

        // RawUpdate initialization

        raw_data_update.mag_x = 40;
        raw_data_update.mag_y = 40;
        raw_data_update.mag_z = 40;                

        // Set simulated raw accel/gyro values to values representing noise
        raw_data_update.accel_x = 50;
        raw_data_update.accel_y = 50;
        raw_data_update.accel_z = 50;

        raw_data_update.gyro_x = 50;
        raw_data_update.gyro_y = 50;
        raw_data_update.gyro_z = 50;

        // State history initialization
        last_yaw = 0;
        last_linear_world_accel_x = 0;
        last_linear_world_accel_y = 0;    

        printf("navX-Sensor SimDevice created.\n");
    }
}

SimIO::~SimIO() {
}

bool SimIO::IsConnected() {
    return is_connected;
}

double SimIO::GetByteCount() {
    #define SIM_BYTES_PER_SECOND 1000
    double num_secs_running = Timer::GetFPGATimestamp() - start_seconds;
    return num_secs_running * SIM_BYTES_PER_SECOND;
}

double SimIO::GetUpdateCount() {
    double num_secs_running = Timer::GetFPGATimestamp() - start_seconds;
    return num_secs_running / update_rate_hz;
}

void SimIO::SetUpdateRateHz(uint8_t update_rate) {
    this->update_rate_hz = update_rate;
}

void SimIO::ZeroYaw() {
    notify_sink->YawResetComplete();
}

void SimIO::ZeroDisplacement() {
}

void SimIO::Run() {
    /* IO Loop */
    // Simulate startup delay
    delayMillis(50);

    // Default to connected state
    is_connected = true;
    notify_sink->ConnectDetected();

    long sensor_timestamp = 2000; // NOTE:  Simulate a 2-second navX-sensor firmware startup delay

    // Update all static values
    notify_sink->SetBoardID(board_id);   
    notify_sink->SetRawData(raw_data_update, sensor_timestamp);

    // Update AHRS data (portions of which are static; others are updated from sim variables)
    notify_sink->SetAHRSPosData(ahrs_update, sensor_timestamp);        

    start_seconds = Timer::GetFPGATimestamp();

    while (!stop) {
        delayMillis(20);
        sensor_timestamp += 20;
        UpdatePeriodicFromSimVariables(sensor_timestamp);            
    }
}

float SimIO::CalculateNormal(float in1, float in2) {
    return fabs(sqrt(fabs(in1*in1)+fabs(in2*in2)));
}

void SimIO::UpdatePeriodicFromSimVariables(long sensor_timestamp) {
    
    if (!sim_device) return;

    bool curr_is_connected =     simConnected.Get();

    // Update connection state
    if (curr_is_connected != is_connected) {
        is_connected = curr_is_connected;
        if (curr_is_connected) {
            notify_sink->ConnectDetected();                
        } else {
            notify_sink->DisconnectDetected();                
        }
    }          

    if (curr_is_connected) {
        ahrs_update.yaw =               (float)simYaw.Get();
        ahrs_update.pitch =             (float)simPitch.Get();
        ahrs_update.roll =              (float)simRoll.Get();
        ahrs_update.compass_heading =   (float)simCompassHeading.Get();
        ahrs_update.fused_heading =     (float)simFusedHeading.Get();
        ahrs_update.linear_accel_x =    (float)simLinearWorldAccelX.Get();
        ahrs_update.linear_accel_y =    (float)simLinearWorldAccelY.Get();
        ahrs_update.linear_accel_z =    (float)simLinearWorldAccelZ.Get();  
    
        // Detect motion
        float last_linear_accel_norm = CalculateNormal(last_linear_world_accel_x, last_linear_world_accel_y);
        float curr_linear_accel_norm = CalculateNormal(ahrs_update.linear_accel_x, ahrs_update.linear_accel_y);
        bool curr_is_moving = (fabs(curr_linear_accel_norm - last_linear_accel_norm) >= 0.02f);
        if (curr_is_moving) {
            board_state.sensor_status |= NAVX_SENSOR_STATUS_MOVING;
        } else {
            board_state.sensor_status &= ~(NAVX_SENSOR_STATUS_MOVING);
        }

        // Detect rotation
        bool curr_is_rotating = fabs(last_yaw - ahrs_update.yaw) >= 0.05f;
        if (!curr_is_rotating) {
            board_state.sensor_status |= NAVX_SENSOR_STATUS_YAW_STABLE;
        } else {
            board_state.sensor_status &= ~(NAVX_SENSOR_STATUS_YAW_STABLE);
        }

        ahrs_update.sensor_status = board_state.sensor_status;

        // Trigger simulated update to notification sink
        notify_sink->SetAHRSPosData(ahrs_update, sensor_timestamp);

        // Update cached state variables
        last_yaw = ahrs_update.yaw;
        last_linear_world_accel_x = ahrs_update.linear_accel_x;
        last_linear_world_accel_y = ahrs_update.linear_accel_y;            
    }             
}
        
double SimIO::GetRate() {
    if (!sim_device) return 0;
    
    return simRate.Get();
}


void SimIO::Stop() {
    stop = true;
}

void SimIO::EnableLogging(bool enable) {
}



