/*
 * RegisterIOSPI.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIOSPI.h"
#include <mutex>
#include <string.h>

RegisterIO_SPI::RegisterIO_SPI(SPIClient& client) :
	client(client)
{
    this->trace = true;
}

bool RegisterIO_SPI::Init() {
    return true;
}

bool RegisterIO_SPI::Write(uint8_t address, uint8_t value ) {
    uint8_t cmd[3];
    cmd[0] = address | 0x80;
    cmd[1] = value;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if (!client.transmit(cmd, sizeof(cmd))) {
         return false; // WRITE ERROR
    }
    return true;
}

bool RegisterIO_SPI::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
    uint8_t cmd[3];
    cmd[0] = first_address;
    cmd[1] = buffer_len;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if (!client.transmit_and_receive(cmd, sizeof(cmd), rx_buffer, buffer_len+1)) {
        return false; // READ ERROR
    }
    memcpy(buffer, rx_buffer, buffer_len);
    return true;
}

bool RegisterIO_SPI::Shutdown() {
    return true;
}
