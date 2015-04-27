The navX MXP project contains the following components:

stm32:

- This directory contains the navX MXP Firmware source code and related Eclipse project files.

schematics:

- board schematics (developed in Eagle 6.5.0, freely available at http://www.cadsoftusa.com/)

crio:

- This directory contains code compatible with the 2014 version of the LabView, Java and C++ WPI Libraries, which runs on the FIRST robotics cRio platform.  This code allows cRio-hosted code to acquire yaw/pitch/roll values from the board in real-time.  Note that this code only implements the sub-set of navX MXP functionality which is compatible with the nav6.

roborio:

- This directory contains code compatible with the 2015 version of the LabView, Java and C++ WPI Libraries, which runs on the FIRST robotics RoboRio platform.  This code allows RoboRio-hosted code to acquire yaw/pitch/roll values from the board in real-time.

docs:

- This directory contains various and sundry documentation files for the navX MXP and the third party components used in the navX MXP.

enclosure:

- This directory contains design files for a 3d-printed enclosure for the navX MXP.

processing:

- This directory contains utility applications developed in the Processing 2.0 language - including the navXUI demonstration application.  These apps are cross-platform and can be compiled by downloading the open source Processing 2.0 development environment.
