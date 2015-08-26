REM Build all binaries

REM CD to Root Directory
pushd ..

REM
REM Begin a command-line "clean build" of the navx protocol java library
REM
pushd .\java\navx
call ant clean build
popd

REM
REM Begin a command-line "clean build" of the navx frc java library
REM

pushd .\roborio\java\navx_frc
call ant clean build
popd

REM
REM Begin a command-line "clean build" of the navx frc C++ library
REM

pushd .\roborio\c++
rm -r -f ./build_workspace_luna
mkdir build_workspace_luna

C:\Eclipse_Luna\eclipsec.exe -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace_luna -import ./navx_frc_cpp -cleanBuild navx_frc_cpp/Debug
popd

REM
REM Copy navx protocol java library file to project folders that need it.
REM

mkdir .\processing\libraries\navx\library\
copy .\java\navx\jar\*.jar .\processing\libraries\navx\library\ /Y

REM
REM Copy the navX protocol library .h files to the 
REM projects that need them.
REM

cp ./stm32/navx-mxp/IMU*.h  ./arduino/navXTestJig/
cp ./stm32/navx-mxp/AHRS*.h ./arduino/navXTestJig/
cp ./stm32/navx-mxp/IMU*.h  ./roborio/c++/navx_frc_cpp/src
cp ./stm32/navx-mxp/AHRS*.h ./roborio/c++/navx_frc_cpp/src

REM
REM Begin a command-line "clean build" of the Debug version of the navx-mxp firmware
REM

REM Use the GIT checkin count as the firmware revision number 

pushd .\stm32\navx-mxp
echo|set /p=#define NAVX_MXP_REVISION  > revision.h
git rev-list --count --first-parent HEAD >> revision.h
popd

rm -r -f ./build_workspace
mkdir build_workspace

C:\Eclipse\eclipsec.exe -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace -import ./stm32 -cleanBuild navx-mxp/Debug
rmdir /S /Q stm32\bin
mkdir stm32\bin

@echo off
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MAJOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MAJOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MAJOR=%%i
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MINOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MINOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MINOR=%%i
for /f %%i in ('git rev-list --count --first-parent HEAD') do set VER_REVISION=%%i
set REVISION_STRING=%VER_MAJOR%.%VER_MINOR%.%VER_REVISION%
@echo on

copy .\stm32\Debug\navx-mxp.hex .\stm32\bin\navx-mxp_%REVISION_STRING%.hex

REM Build CSharp Components

call buildcsharp.bat

REM Build Processing components

call buildprocessing.bat

REM Build setup program

call buildsetup.bat

popd

