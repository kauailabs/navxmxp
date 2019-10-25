REM Clean all binaries

REM
REM Begin a command-line "clean" of the CRio java libraries
REM

pushd ..
pushd .\roborio\java\navx_frc
call gradlew clean 
popd
pushd .\roborio\c++\navx_frc_cpp
call gradlew clean
popd

REM
REM Begin a command-line "clean" of the Debug version of the navx-mxp firmware
REM

pushd .\stm32\
del /S /Q Debug\*.*
del /S /Q Release\*.*
popd

REM
REM clean the maven publications
REM

rmdir /S /Q publish\maven
rmdir /S /Q %HOMEDRIVE%%HOMEPATH%\.m2\repository\com\kauailabs\navx

popd