/* ============================================
VMX-pi HAL source code is placed under the MIT license
Copyright (c) 2017 Kauai Labs
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef SRC_AHRS_H_
#define SRC_AHRS_H_

#include <thread>
#include "ITimestampedDataSubscriber.h"

class IIOProvider;
class ContinuousAngleTracker;
class AHRSInternal;

class SPIClient;
class PIGPIOClient;

using namespace std;

class AHRS {
public:

    enum BoardAxis {
        kBoardAxisX = 0,
        kBoardAxisY = 1,
        kBoardAxisZ = 2,
    };

    struct BoardYawAxis
    {
        /* Identifies one of the board axes */
        BoardAxis board_axis;
        /* true if axis is pointing up (with respect to gravity); false if pointing down. */
        bool up;
    };

    enum SerialDataType {
    /**
     * (default):  6 and 9-axis processed data
     */
    kProcessedData = 0,
    /**
     * unprocessed data from each individual sensor
     */
    kRawData = 1
    };

private:
    friend class AHRSInternal;
    AHRSInternal *      ahrs_internal;

    volatile float      yaw;
    volatile float      pitch;
    volatile float      roll;
    volatile float      compass_heading;
    volatile float      world_linear_accel_x;
    volatile float      world_linear_accel_y;
    volatile float      world_linear_accel_z;
    volatile float      mpu_temp_c;
    volatile float      fused_heading;
    volatile float      altitude;
    volatile float      baro_pressure;
    volatile bool       is_moving;
    volatile bool       is_rotating;
    volatile float      baro_sensor_temp_c;
    volatile bool       altitude_valid;
    volatile bool       is_magnetometer_calibrated;
    volatile bool       magnetic_disturbance;
    volatile float    	quaternionW;
    volatile float    	quaternionX;
    volatile float    	quaternionY;
    volatile float    	quaternionZ;

    /* Integrated Data */
    float velocity[3];
    float displacement[3];


    /* Raw Data */
    volatile int16_t    raw_gyro_x;
    volatile int16_t    raw_gyro_y;
    volatile int16_t    raw_gyro_z;
    volatile int16_t    raw_accel_x;
    volatile int16_t    raw_accel_y;
    volatile int16_t    raw_accel_z;
    volatile int16_t    cal_mag_x;
    volatile int16_t    cal_mag_y;
    volatile int16_t    cal_mag_z;

    /* Configuration/Status */
    volatile uint8_t    update_rate_hz;
    volatile int16_t    accel_fsr_g;
    volatile int16_t    gyro_fsr_dps;
    volatile int16_t    capability_flags;
    volatile uint8_t    op_status;
    volatile int16_t    sensor_status;
    volatile uint8_t    cal_status;
    volatile uint8_t    selftest_status;

    /* Board ID */
    volatile uint8_t    board_type;
    volatile uint8_t    hw_rev;
    volatile uint8_t    fw_ver_major;
    volatile uint8_t    fw_ver_minor;

    long                last_sensor_timestamp;
    double              last_update_time;

    ContinuousAngleTracker *yaw_angle_tracker;
    IIOProvider *           io;

    thread *           task;

#define MAX_NUM_CALLBACKS 3
    ITimestampedDataSubscriber *callbacks[MAX_NUM_CALLBACKS];
    void *callback_contexts[MAX_NUM_CALLBACKS];

public:
    AHRS(SPIClient& client, PIGPIOClient& pigpio, uint8_t update_rate_hz);
    virtual ~AHRS();

    float  GetPitch();
    float  GetRoll();
    float  GetYaw();
    float  GetCompassHeading();
    void   ZeroYaw();
    bool   IsCalibrating();
    bool   IsConnected();
    double GetByteCount();
    double GetUpdateCount();
    long   GetLastSensorTimestamp();
    float  GetWorldLinearAccelX();
    float  GetWorldLinearAccelY();
    float  GetWorldLinearAccelZ();
    bool   IsMoving();
    bool   IsRotating();
    float  GetBarometricPressure();
    float  GetAltitude();
    bool   IsAltitudeValid();
    float  GetFusedHeading();
    bool   IsMagneticDisturbance();
    bool   IsMagnetometerCalibrated();
    float  GetQuaternionW();
    float  GetQuaternionX();
    float  GetQuaternionY();
    float  GetQuaternionZ();
    void   ResetDisplacement();
    void   UpdateDisplacement( float accel_x_g, float accel_y_g,
                               int update_rate_hz, bool is_moving );
    float  GetVelocityX();
    float  GetVelocityY();
    float  GetVelocityZ();
    float  GetDisplacementX();
    float  GetDisplacementY();
    float  GetDisplacementZ();
    double GetAngle();
    double GetRate();
    void   Reset();
    float  GetRawGyroX();
    float  GetRawGyroY();
    float  GetRawGyroZ();
    float  GetRawAccelX();
    float  GetRawAccelY();
    float  GetRawAccelZ();
    float  GetRawMagX();
    float  GetRawMagY();
    float  GetRawMagZ();
    float  GetPressure();
    float  GetTempC();
    AHRS::BoardYawAxis GetBoardYawAxis();
    std::string GetFirmwareVersion();

    bool RegisterCallback( ITimestampedDataSubscriber *callback, void *callback_context);
    bool DeregisterCallback( ITimestampedDataSubscriber *callback );

    int GetActualUpdateRate();
    int GetRequestedUpdateRate();

    void Stop();

private:
    void commonInit( uint8_t update_rate_hz );
    static int ThreadFunc(IIOProvider *io_provider);

    uint8_t GetActualUpdateRateInternal(uint8_t update_rate);
};

#endif /* SRC_AHRS_H_ */
