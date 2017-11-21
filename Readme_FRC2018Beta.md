Current status of the navX-MXP/Micro libraries for FRC with respect to the FRC 2018 Beta

C++ and Java are known working for all interfaces.  In Beta 3, the Java USB interface is now working.

Here are the issues Kauai Labs has identified when testing navX-MXP with the FRC 2018 Beta 3 libraries:

- There is some less-than-reliable behavior on the SPI interface.  What has always been a reliable interface has become unstable.  We've notified FIRST/WPI of the issue.

- If the "reboot robot code" button on the Driver station is pushed, sometimes (approximately 1 in 7 tries), after the robot code is restarted the Roborio will stop communicating with the Driver station

- If the connection to the Driver Station is lost (for example, if the RoboRIO is rebooted), often when the connection to the Driver Station is re-established the Smart Dashboard will no longer be updated.  This issue is slated to be fixed in Beta 4.

