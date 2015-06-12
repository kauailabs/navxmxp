The navX MXP firmware incorporates the Invensense Motion Driver v. 6.1 (a software development including source code and accompanying documentation).

The Invensense Motion Driver license restricts the contents of the Invensense Motion Driver v. 6.1 (the source code and accompanying documentation).  As a result of this license, Kauai Labs is unable to re-distribute the Invensense Motion Driver.

*The Invensense Motion Driver v. 6.1 is NOT covered by the navX MXP Firmware License.*

Since the navX MXP is an open-source project (under the MIT license), this means that all source code and documentation contained within the Invensense Motion Driver SDK cannot be distributed along with the navX MXP firmware.

To address this issue, Kauai Labs has developed the following approach, which must be followed by any party desiring to compile the navX MXP firmware.

In general, to compile the navX MXP Firwmare, you must:

(a) Register with Invensense (at www.invensense.com) as a developer.
(b) Carefully read and agree to the Invensense license agreement.
(c) Download the Motion Driver v. 6.1.2 from the Invensense Developer support section of the Invensense website.
(d) Copy those portions of the Invensense Motion Driver v. 6.1.2 which are required into the directory containing the navX MXP Firmware source code.  See detailed instructions below.
(e) Make some trivial modifications to a few of the files within the Invensense Motion Driver v. 6.1.2 source code.  These modifications port the Motion Driver to the STM32 microcontroller platform used in the navX MXP.

** Detailed Instructions for integrating the Invensense Motion Driver v. 6.1.2 into the navX MXP source code. **

After downloading your licensed copy of the Invensense Motion Driver v. 6.1.2 on your development system:

1) Copy the needed portions of the Invensense Motion Driver v.6.1.2 to the navX MXP Firwmare Directory

  - the directory structure of the Invensense Motion Driver v.6.1.2 is as follows:

    \documentation
    \eMPL-pythonclient
    \mpl libraries
    \msp430
    \arm

  - copy the contents of these directories (including any sub-directory) into the "stm32\MPU9250" sub-directory of the navX MXP source code, as follows:

     \arm\STM32F4_MD6\Projects\eMD6\core\*.* --> stm32\MPU9250\core

  - To reiterate, all subdirectories and their contents should be copied in the above step.  Please pay careful attention when following these instruxtions.  You will need to use a text editor to perform these steps:

2) Modify three driver files to port the driver to the STM32 platform:

  a) core\driver\inv_mpu.h

  Starting after line 40, insert these three new lines of code:

#elif defined EMPL_TARGET_STM32
    unsigned short pin;
    void (*cb)();

  b) core\driver\inv_mpu.c

  Starting after line 103, insert these two new lines of code:

#elif defined EMPL_TARGET_STM32
#include "stm32_shim.h"

  Starting after line 2477, insert these new lines of code:

#ifdef MPU6500
    data[0] = BIT_FIFO_SIZE_1024 | st.test->reg_lpf;

    if (i2c_write(st.hw->addr, st.reg->accel_cfg2, 1, &data))
            return -1;
#endif

  And finally in the same function, replace this line:

    	delay_ms(test.sample_wait_ms); //wait 10ms to fill FIFO

  with this line:

    	delay_ms(test.sample_wait_ms-8); //wait 10ms to fill FIFO, less time to xfer i2c data from fifo


  c) core\driver\inv_mpu_dmp_motion_driver.c

  Starting after line 64, insert these two new lines of code:

#elif defined EMPL_TARGET_STM32
#include "stm32_shim.h"

Be sure to save these changes.

3) Delete the contents of the stm32\core\driver\stm32L directory

At this point, you should be able to sucessfully compile the navX MXP Firmware and resolve all references to the Invensense Motion Driver v6.1.2
