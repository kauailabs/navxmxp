/*
 * RegisterIOSPI.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#include "RegisterIOSPI.h"
#include "NavXSPIMessage.h"
#include <mutex>
#include <string.h>

RegisterIO_SPI::RegisterIO_SPI(SPIClient& client, PIGPIOClient& pigpio_ref) :
	client(client),
	pigpio(pigpio_ref)
{
    this->trace = true;
    this->first_read = true;
    pigpio.SetAHRSInterruptSink(this);
}

bool RegisterIO_SPI::Init() {
    return true;
}

bool RegisterIO_SPI::Write(uint8_t address, uint8_t value ) {
	NavXSPIMessage write_cmd(0, address, 1, &value);
    if (!client.write(write_cmd)) {
         return false; // WRITE ERROR
    }
    return true;
}

bool RegisterIO_SPI::Read(uint8_t first_address, uint8_t* buffer, uint8_t buffer_len) {
	NavXSPIMessage read_cmd(0 /* IMU REGISTER BANK */, first_address, buffer_len);
	uint8_t response_packet[buffer_len + 1]; /* Reserve one extra byte for CRC */
	if(!client.read(read_cmd, response_packet, buffer_len + 1)) {
        return false; // READ ERROR
    } else {
    	memcpy(buffer, response_packet, buffer_len);
    	if (first_read) {
    		/* Reading clears any pending AHRS interrupts.  Wait until
    		 * after first read is successful before enabling interrupts.
    		 */
    		first_read = false;
    	    pigpio.EnableAHRSInterrupt();
    	}
    }
    return true;
}

bool RegisterIO_SPI::Shutdown() {
	if (!first_read) {
		pigpio.DisableAHRSInterrupt();
	}
	pigpio.SetAHRSInterruptSink(0);
    return true;
}

void RegisterIO_SPI::AHRSInterrupt(uint64_t timestamp_us)
{
	cv.notify_all();
}

std::condition_variable& RegisterIO_SPI::GetNewDataConditionVariable()
{
	return cv;
}


