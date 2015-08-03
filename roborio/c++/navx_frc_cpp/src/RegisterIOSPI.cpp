/*
 * RegisterIOSPI.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include <RegisterIOSPI.h>

static ReentrantSemaphore cIMUStateSemaphore;
RegisterIO_SPI::RegisterIO_SPI(SPI *port, uint32_t bitrate) {
    this->port = port;
    this->bitrate = bitrate;
}

bool RegisterIO_SPI::Init() {
    port->SetClockRate(bitrate);
    port->SetMSBFirst();
    port->SetSampleDataOnFalling();
    port->SetClockActiveLow();
    port->SetChipSelectActiveLow();
    return true;
}

bool RegisterIO_SPI::Write(uint8_t address, uint8_t value ) {
    Synchronized sync(cIMUStateSemaphore);
    uint8_t cmd[3];
    cmd[0] = address | 0x80;
    cmd[1] = value;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( port->Write(cmd, sizeof(cmd)) != sizeof(cmd)) {
        printf("SPI Write fail\n");
        return false; // WRITE ERROR
    }
    return true;
}

bool RegisterIO_SPI::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
    Synchronized sync(cIMUStateSemaphore);
    uint8_t cmd[3];
    cmd[0] = first_address;
    cmd[1] = buffer_len;
    cmd[2] = IMURegisters::getCRC(cmd, 2);
    if ( port->Write(cmd, sizeof(cmd)) != sizeof(cmd) ) {
        printf("SPI Write fail\n");
        return false; // WRITE ERROR
    }
    // delay 200 us /* TODO:  What is min. granularity of delay()? */
    Wait(0.001);
    if ( port->Read(true, rx_buffer, buffer_len+1) != buffer_len+1 ) {
        printf("SPI Read fail\n");
        return false; // READ ERROR
    }
    uint8_t crc = IMURegisters::getCRC(rx_buffer, buffer_len);
    if ( crc != rx_buffer[buffer_len] ) {
        printf("SPI CRC err.  Length:  %d, Got:  %d; Calculated:  %d\n", buffer_len, rx_buffer[buffer_len], crc);
        return false; // CRC ERROR
    } else {
        memcpy(buffer, rx_buffer, buffer_len);
    }
    return true;
}

bool RegisterIO_SPI::Shutdown() {
    return true;
}
