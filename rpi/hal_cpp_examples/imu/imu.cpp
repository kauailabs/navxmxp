/*
 * main.cpp
 *
 *  Created on: 13 Aug 2016
 *      Author: pi
 */

#include <stdio.h>  /* printf() */
#include <string.h> /* memcpy() */
#include <inttypes.h>

#include "VMXPi.h"

void DisplayVMXError(VMXErrorCode vmxerr) {
	const char *p_err_description = GetVMXErrorString(vmxerr);
	printf("VMXError %d:  %s\n", vmxerr, p_err_description);
}

class AHRSCallback : public ITimestampedDataSubscriber
{
	AHRS& ahrs;
public:
	AHRSCallback(AHRS& ahrs_ref) :
		ahrs(ahrs_ref) {}
	virtual ~AHRSCallback() {}
    virtual void timestampedDataReceived( long system_timestamp, long sensor_timestamp, AHRSProtocol::AHRSUpdateBase& sensor_data, void * context )
    {
    	printf("AHRS Callback Data Received.  SysTimestamp:  %ld, SensorTimestamp:  %ld\n",
    			system_timestamp,
    			sensor_timestamp);
		printf("Yaw, Pitch, Roll:  %f, %f, %f\n", ahrs.GetYaw(), ahrs.GetPitch(), ahrs.GetRoll());
    }
};


int main(int argc, char *argv[])
{
	bool realtime = false;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	AHRSCallback ahrs_callback(vmx.ahrs);
	try {
		if(vmx.IsOpen()) {
			printf("AHRS Connected:  %s\n", (vmx.ahrs.IsConnected() ? "Yes" : "No"));

			/* Usage Model 1:  Foreground polling (of latest data in receive cache) */
			for ( int i = 0; i < 10; i++) {
				printf("Yaw, Pitch, Roll:  %f, %f, %f\n", vmx.ahrs.GetYaw(), vmx.ahrs.GetPitch(), vmx.ahrs.GetRoll());
				vmx.time.DelayMilliseconds(20);
			}

			/* Usage Model 2:  Interrupt-driven callbacks whenever new data arrives. */
			vmx.ahrs.RegisterCallback(&ahrs_callback, NULL);

			vmx.time.DelaySeconds(10);
		} else {
			printf("Error:  Unable to open VMX Client.\n");
			printf("\n");
			printf("        - Is pigpio functional/running?\n");
			printf("        - Does this application have root privileges?\n");
		}
	}
	catch(const std::exception& ex){
		printf("Caught exception:  %s", ex.what());
	}
}


