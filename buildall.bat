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

REM pushd .\roborio\java\navx-mxp
REM call ant clean jar
REM popd

REM copy .\roborio\java\navx-mxp\build\*.jar .\processing\libraries\navx_mxp\library\

REM
REM Begin a command-line "clean build" of the Debug version of the navx-mxp firmware
REM

REM TODO:  Update the revision number to the latest revision number from subversion

REM pushd .\stm32\navx-mxp
REM echo|set /p=#define NAVX_MXP_REVISION  > revision.h
REM svn info | grep -i revision | cut -f2 -d: | tr -d [:space:] >> revision.h
REM popd

rm -r -f ./build_workspace
mkdir build_workspace

C:\Eclipse\eclipsec.exe -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ./build_workspace -import ./stm32 -cleanBuild navx-mxp/Debug
rmdir /S /Q stm32\bin
mkdir stm32\bin

REM @echo off
REM for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MAJOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MAJOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MAJOR=%%i
REM for /f %%i in ('grep NAVX_MXP_FIRMWARE_VERSION_MINOR stm32/navx-mxp/version.h ^| sed 's/#define NAVX_MXP_FIRMWARE_VERSION_MINOR/ /' ^| sed 's/^[ \t]*//'') do set VER_MINOR=%%i
REM for /f %%i in ('svn info ^| grep Revision: ^| sed 's/Revision:/ /' ^| sed 's/^[ \t]*//') do set VER_REVISION=%%i
REM set REVISION_STRING=%VER_MAJOR%.%VER_MINOR%.%VER_REVISION%
REM @echo on

set REVISION_STRING=1.1.0

copy .\stm32\Debug\navx-mxp.hex .\stm32\bin\navx-mxp_%REVISION_STRING%.hex


