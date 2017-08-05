/*
 * VMXVersion.cpp
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#include "VMXVersion.h"

VMXVersion::VMXVersion(AHRS& ahrs_ref) :
	ahrs(ahrs_ref)
{
}

void VMXVersion::ReleaseResources()
{
}

VMXVersion::~VMXVersion() {
	ReleaseResources();
}

std::string VMXVersion::GetFirmwareVersion()
{
	return ahrs.GetFirmwareVersion();
}

