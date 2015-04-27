REM Clean all binaries

REM
REM Begin a command-line "clean" of the CRio java libraries
REM

pushd .\crio\java\nav6
call ant clean 
popd

REM
REM Begin a command-line "clean" of the Debug version of the navx-mxp firmware
REM

pushd .\stm32\
del /S /Q Debug\*.*
del /S /Q Release\*.*
popd

