#/bin/bash
rm -r -f ./vmxpi-hal/usr/local/include/vmxpi_hal_cpp/*
rm -r -f ./vmxpi-hal/usr/local/lib/vmxpi_hal_cpp/*
rm -r -f ./vmxpi-hal/usr/local/lib/vmxpi_hal_cpp/*
cp ./hal_cpp/include/* ./vmxpi-hal/usr/local/include/vmxpi/
cp ./hal_cpp/Debug/*.so ./vmxpi-hal/usr/local/lib/vmxpi/
dpkg-deb --build vmxpi-hal
ls -l *.deb > override
dpkg-scanpackages ./ override | gzip > Packages.gz
# Transfer .deb file and Packages.gz to server
ftp -n -v ftp.kauailabs.com << EOT
user $1 $2
prompt
cd kauailabs.com/packages
bin
del Packages.gz
put vmxpi-hal.deb
put Packages.gz
bye
EOT
