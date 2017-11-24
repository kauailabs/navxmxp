REM Clean all binaries

pushd ..

REM
REM Cleanup any existing builds of the vmx-pi firmware
REM

pushd .\stm32\
del /S /Q Debug\*.*
del /S /Q Release\*.*
popd

popd