/*
 * RegisterIOI2C.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIOI2C.h"
#include "wpi/priority_mutex.h"
#include "frc/Timer.h"

using namespace wpi;

static wpi::mutex imu_mutex;
RegisterIO_I2C::RegisterIO_I2C(I2C* port) {
    this->port = port;
    this->trace = false;
}

bool RegisterIO_I2C::Init() {
    return true;
}

bool RegisterIO_I2C::Write(uint8_t address, uint8_t value ) {
	std::unique_lock<wpi::mutex> sync(imu_mutex);
    bool aborted = port->Write(address | 0x80, value);
    if (aborted && trace) printf("navX-MXP I2C Write error\n");
    return !aborted;
}

static const int MAX_WPILIB_I2C_READ_BYTES = 127;

bool RegisterIO_I2C::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
	std::unique_lock<wpi::mutex> sync(imu_mutex);
    int len = buffer_len;
    int buffer_offset = 0;
    uint8_t read_buffer[MAX_WPILIB_I2C_READ_BYTES];
    while ( len > 0 ) {
        int read_len = (len > MAX_WPILIB_I2C_READ_BYTES) ? MAX_WPILIB_I2C_READ_BYTES : len;
        if (!port->Write(first_address + buffer_offset, read_len) &&
            !port->ReadOnly(read_len, read_buffer) ) {
            memcpy(buffer + buffer_offset, read_buffer, read_len);
            buffer_offset += read_len;
            len -= read_len;
        } else {
        	if (trace) printf("navX-MXP I2C Read error\n");
            break;
        }
    }
    return (len == 0);
}

bool RegisterIO_I2C::Shutdown() {
    return true;
}

void RegisterIO_I2C::EnableLogging(bool enable) {
	trace = enable;
}

