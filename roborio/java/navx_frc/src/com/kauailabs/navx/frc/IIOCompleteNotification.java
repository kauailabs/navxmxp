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

import com.kauailabs.navx.AHRSProtocol;
import com.kauailabs.navx.IMUProtocol;

interface IIOCompleteNotification {
    class BoardState {
        public byte  op_status;
        public short sensor_status;
        public byte  cal_status;
        public byte  selftest_status;
        public short capability_flags;
        public byte  update_rate_hz;
        public short accel_fsr_g;
        public short gyro_fsr_dps;
    }
    void setYawPitchRoll(IMUProtocol.YPRUpdate yprupdate);
    void setAHRSData(AHRSProtocol.AHRSUpdate ahrs_update);
    void setAHRSPosData(AHRSProtocol.AHRSPosUpdate ahrs_update);
    void setRawData(IMUProtocol.GyroUpdate raw_data_update);
    void setBoardID(AHRSProtocol.BoardID board_id);
    void setBoardState( BoardState board_state);
}
