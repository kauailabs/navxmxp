/*
 * VMXVersion.cpp
 *
 *  Created on: 29 Jul 2017
 *      Author: pi
 */

#include <sstream>
#include <string>
#include "VMXVersion.h"
#include "version.h"

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

std::string VMXVersion::GetHALVersion()
{
    std::ostringstream os;
    os << (int)VMXPI_HAL_VERSION_MAJOR << "." << (int)VMXPI_HAL_VERSION_MINOR << "." << (int)VMXPI_HAL_VERSION_BUILD;
    std::string hal_version = os.str();
    return hal_version;
}
