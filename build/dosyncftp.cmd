# Automatically abort script on errors
option batch abort
# Disable overwrite confirmations that conflict w/the previous
option confirm off
option batch on
# Connect to kauailabs ftp server using a password
open ftp://%FTP_USERNAME%:%FTP_PASSWORD%@ftp.kauailabs.com/
# Synchronize files
put -neweronly -transfer=binary ..\navx-mxp.zip kauailabs.com/public_files/navx-mxp/
put -neweronly -transfer=binary ..\navx-mxp-libs.zip kauailabs.com/public_files/navx-mxp/
cd kauailabs.com/public_files/navx-mxp
mkdir installers
synchronize remote -transfer=binary ../installers installers/
mkdir drivers
synchronize remote -transfer=binary ../drivers drivers/
# Disconnect
close
# Quit
exit