# Automatically abort script on errors
option batch abort
# Disable overwrite confirmations that conflict w/the previous
option confirm off
option batch on
# Connect to kauailabs ftp server using a password
open ftp://%FTP_USERNAME%:%FTP_PASSWORD%@ftp.kauailabs.com/
# Synchronize files
synchronize remote -delete ../roborio/java/navx_frc/build/docs/javadoc kauailabs.com/public_files/navx-mxp/apidocs/java
synchronize remote -delete ../android/navx_ftc/docs kauailabs.com/public_files/navx-micro/apidocs/android
synchronize remote -delete ../roborio/C++/doxygen/html kauailabs.com/public_files/navx-mxp/apidocs/c++
# Disconnect
close
# Quit
exit