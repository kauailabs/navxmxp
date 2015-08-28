set ZIP_UTILITY="C:\Program Files\7-Zip\7z"
REM Batch file to create the navX MXP distributable package

pushd ..
md dist
pushd dist

mkdir enclosure

cp -r ../enclosure/* ./enclosure
cp ../setup/Output/* .
cp ../setup/target/readme.txt ./
cp ../setup/target/wheres_the_source.txt ./
cp ../setup/getting_started.txt ./
cp ../LICENSE.txt ./

REM Delete any files copied to the dist directory which are not appropriate for distribution
del /S /Q .\roborio\c++\navXMXP_CPlusPlus_RobotExample\Debug\*.*
del /S /Q .\roborio\java\navXMXPSimpleRobotExample\build\*.*
rm  -r -f .\.svn

REM Create the zip file

rm ../navx-mxp.zip
%ZIP_UTILITY% a ../navx-mxp.zip *

REM Cleanup

popd
rm -r -f dist
popd