# Automatically abort script on errors
option batch abort
# Disable overwrite confirmations that conflict w/the previous
option confirm off
option batch on
# Connect to kauailabs ftp server using a password
open ftp://%FTP_USERNAME%:%FTP_PASSWORD%@ftp.kauailabs.com/
# Synchronize local maven repo files to KauaiLabs maven repo
cd kauailabs.com
mkdir maven2
cd maven2
mkdir com
cd com
mkdir kauailabs
synchronize remote -transfer=binary %HOMEDRIVE%%HOMEPATH%/.m2/repository/com/kauailabs kauailabs.com/maven2/com/kauailabs
# Update vendordeps file
cd kauailabs.com
mkdir dist
cd dist
mkdir frc
cd frc
# NOTE:  The "year" directory is updated to match the compatible yearly release
# of the WPI Library
mkdir 2022
cd ~/
put -transfer=binary ..\build\vendordeps\navx_frc.json kauailabs.com/dist/frc/2022/
# Disconnect
close
# Quit
exit