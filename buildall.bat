REM Build all binaries

REM
REM Begin a command-line "clean build" of the CRio java libraries
REM

pushd .\crio\java\nav6
call ant clean jar
popd

copy .\crio\java\nav6\build\*.jar .\processing\libraries\nav6\library\

REM
REM Begin a command-line "clean build" of the Roborio java libraries
REM

pushd .\roborio\java\navx-mxp
call ant clean jar
popd

copy .\roborio\java\navx-mxp\build\*.jar .\processing\libraries\navx_mxp\library\

REM
REM Begin a command-line "clean build" of the Debug version of the navx-mxp firmware
REM

REM Use the GIT checkin count as the revision number 

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

REM Build setup program

call buildsetup.bat

