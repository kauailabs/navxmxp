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

int main(int argc, char *argv[])
{
	bool realtime = false;
	uint8_t update_rate_hz = 50;
	VMXPi vmx(realtime, update_rate_hz);
	try {
		if(vmx.IsOpen()) {

			for ( int i = 0; i < 100; i++) {
				/* Display System (Battery) Voltage */
				float system_voltage = -1.0f;
				if(vmx.power.GetSystemVoltage(system_voltage)){
					printf("System (Battery) Voltage:  %f", system_voltage);
				} else {
					printf("Error retrieving system voltage.");
				}
				bool overcurrent;
				if (vmx.power.GetOvercurrent(overcurrent)) {
					printf("- %s\n", (overcurrent ? "OVERCURRENT" : ""));
				} else {
					printf("\n");
				}
				vmx.time.DelayMilliseconds(10);
			}
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


