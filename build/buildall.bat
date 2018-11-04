SET ECLIPSEC_MARS=C:\Users\Scott\eclipse\cpp-mars\eclipse\eclipsec.exe
SET ECLIPSEC_JUNO=C:\Eclipse\eclipsec.exe
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

REM
REM Begin a command-line "clean build" of the Debug version of the navx-mxp firmware
REM

REM Use the GIT checkin count as the firmware revision number 

pushd .\stm32\navx-mxp
echo|set /p=#define NAVX_MXP_REVISION  > revision.h
git rev-list --count --first-parent HEAD >> revision.h
popd

REM Build navX-MXP and navX-Micro Firmware

rm -r -f ./build_workspace_navx-mxp
mkdir build_workspace_navx-mxp

%ECLIPSEC_JUNO% -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace_navx-mxp -import ./stm32 -cleanBuild navx-mxp/navX-MXP_Debug

rm -r -f ./build_workspace_navx-micro
mkdir build_workspace_navx-micro

%ECLIPSEC_JUNO% -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace_navx-micro -import ./stm32 -cleanBuild navx-mxp/navX-Micro_Debug

rmdir /S /Q stm32\bin
mkdir stm32\bin

@echo off
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MAJOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MAJOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MAJOR=%%i
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MINOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MINOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MINOR=%%i
for /f %%i in ('git rev-list --count --first-parent HEAD') do set VER_REVISION=%%i
set REVISION_STRING=%VER_MAJOR%.%VER_MINOR%.%VER_REVISION%
REM Place version string into setup script 
@echo on

copy .\stm32\navX-MXP_Debug\navx-mxp.hex .\stm32\bin\navx-mxp_%REVISION_STRING%.hex
copy .\stm32\navX-Micro_Debug\navx-micro.hex .\stm32\bin\navx-micro_%REVISION_STRING%.hex

REM Build CSharp Components

pushd .\build
call buildcsharp.bat

REM Build Processing components

call buildprocessing.bat
popd

REM
REM Begin a command-line "clean build" of the navx frc C++ library
REM

pushd .\roborio\c++\navx_frc_cpp
call gradlew clean
call gradlew build
REM For now, publish to a local maven repo, for access by setup builder and the maven_deploy script.
call gradlew publish
REM rm -r -f ./build_workspace_luna
REM mkdir build_workspace_luna

REM %ECLIPSEC_MARS% -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace_luna -import ./navx_frc_cpp -cleanBuild navx_frc_cpp/Debug
popd

REM
REM Begin a command-line "clean build" of the navx frc Java library
REM

pushd .\roborio\java\navx_frc
REM call ant clean build
call gradlew clean
call gradlew build
REM For now, publish to a local maven repo, for access by setup builder and the maven_deploy script.
call gradlew publish
popd

REM Build FTC Library
pushd .\android\navx_ftc
rmdir /S /Q build
call gradlew.bat assembleDebug
call gradlew.bat assembleRelease
popd

REM Build setup program

copy .\setup\navx-mxp-setup.iss .\setup\navx-mxp-setup-orig.iss 
Powershell -command "(get-content .\setup\navx-mxp-setup.iss) -replace ('0.0.000','%REVISION_STRING%') | out-file .\setup\navx-mxp-setup.iss -encoding ASCII"
pushd build
call buildsetup_navx-mxp.bat
popd
copy .\setup\navx-mxp-setup-orig.iss .\setup\navx-mxp-setup.iss
del .\setup\navx-mxp-setup-orig.iss

copy .\setup\navx-micro-setup.iss .\setup\navx-micro-setup-orig.iss 
Powershell -command "(get-content .\setup\navx-micro-setup.iss) -replace ('0.0.000','%REVISION_STRING%') | out-file .\setup\navx-micro-setup.iss -encoding ASCII"
pushd build
echo %REVISION_STRING% > version.txt
call buildsetup_navx-micro.bat
popd
copy .\setup\navx-micro-setup-orig.iss .\setup\navx-micro-setup.iss
del .\setup\navx-micro-setup-orig.iss

copy .\build\vendordeps\navx_frc.json.template .\build\vendordeps\navx_frc.json
Powershell -command "(get-content .\build\vendordeps\navx_frc.json) -replace ('0.0.000','%REVISION_STRING%') | out-file .\build\vendordeps\navx_frc.json -encoding 

popd
