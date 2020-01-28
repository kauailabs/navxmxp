/*
 * SimIO.h
 *
 *  Created on: Jan 28, 2020
 *      Author: Scott
 */

#ifndef SRC_SIMIO_H_
#define SRC_SIMIO_H_

#include <stdint.h>
#include "IIOProvider.h"
#include "IMUProtocol.h"
#include "AHRSProtocol.h"
#include "IBoardCapabilities.h"
#include "IIOCompleteNotification.h"
#include "frc/Timer.h"

#include <hal/SimDevice.h>

class SimIO : public IIOProvider {
private:
    bool stop;
    bool is_connected;
    double start_seconds;  
    IIOCompleteNotification *notify_sink;         
    uint8_t update_rate_hz; 

    hal::SimDevice *sim_device;
    hal::SimBoolean simConnected;
    hal::SimDouble simRate;
    hal::SimDouble simYaw;
    hal::SimDouble simPitch;
    hal::SimDouble simRoll;
    hal::SimDouble simCompassHeading;
    hal::SimDouble simFusedHeading;    
    hal::SimDouble simLinearWorldAccelX;
    hal::SimDouble simLinearWorldAccelY;
    hal::SimDouble simLinearWorldAccelZ;

    float last_yaw;
    float last_linear_world_accel_x;
    float last_linear_world_accel_y;

    AHRSProtocol::BoardID board_id;
    IIOCompleteNotification::BoardState board_state;    
    AHRSProtocol::AHRSPosUpdate ahrs_update;    
    IMUProtocol::GyroUpdate raw_data_update;

public:
    SimIO( uint8_t update_rate_hz,
                IIOCompleteNotification *notify_sink,
                hal::SimDevice *sim_device);
    bool   IsConnected();
    double GetByteCount();
    double GetUpdateCount();
    void   SetUpdateRateHz(uint8_t update_rate);
    void   ZeroYaw();
    void   ZeroDisplacement();
    void   Run();
    void   Stop();
    void   EnableLogging(bool enable);
    virtual ~SimIO();

    double GetRate();
private:
    void UpdatePeriodicFromSimVariables(long sensor_timestamp);
    float CalculateNormal(float in1, float in2);
};

#endif /* SRC_SIMIO_H_ */
