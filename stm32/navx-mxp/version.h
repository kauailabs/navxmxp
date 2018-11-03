#define NAVX_MXP_FIRMWARE_VERSION_MAJOR 	3
#define NAVX_MXP_FIRMWARE_VERSION_MINOR		1

#define NAVX_MXP_HARDWARE_VERSION_NUMBER    33 /* Revision 3.3 MXP IO */

// Version Log
//
// 0.1:  Initial Development Version (11/28/2014)
//
// 0.2:  AHRS Feature Development (12/15/2014)
//
// 1.0:  First Release Version (12/28/2014)
//
// 1.1:  Second Release Version (6/12/2015)
//
//        - Adds OmniMount Feature
//        - Adds on-board Velocity/Displacement Integration
//        - Improved responsiveness of I2C and SPI Interfaces
//        - Fixed an issue which could cause the UART received packet handling to hang
//        - Improved performance of UART Transmit interface (buffered writes)
// 2.0:  Third Release Version (8/15/2015)
//        - Added recovery from internal I2C Bus Hangs when communicating w/MPU-9250
//        - Added recovery from External I2C Bus stuck condition
// 2.1:  Fourth Release Version (10/11/2015)
//        - Added support for I2C Master devices which use repeated starts and don't
//          send a "STOP" signal between reads.  This approach is used by the 
//          Core Device Interface Module from Modern Robotics, Inc.  With this feature,
//          the navX MXP is now compatible with the Android-based FTC Robotics 
// 2.2:  Fifth Release Version (10/16/2015)
//       -  Added a "shadow" of the calibration state into the previously unused portion
//          of the sensor status register.  This replication allows Android-based FTC
//          Robotics systems to acquire all processed data, sensor status and timestamp
//          in one single 26-byte I2C bus transaction.
//          controller system released in Summer/Fall, 2015.
// 2.3: Sixth Release Version (12/21/2015)
//       -  Maintenance release.  Fixed a few communications issues encountered during
//          integration w/the new, v2 LabView Library.
// 3.0: Seventh Release Version (5/19/2016)
//       -  New Feature Release:
//          - Updated firmware, C++/Java libraries to support 200Hz update rate
//          - Added new "AHRSPosTimestamp" message, adding timestamp to serial packets
//          - Updated navXUI to support 200Hz operation and new timestamp message
//          - Updated navXUI to support save-to-file operation
// 3.1: Eighth Release Vesrion (11/3/2018)
//       - No changes to firmware
//       - Modified FIRST FRC Libraries to not use the firmware-based yaw reset, due to timing issues
//       - Updated build process to use gradle and support VS Code as required by WPI