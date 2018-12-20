/*
 * RegisterIOSPI.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIOSPI.h"
#include "wpi/priority_mutex.h"
#include "frc/Timer.h"

using namespace wpi;

static wpi::mutex imu_mutex;
RegisterIO_SPI::RegisterIO_SPI(SPI *port, uint32_t bitrate) {
    this->port = port;
    this->bitrate = bitrate;
    this->trace = false;
}

bool RegisterIO_SPI::Init() {
    port->SetClockRate(bitrate);
    port->SetMSBFirst();
    port->SetSampleDataOnTrailingEdge();
    port->SetClockActiveLow();
    port->SetChipSelectActiveLow();
    if (trace) printf("navX-MXP:  Initialized SPI communication at bitrate %d\n", bitrate);
    return true;
}

bool RegisterIO_SPI::Write(uint8_t address, uint8_t value ) {
	std::unique_lock<wpi::mutex> sync(imu_mutex);
    uint8_t cmd[3];
    cmd[0] = address | 0x80;
    cmd[1] = value;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( port->Write(cmd, sizeof(cmd)) != sizeof(cmd)) {
        if (trace) printf("navX-MXP SPI Write error\n");
        return false; // WRITE ERROR
    }
    return true;
}

bool RegisterIO_SPI::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
	std::unique_lock<wpi::mutex> sync(imu_mutex);
    uint8_t cmd[3];
    cmd[0] = first_address;
    cmd[1] = buffer_len;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( port->Write(cmd, sizeof(cmd)) != sizeof(cmd) ) {
        return false; // WRITE ERROR
    }
    // delay 200 us /* TODO:  What is min. granularity of delay()? */
    Wait(0.001);
    if ( port->Read(true, rx_buffer, buffer_len+1) != buffer_len+1 ) {
        if (trace) printf("navX-MXP SPI Read error\n");
        return false; // READ ERROR
    }
    uint8_t crc = IMURegisters::getCRC(rx_buffer, buffer_len);
    if ( crc != rx_buffer[buffer_len] ) {
        if (trace) printf("navX-MXP SPI CRC err.  Length:  %d, Got:  %d; Calculated:  %d\n", buffer_len, rx_buffer[buffer_len], crc);
        return false; // CRC ERROR
    } else {
        memcpy(buffer, rx_buffer, buffer_len);
    }
    return true;
}

bool RegisterIO_SPI::Shutdown() {
    return true;
}

void RegisterIO_SPI::EnableLogging(bool enable) {
	trace = enable;
}
