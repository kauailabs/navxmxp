#define NAVX_MXP_FIRMWARE_VERSION_MAJOR 	2
#define NAVX_MXP_FIRMWARE_VERSION_MINOR		0

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
