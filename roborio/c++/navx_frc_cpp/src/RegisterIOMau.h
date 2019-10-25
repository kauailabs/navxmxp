/*
 * RegisterIO.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_REGISTERIOMAU_H_
#define SRC_REGISTERIOMAU_H_

#include <stdint.h>
#include "IIOProvider.h"
#include "IRegisterIO.h"
#include "IMUProtocol.h"
#include "AHRSProtocol.h"
#include "IBoardCapabilities.h"
#include "IIOCompleteNotification.h"

class RegisterIOMau : public IIOProvider {
private:
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
    long last_sensor_timestamp;

    void (*pfunc_Init)(uint8_t update_rate_hz);
    bool (*pfunc_IsConnected)();
    double (*pfunc_GetByteCount)();
    double (*pfunc_GetUpdateCount)();
    void (*pfunc_ZeroYaw)();
    void (*pfunc_ResetDisplacement)();
    void (*pfunc_ReadConfiguration_Data)();
    bool (*pfunc_BlockOnNewCurrentRegisterData)(uint32_t, uint8_t *, uint8_t *, uint8_t, uint8_t *);
    bool (*pfunc_ReadConfigurationData)(uint8_t, uint8_t *, uint8_t);

    void *dlhandle;

public:
    RegisterIOMau( uint8_t update_rate_hz,
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
    virtual ~RegisterIOMau();
private:
    bool GetConfiguration();
    bool GetCurrentData();
};

#endif /* SRC_REGISTERIOMAU_H_ */
