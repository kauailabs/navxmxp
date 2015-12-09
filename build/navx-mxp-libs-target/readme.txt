The navx-mxp-libs.zip file contains all navx-mxp libraries, and is intended for use 
by non-windows navX-MXP developers.

*NOTE to Windows Developers:  All of the libs in this .zip file are automatically installed 
as part of the setup.exe installer included within the navX-MXP latest build.*

Installation instructions for Linux:

# Create a "navx-mxp" directory underneath your home directory

cd ~
mkdir navx-mxp
cd navx-mxp

# Unzip the contents of navx-mxp-libs.zip to the new navx-mxp directory.

At this point, the instructions for the C++, Java and Libaries documented at the online navX-MXP site 
for the RoboRIO libraries (http://navx-mxp.kauailabs.com/software/roborio-libraries/) and 
for the Android (FTC) libraries (http://navx-mxp.kauailabs.com/software/android-library-ftc/) may be
followed.  The only difference will be to replace the windows path to these libraries with the
Linux path the libraries.  Specifically, replace:

C:\Users\<username>\navx-mxp

  with

~\navx-mxp