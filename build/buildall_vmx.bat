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

%ECLIPSEC_JUNO% -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace_navx-mxp -import ./stm32 -cleanBuild navx-mxp/navX-PI_Release

rmdir /S /Q stm32\bin
mkdir stm32\bin

@echo off
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MAJOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MAJOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MAJOR=%%i
for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MINOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MINOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MINOR=%%i
for /f %%i in ('git rev-list --count --first-parent HEAD') do set VER_REVISION=%%i
set REVISION_STRING=%VER_MAJOR%.%VER_MINOR%.%VER_REVISION%
REM Place version string into setup script 
@echo on

copy .\stm32\navX-PI_Debug\vmx-pi.hex .\stm32\bin\vmx-pi_%REVISION_STRING%.hex

REM Build CSharp Components

pushd .\build
call buildcsharp.bat

REM Build Processing components

call buildprocessing.bat
popd

REM Build setup program

copy .\setup\vmx-pi-setup.iss .\setup\vmx-pi-setup-orig.iss 
Powershell -command "(get-content .\setup\vmx-pi-setup.iss) -replace ('0.0.000','%REVISION_STRING%') | out-file .\setup\vmx-pi-setup.iss -encoding ASCII"
pushd build
call buildsetup_vmx-pi.bat
popd
copy .\setup\vmx-pi-setup-orig.iss .\setup\vmx-pi-setup.iss
del .\setup\vmx-pi-setup-orig.iss

popd
