/*
 * RegisterIOSPI.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIOSPI.h"
#include <mutex>
#include <string.h>

static std::mutex imu_mutex;
RegisterIO_SPI::RegisterIO_SPI(DaGamaClient *client) {
    this->client = client;
    this->trace = true;
}

bool RegisterIO_SPI::Init() {
    return true;
}

bool RegisterIO_SPI::Write(uint8_t address, uint8_t value ) {
	std::unique_lock<std::mutex> sync(imu_mutex);
    uint8_t cmd[3];
    cmd[0] = address | 0x80;
    cmd[1] = value;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( client->transmit(cmd, sizeof(cmd)) != sizeof(cmd)) {
         return false; // WRITE ERROR
    }
    return true;
}

bool RegisterIO_SPI::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
	std::unique_lock<std::mutex> sync(imu_mutex);
    uint8_t cmd[3];
    cmd[0] = first_address;
    cmd[1] = buffer_len;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( client->transmit(cmd, sizeof(cmd)) != sizeof(cmd) ) {
        return false; // WRITE ERROR
    }
    if ( client->receive(rx_buffer, buffer_len+1) != buffer_len+1 ) {
        return false; // READ ERROR
    }
    memcpy(buffer, rx_buffer, buffer_len);
    return true;
}

bool RegisterIO_SPI::Shutdown() {
    return true;
}
