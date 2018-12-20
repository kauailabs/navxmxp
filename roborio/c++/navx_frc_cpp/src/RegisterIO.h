/*
 * RegisterIO.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_REGISTERIO_H_
#define SRC_REGISTERIO_H_

#include <stdint.h>
#include "IIOProvider.h"
#include "IRegisterIO.h"
#include "IMUProtocol.h"
#include "AHRSProtocol.h"
#include "IBoardCapabilities.h"
#include "IIOCompleteNotification.h"
#include "frc/Timer.h"

class RegisterIO : public IIOProvider {
private:
    IRegisterIO *io_provider;
    uint8_t update_rate_hz;
    bool stop;
    IMUProtocol::GyroUpdate raw_data_update;
    AHRSProtocol::AHRSUpdate ahrs_update;
    AHRSProtocol::AHRSPosUpdate ahrspos_update;
    IIOCompleteNotification *notify_sink;
    IIOCompleteNotification::BoardState board_state;
    AHRSProtocol::BoardID board_id;
    IBoardCapabilities *board_capabilities;
    double last_update_time;
    int byte_count;
    int update_count;
    long last_sensor_timestamp;
public:
    RegisterIO( IRegisterIO *io_provider,
                uint8_t update_rate_hz,
                IIOCompleteNotification *notify_sink,
                IBoardCapabilities *board_capabilities  );
    bool   IsConnected();
    double GetByteCount();
    double GetUpdateCount();
    void   SetUpdateRateHz(uint8_t update_rate);
    void   ZeroYaw();
    void   ZeroDisplacement();
    void   Run();
    void   Stop();
    void   EnableLogging(bool enable);
    virtual ~RegisterIO();
private:
    bool   GetConfiguration();
    void   GetCurrentData();
};

#endif /* SRC_REGISTERIO_H_ */
