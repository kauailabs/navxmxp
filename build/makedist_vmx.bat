set ZIP_UTILITY="C:\Program Files\7-Zip\7z"
REM Batch file to create the navX MXP distributable package

pushd ..
md dist
pushd dist

cp ../setup/Output_vmx-pi/* .
cp ../setup/target/vmx-pi/readme.txt ./
cp ../setup/target/vmx-pi/getting_started.txt ./
cp ../LICENSE.txt ./

REM Delete any files copied to the dist directory which are not appropriate for distribution
rm  -r -f .\.svn

REM Create the zip file

rm ../vmx-pi.zip
%ZIP_UTILITY% a ../vmx-pi.zip *

REM Cleanup

popd
rm -r -f dist
popd