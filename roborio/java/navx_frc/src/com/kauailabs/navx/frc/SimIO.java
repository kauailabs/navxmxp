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

import edu.wpi.first.wpilibj.Timer;
import com.kauailabs.navx.AHRSProtocol;
import com.kauailabs.navx.AHRSProtocol.AHRSPosUpdate;
import com.kauailabs.navx.IMUProtocol.GyroUpdate;
import com.kauailabs.navx.IMUProtocol;

import edu.wpi.first.hal.SimBoolean;
import edu.wpi.first.hal.SimDevice;
import edu.wpi.first.hal.SimDouble;

class SimIO implements IIOProvider {  
    
    boolean stop = false;
    boolean is_connected = false;
    double start_seconds = 0;
    IIOCompleteNotification notify_sink;
    byte update_rate_hz;

    private SimDevice sim_device;
    private SimBoolean simConnected;
    private SimDouble simRate;
    private SimDouble simYaw;
    private SimDouble simPitch;
    private SimDouble simRoll;
    private SimDouble simCompassHeading;
    private SimDouble simFusedHeading;    
    private SimDouble simLinearWorldAccelX;
    private SimDouble simLinearWorldAccelY;
    private SimDouble simLinearWorldAccelZ;    

    float last_yaw;
    float last_linear_world_accel_x;
    float last_linear_world_accel_y;

    AHRSProtocol.BoardID board_id; 
    IIOCompleteNotification.BoardState board_state;
    AHRSPosUpdate ahrs_update;
    GyroUpdate raw_data_update;    
    
    public SimIO(byte update_rate_hz, IIOCompleteNotification notify_sink, SimDevice sim_device) {      
        this.notify_sink = notify_sink;
        this.update_rate_hz = update_rate_hz;
        this.sim_device = sim_device;
        if (sim_device != null) {
            
            // Booleans

            simConnected            = sim_device.createBoolean("Connected", false, true); 
            
            // Doubles
            
            simRate                 = sim_device.createDouble("Rate", false, 0.0f);
            simYaw                  = sim_device.createDouble("Yaw", false, 0.0f);
            simPitch                = sim_device.createDouble("Pitch", false, 0.0f);
            simRoll                 = sim_device.createDouble("Roll", false, 0.0f);     
            
            simCompassHeading       = sim_device.createDouble("CompassHeading", false, 0.0f);
            simFusedHeading         = sim_device.createDouble("FusedHeading", false, 0.0f);     

            simLinearWorldAccelX    = sim_device.createDouble("LinearWorldAccelX", false, 0.0f);                                
            simLinearWorldAccelY    = sim_device.createDouble("LinearWorldAccelY", false, 0.0f); 
            simLinearWorldAccelZ    = sim_device.createDouble("LinearWorldAccelZ", false, 0.0f);   
            
            System.out.println("navX-Sensor SimDevice created.");            
        }      

        /////////////////////////
        // BoardID initialization
        /////////////////////////

        board_id = new AHRSProtocol.BoardID();
        board_id.fw_ver_major = 3;
        board_id.fw_ver_minor = 1;
        board_id.fw_revision = 400;
        board_id.type = 33; // navx-MXP type id

        /////////////////////////
        // BoardState initialization
        /////////////////////////     

        board_state = new IIOCompleteNotification.BoardState();     
        board_state.selftest_status = 
            AHRSProtocol.NAVX_SELFTEST_STATUS_COMPLETE |
            AHRSProtocol.NAVX_SELFTEST_RESULT_GYRO_PASSED |
            AHRSProtocol.NAVX_SELFTEST_RESULT_ACCEL_PASSED |
            AHRSProtocol.NAVX_SELFTEST_RESULT_MAG_PASSED; // BARO Passed is NOT set 
        board_state.sensor_status =
            //AHRSProtocol.NAVX_SENSOR_STATUS_MOVING |              // NOTE:  Updated by Sim Variable
            AHRSProtocol.NAVX_SENSOR_STATUS_YAW_STABLE |
            //AHRSProtocol.NAVX_SENSOR_STATUS_MAG_DISTURBANCE |     // NOTE:  Always false
            //AHRSProtocol.NAVX_SENSOR_STATUS_ALTITUDE_VALID |      // NOTE:  Always false
            //AHRSProtocol.NAVX_SENSOR_STATUS_SEALEVEL_PRESS_SET |  // NOTE:  Always false
            AHRSProtocol.NAVX_SENSOR_STATUS_FUSED_HEADING_VALID;            
        board_state.op_status =
            AHRSProtocol.NAVX_OP_STATUS_NORMAL;
        board_state.cal_status = 
            IMUProtocol.NAV6_CALIBRATION_STATE_COMPLETE;
        board_state.capability_flags =
                // Note:  Configure capabilities to NOT include VEL_AND_DISP,
                // this forces the internal InertialDataIntegrator to be used.
                //AHRSProtocol.NAVX_CAPABILITY_FLAG_VEL_AND_DISP |
                AHRSProtocol.NAVX_CAPABILITY_FLAG_OMNIMOUNT |
                AHRSProtocol.NAVX_CAPABILITY_FLAG_AHRSPOS_TS |
                // Note:  Configure capabilities to NOT include HW-based YAW RESET;
                // This causes software-based yaw reset to be used
                //AHRSProtocol.NAVX_CAPABILITY_FLAG_YAW_RESET |                
                AHRSProtocol.OMNIMOUNT_DEFAULT;        
        board_state.accel_fsr_g = 2;
        board_state.gyro_fsr_dps = 2000;                
        board_state.update_rate_hz = this.update_rate_hz;    

        /////////////////////////        
        // AHRSUpdate initialization
        /////////////////////////

        ahrs_update = new AHRSPosUpdate();  
        
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
        ahrs_update.sensor_status = (byte)board_state.sensor_status;

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
        raw_data_update = new GyroUpdate();

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
    }
    
    private final int sim_bytes_per_second = 1000;
    
    public void stop() {
        stop = true;
    }
    
    public void run() {        
        /* IO Loop */
        // Simulate startup delay
        Timer.delay(0.05);

        // Default to connected state
        is_connected = true;
        notify_sink.connectDetected();

        long sensor_timestamp = 2000; // NOTE:  Simulate a 2-second navX-sensor firmware startup delay

        // Update all static values
        notify_sink.setBoardID(board_id);   
        notify_sink.setRawData(raw_data_update, sensor_timestamp);

        // Update AHRS data (portions of which are static; others are updated from sim variables)
        notify_sink.setAHRSPosData(ahrs_update, sensor_timestamp);        

        start_seconds = Timer.getFPGATimestamp();

        while (!stop) {
            Timer.delay(0.02f);
            sensor_timestamp += 20;
            UpdatePeriodicFromSimVariables(sensor_timestamp);            
        }
    }

    private float CalculateNormal(float in1, float in2) {
        return (float)Math.abs(Math.sqrt(Math.abs(in1*in1)+Math.abs(in2*in2)));
    }

    private void UpdatePeriodicFromSimVariables(long sensor_timestamp) {
        
        if (sim_device == null) return;

        boolean curr_is_connected =     simConnected.get();

        // Update connection state
        if (curr_is_connected != is_connected) {
            is_connected = curr_is_connected;
            if (curr_is_connected) {
                notify_sink.connectDetected();                
            } else {
                notify_sink.disconnectDetected();                
            }
        }          

        if (curr_is_connected) {
            ahrs_update.yaw =               (float)simYaw.get();
            ahrs_update.pitch =             (float)simPitch.get();
            ahrs_update.roll =              (float)simRoll.get();
            ahrs_update.compass_heading =   (float)simCompassHeading.get();
            ahrs_update.fused_heading =     (float)simFusedHeading.get();
            ahrs_update.linear_accel_x =    (float)simLinearWorldAccelX.get();
            ahrs_update.linear_accel_y =    (float)simLinearWorldAccelY.get();
            ahrs_update.linear_accel_z =    (float)simLinearWorldAccelZ.get();  
        
            // Detect motion
            float last_linear_accel_norm = CalculateNormal(last_linear_world_accel_x, last_linear_world_accel_y);
            float curr_linear_accel_norm = CalculateNormal(ahrs_update.linear_accel_x, ahrs_update.linear_accel_y);
            boolean curr_is_moving = (Math.abs(curr_linear_accel_norm - last_linear_accel_norm) >= 0.02);
            if (curr_is_moving) {
                board_state.sensor_status |= AHRSProtocol.NAVX_SENSOR_STATUS_MOVING;
            } else {
                board_state.sensor_status &= ~(AHRSProtocol.NAVX_SENSOR_STATUS_MOVING);
            }

            // Detect rotation
            boolean curr_is_rotating = Math.abs(last_yaw - ahrs_update.yaw) >= 0.05f;
            if (!curr_is_rotating) {
                board_state.sensor_status |= AHRSProtocol.NAVX_SENSOR_STATUS_YAW_STABLE;
            } else {
                board_state.sensor_status &= ~(AHRSProtocol.NAVX_SENSOR_STATUS_YAW_STABLE);
            }

            ahrs_update.sensor_status = (byte)board_state.sensor_status;

            // Trigger simulated update to notification sink
            notify_sink.setAHRSPosData(ahrs_update, sensor_timestamp);

            // Update cached state variables
            last_yaw = ahrs_update.yaw;
            last_linear_world_accel_x = ahrs_update.linear_accel_x;
            last_linear_world_accel_y = ahrs_update.linear_accel_y;            
        }             
    }
        
    public double getRate() {
        if (sim_device == null) return 0;
        
        return simRate.get();
    }

    @Override
    public boolean isConnected() {
        return is_connected;
    }

    @Override
    public double getByteCount() {
        double num_secs_running = Timer.getFPGATimestamp() - start_seconds;
        return num_secs_running * sim_bytes_per_second;
    }

    @Override
    public double getUpdateCount() {
        double num_secs_running = Timer.getFPGATimestamp() - start_seconds;
        return num_secs_running / update_rate_hz;
    }

    @Override
    public void setUpdateRateHz(byte update_rate_hz) {
        this.update_rate_hz = update_rate_hz;
    }
    
    @Override
    public void zeroYaw() {    
        this.notify_sink.yawResetComplete(); 
    }

    @Override
    public void zeroDisplacement() {     
    }
    
    @Override
    public void enableLogging(boolean enable) {
    }
}
