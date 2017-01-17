<b>Release Notes</b> 
<p>
Version 3.0.316

Kauai Labs has been working with FIRST and National Instruments on an issue discovered when using USB Serial devices on the RoboRIO.

The issue occurs when writing to USB serial devices on the RoboRIO while simultaneously receiving data from the device.

[Updated FRC RoboRIO LabVIEW, C++ and Java Libraries](http://www.pdocs.kauailabs.com/navx-mxp/software/roborio-libraries/) containing a workaround for this issue are now available.

<b>What do I need to do?</b>

-	<i>C++, Java, LabVIEW</i>:  If your RoboRIO software communicates with navX-Micro or navX-MXP over USB, please update your libraries to version 3.0.316 or higher.
-	<i>LabVIEW</i>: the method for zeroing the yaw has changed slightly:  the Zero Yaw VI now requires a Boolean input which is set to TRUE to initiate Zeroing the yaw. 

![New LabVIEW Zero Yaw Method](http://www.pdocs.kauailabs.com/navx-mxp/wp-content/uploads/2017/01/LabviewNavX-AE_ZeroYawAPIChange.png)

With the USB Serial workaround, the libraries now enforce a limit - if communicating over USB – that the following actions cannot occur more frequently than 1/second:

-	Zeroing the Yaw
-	Changing the Update Rate

Kauai Labs believes these limitations will not impact teams, however if this limit when using USB communication is problematic, Kauai Labs recommends communicating instead using of the other communications interfaces.

