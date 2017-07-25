/*
 * SystemClocks.cpp
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#include "SystemClocks.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

SystemClocks::SystemClocks(PIGPIOClient& pigpio_ref) :
	pigpio(pigpio_ref)
{
}

bool SystemClocks::Init()
{
	return true;
}

SystemClocks::~SystemClocks() {
}

uint64_t SystemClocks::GetCurrentSystemTimeInMicroseconds()
{
	  struct timespec now;
	  clock_gettime( CLOCK_MONOTONIC_RAW, &now );
	  return (uint64_t)now.tv_sec * 1000000U + (uint64_t)(now.tv_nsec/1000);
}

uint64_t SystemClocks::GetCurrentSystemTimeInMicrosecondsAlt()
{
    struct timeval start;
    gettimeofday(&start, NULL);

    return (uint64_t)start.tv_sec * 1000000U + (uint64_t)start.tv_usec;
}

uint32_t SystemClocks::GetCurrentMicroseconds()
{
	return pigpio.GetCurrentMicrosecondTicks();
}

uint64_t SystemClocks::GetCurrentTotalMicroseconds()
{
	return pigpio.GetTotalCurrentMicrosecondTicks();
}
