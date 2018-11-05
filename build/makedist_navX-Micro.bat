set ZIP_UTILITY="C:\Program Files\7-Zip\7z"
REM Batch file to create the navX Micro distributable package

pushd ..
md dist
pushd dist

mkdir enclosure

cp -r ../enclosure/navx-micro/* ./enclosure
cp ../setup/Output_navX-Micro/* .
cp ../setup/target/navx-micro/readme.txt ./
cp ../setup/target/navx-micro/wheres_the_source.txt ./
cp ../setup/target/navx-micro/getting_started.txt ./
cp ../LICENSE.txt ./

REM Delete any files copied to the dist directory which are not appropriate for distribution
del /S /Q .\roborio\c++\navXMXP_CPlusPlus_RobotExample\Debug\*.*
del /S /Q .\roborio\java\navXMXPSimpleRobotExample\build\*.*
rm  -r -f .\.svn

REM Create the zip file

rm ../navx-micro.zip
%ZIP_UTILITY% a ../navx-micro.zip *

REM Cleanup

popd
rm -r -f dist
popd

pushd ..
md dist2
pushd dist2

REM Copy binary libraries to the "libraries" directory (for installation by those who don't run the setup program)
mkdir roborio\java\lib
mkdir roborio\java\src
mkdir roborio\java\examples
mkdir roborio\cpp\lib
mkdir roborio\cpp\include
mkdir roborio\cpp\examples
mkdir roborio\cpp\src
mkdir roborio\labview
mkdir android\libs
mkdir android\src
mkdir android\examples

cp ../roborio/java/navx_frc/jar/* ./roborio/java/lib
rm -r ../roborio/java/navXMXP_*/bin
rm -r ../roborio/java/navXMXP_*/build
rm -r ../roborio/java/navXMXP_*/dist
cp -r ../roborio/java/navXMXP_* ./roborio/java/examples
cp -r ../roborio/java/navx/src/com ./roborio/java/src
cp -r ../roborio/java/navx_frc/src/com ./roborio/java/src
cp ../roborio/c++/navx_frc_cpp/Debug/* ./roborio/cpp/lib
cp ../roborio/c++/navx_frc_cpp/include/* ./roborio/cpp/include
rm -r ../roborio/c++/navXMXP_*/Debug
cp -r ../roborio/c++/navXMXP_* ./roborio/cpp/examples
cp -r ../roborio/c++/navx_frc_cpp/.settings ./roborio/cpp/src
cp ../roborio/c++/navx_frc_cpp/* ./roborio/cpp/src
cp -r ../roborio/c++/navx_frc_cpp/src ./roborio/cpp/src
cp -r ../roborio/c++/navx_frc_cpp/include ./roborio/cpp/src
cp -r ../roborio/labview/* ./roborio/labview
cp ../android/navx_ftc/build/outputs/aar/* ./android/libs
cp -r ../android/navx_ftc/src/main/* ./android/src
cp -r ../android/OpModes/* ./android/examples

cp ../build/version.txt .

cp -r ../build/navx-micro-libs-target/* .

REM Create the "libs" zip file

rm ../navx-micro-libs.zip
%ZIP_UTILITY% a ../navx-micro-libs.zip *
popd
popd
