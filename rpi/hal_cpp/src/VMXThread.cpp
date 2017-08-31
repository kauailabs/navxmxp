/*
 * VMXThread.cpp
 *
 *  Created on: 24 Jul 2017
 *      Author: pi
 */

#include "VMXThread.h"
#include "PIGPIOClient.h"

VMXThread::VMXThread(PIGPIOClient& pigpio_ref) :
	pigpio(pigpio_ref)
{
}

void VMXThread::ReleaseResources()
{
}

VMXThread::~VMXThread()
{
	ReleaseResources();
}


