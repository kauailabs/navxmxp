/*
 * RegisterIO.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIO.h"
#include "IMURegisters.h"
#include "delay.h"

using namespace frc;

RegisterIO::RegisterIO( IRegisterIO *io_provider,
        uint8_t update_rate_hz,
        IIOCompleteNotification *notify_sink,
        IBoardCapabilities *board_capabilities  ) {
    this->io_provider           = io_provider;
    this->update_rate_hz        = update_rate_hz;
    this->board_capabilities    = board_capabilities;
    this->notify_sink           = notify_sink;
    this->last_sensor_timestamp = 0;
    this->update_count 			= 0;
    this->byte_count			= 0;
    this->last_update_time      = 0;
    this->stop                  = false;

    raw_data_update = {};
    ahrs_update     = {};
    ahrspos_update  = {};
    board_state     = {};
    board_id        = {};
}

static const double IO_TIMEOUT_SECONDS = 1.0;
static const double DELAY_OVERHEAD_MILLISECONDS = 4.0;

RegisterIO::~RegisterIO() {
}

bool RegisterIO::IsConnected() {
    double time_since_last_update = Timer::GetFPGATimestamp() - this->last_update_time;
    return time_since_last_update <= IO_TIMEOUT_SECONDS;
}

double RegisterIO::GetByteCount() {
    return byte_count;
}

double RegisterIO::GetUpdateCount() {
    return update_count;
}

void RegisterIO::SetUpdateRateHz(uint8_t update_rate) {
    io_provider->Write( NAVX_REG_UPDATE_RATE_HZ, update_rate);
}

void RegisterIO::ZeroYaw() {
    io_provider->Write( NAVX_REG_INTEGRATION_CTL,
                        NAVX_INTEGRATION_CTL_RESET_YAW );
    notify_sink->YawResetComplete();
}

void RegisterIO::ZeroDisplacement() {
    io_provider->Write( NAVX_REG_INTEGRATION_CTL,
                        NAVX_INTEGRATION_CTL_RESET_DISP_X |
                        NAVX_INTEGRATION_CTL_RESET_DISP_Y |
                        NAVX_INTEGRATION_CTL_RESET_DISP_Z );
}

void RegisterIO::Run() {
    io_provider->Init();

    /* Initial Device Configuration */
    SetUpdateRateHz(this->update_rate_hz);
    GetConfiguration();

    double update_rate_ms = 1.0/(double)this->update_rate_hz;
    if ( update_rate_ms > DELAY_OVERHEAD_MILLISECONDS) {
    	update_rate_ms -= DELAY_OVERHEAD_MILLISECONDS;
    }

    /* IO Loop */
    while (!stop) {
        if ( board_state.update_rate_hz != this->update_rate_hz ) {
            SetUpdateRateHz(this->update_rate_hz);
        }
        GetCurrentData();
        delayMillis(update_rate_ms);
    }
}

void RegisterIO::Stop() {
    stop = true;
}

void RegisterIO::EnableLogging(bool enable) {
	io_provider->EnableLogging(enable);
}

bool RegisterIO::GetConfiguration() {
    bool success = false;
    int retry_count = 0;
    while ( retry_count < 3 && !success ) {
        char config[NAVX_REG_SENSOR_STATUS_H+1] = {0};
        if ( io_provider->Read(NAVX_REG_WHOAMI, (uint8_t *)config, sizeof(config)) ) {
            board_id.hw_rev                 = config[NAVX_REG_HW_REV];
            board_id.fw_ver_major           = config[NAVX_REG_FW_VER_MAJOR];
            board_id.fw_ver_minor           = config[NAVX_REG_FW_VER_MINOR];
            board_id.type                   = config[NAVX_REG_WHOAMI];
            notify_sink->SetBoardID(board_id);

            board_state.cal_status          = config[NAVX_REG_CAL_STATUS];
            board_state.op_status           = config[NAVX_REG_OP_STATUS];
            board_state.selftest_status     = config[NAVX_REG_SELFTEST_STATUS];
            board_state.sensor_status       = IMURegisters::decodeProtocolUint16(config + NAVX_REG_SENSOR_STATUS_L);
            board_state.gyro_fsr_dps        = IMURegisters::decodeProtocolUint16(config + NAVX_REG_GYRO_FSR_DPS_L);
            board_state.accel_fsr_g         = (int16_t)config[NAVX_REG_ACCEL_FSR_G];
            board_state.update_rate_hz      = config[NAVX_REG_UPDATE_RATE_HZ];
            board_state.capability_flags    = IMURegisters::decodeProtocolUint16(config + NAVX_REG_CAPABILITY_FLAGS_L);
            notify_sink->SetBoardState(board_state);
            success = true;
        } else {
            success = false;
            delayMillis(50);
        }
        retry_count++;
    }
    return success;
}

void RegisterIO::GetCurrentData() {
    uint8_t first_address = NAVX_REG_UPDATE_RATE_HZ;
    bool displacement_registers = board_capabilities->IsDisplacementSupported();
    uint8_t buffer_len;
    char curr_data[NAVX_REG_LAST + 1];
    /* If firmware supports displacement data, acquire it - otherwise implement */
    /* similar (but potentially less accurate) calculations on this processor.  */
    if ( displacement_registers ) {
        buffer_len = NAVX_REG_LAST + 1 - first_address;
    } else {
        buffer_len = NAVX_REG_QUAT_OFFSET_Z_H + 1 - first_address;
    }
    if ( io_provider->Read(first_address,(uint8_t *)curr_data, buffer_len) ) {
    	long sensor_timestamp = IMURegisters::decodeProtocolUint32(curr_data + NAVX_REG_TIMESTAMP_L_L-first_address);
        if ( sensor_timestamp == last_sensor_timestamp ) {
        	return;
        }
        last_sensor_timestamp = sensor_timestamp;
        ahrspos_update.op_status       = curr_data[NAVX_REG_OP_STATUS - first_address];
        ahrspos_update.selftest_status = curr_data[NAVX_REG_SELFTEST_STATUS - first_address];
        ahrspos_update.cal_status      = curr_data[NAVX_REG_CAL_STATUS];
        ahrspos_update.sensor_status   = curr_data[NAVX_REG_SENSOR_STATUS_L - first_address];
        ahrspos_update.yaw             = IMURegisters::decodeProtocolSignedHundredthsFloat(curr_data + NAVX_REG_YAW_L-first_address);
        ahrspos_update.pitch           = IMURegisters::decodeProtocolSignedHundredthsFloat(curr_data + NAVX_REG_PITCH_L-first_address);
        ahrspos_update.roll            = IMURegisters::decodeProtocolSignedHundredthsFloat(curr_data + NAVX_REG_ROLL_L-first_address);
        ahrspos_update.compass_heading = IMURegisters::decodeProtocolUnsignedHundredthsFloat(curr_data + NAVX_REG_HEADING_L-first_address);
        ahrspos_update.mpu_temp        = IMURegisters::decodeProtocolSignedHundredthsFloat(curr_data + NAVX_REG_MPU_TEMP_C_L - first_address);
        ahrspos_update.linear_accel_x  = IMURegisters::decodeProtocolSignedThousandthsFloat(curr_data + NAVX_REG_LINEAR_ACC_X_L-first_address);
        ahrspos_update.linear_accel_y  = IMURegisters::decodeProtocolSignedThousandthsFloat(curr_data + NAVX_REG_LINEAR_ACC_Y_L-first_address);
        ahrspos_update.linear_accel_z  = IMURegisters::decodeProtocolSignedThousandthsFloat(curr_data + NAVX_REG_LINEAR_ACC_Z_L-first_address);
        ahrspos_update.altitude        = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_ALTITUDE_D_L - first_address);
        ahrspos_update.barometric_pressure = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_PRESSURE_DL - first_address);
        ahrspos_update.fused_heading   = IMURegisters::decodeProtocolUnsignedHundredthsFloat(curr_data + NAVX_REG_FUSED_HEADING_L-first_address);
        ahrspos_update.quat_w          = ((float)IMURegisters::decodeProtocolInt16(curr_data + NAVX_REG_QUAT_W_L-first_address)) / 32768.0f;
        ahrspos_update.quat_x          = ((float)IMURegisters::decodeProtocolInt16(curr_data + NAVX_REG_QUAT_X_L-first_address)) / 32768.0f;
        ahrspos_update.quat_y          = ((float)IMURegisters::decodeProtocolInt16(curr_data + NAVX_REG_QUAT_Y_L-first_address)) / 32768.0f;
        ahrspos_update.quat_z          = ((float)IMURegisters::decodeProtocolInt16(curr_data + NAVX_REG_QUAT_Z_L-first_address)) / 32768.0f;
        if ( displacement_registers ) {
            ahrspos_update.vel_x       = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_VEL_X_I_L-first_address);
            ahrspos_update.vel_y       = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_VEL_Y_I_L-first_address);
            ahrspos_update.vel_z       = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_VEL_Z_I_L-first_address);
            ahrspos_update.disp_x      = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_DISP_X_I_L-first_address);
            ahrspos_update.disp_y      = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_DISP_Y_I_L-first_address);
            ahrspos_update.disp_z      = IMURegisters::decodeProtocol1616Float(curr_data + NAVX_REG_DISP_Z_I_L-first_address);
            notify_sink->SetAHRSPosData(ahrspos_update, sensor_timestamp);
        } else {
            ahrs_update.op_status           = ahrspos_update.op_status;
            ahrs_update.selftest_status     = ahrspos_update.selftest_status;
            ahrs_update.cal_status          = ahrspos_update.cal_status;
            ahrs_update.sensor_status       = ahrspos_update.sensor_status;
            ahrs_update.yaw                 = ahrspos_update.yaw;
            ahrs_update.pitch               = ahrspos_update.pitch;
            ahrs_update.roll                = ahrspos_update.roll;
            ahrs_update.compass_heading     = ahrspos_update.compass_heading;
            ahrs_update.mpu_temp            = ahrspos_update.mpu_temp;
            ahrs_update.linear_accel_x      = ahrspos_update.linear_accel_x;
            ahrs_update.linear_accel_y      = ahrspos_update.linear_accel_y;
            ahrs_update.linear_accel_z      = ahrspos_update.linear_accel_z;
            ahrs_update.altitude            = ahrspos_update.altitude;
            ahrs_update.barometric_pressure = ahrspos_update.barometric_pressure;
            ahrs_update.fused_heading       = ahrspos_update.fused_heading;
            notify_sink->SetAHRSData( ahrs_update, sensor_timestamp );
        }

        board_state.cal_status      = curr_data[NAVX_REG_CAL_STATUS-first_address];
        board_state.op_status       = curr_data[NAVX_REG_OP_STATUS-first_address];
        board_state.selftest_status = curr_data[NAVX_REG_SELFTEST_STATUS-first_address];
        board_state.sensor_status   = IMURegisters::decodeProtocolUint16(curr_data + NAVX_REG_SENSOR_STATUS_L-first_address);
        board_state.update_rate_hz  = curr_data[NAVX_REG_UPDATE_RATE_HZ-first_address];
        board_state.gyro_fsr_dps    = IMURegisters::decodeProtocolUint16(curr_data + NAVX_REG_GYRO_FSR_DPS_L-first_address);
        board_state.accel_fsr_g     = (int16_t)curr_data[NAVX_REG_ACCEL_FSR_G-first_address];
        board_state.capability_flags= IMURegisters::decodeProtocolUint16(curr_data + NAVX_REG_CAPABILITY_FLAGS_L-first_address);
        notify_sink->SetBoardState(board_state);

        raw_data_update.gyro_x      = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_GYRO_X_L-first_address);
        raw_data_update.gyro_y      = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_GYRO_Y_L-first_address);
        raw_data_update.gyro_z      = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_GYRO_Z_L-first_address);
        raw_data_update.accel_x     = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_ACC_X_L-first_address);
        raw_data_update.accel_y     = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_ACC_Y_L-first_address);
        raw_data_update.accel_z     = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_ACC_Z_L-first_address);
        raw_data_update.mag_x       = IMURegisters::decodeProtocolInt16(curr_data +  NAVX_REG_MAG_X_L-first_address);
        raw_data_update.temp_c      = ahrspos_update.mpu_temp;
        notify_sink->SetRawData(raw_data_update, sensor_timestamp);

        this->last_update_time = Timer::GetFPGATimestamp();
        byte_count += buffer_len;
        update_count++;
    }
}


