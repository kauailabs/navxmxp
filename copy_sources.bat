SET NAVX_SOURCE_ROOT=C:\Users\Scott\Documents\svn_Kauailabs\Schematics\nav10\nav10_mxp_3_3
SET NAVX_PUBLIC_SVN_ROOT=C:\Users\Scott\Documents\svn_navx-mxp

copy *.* %NAVX_PUBLIC_SVN_ROOT%\
copy %NAVX_SOURCE_ROOT%\Firmware\MPU9250\INVENSENSE_LICENSE_README.txt %NAVX_PUBLIC_SVN_ROOT%\
pushd %NAVX_PUBLIC_SVN_ROOT%
if not exist schematic mkdir schematics
copy %NAVX_SOURCE_ROOT%\CircuitDesign\nav10_mxp_3_3_final_expio.zip schematics
copy %NAVX_SOURCE_ROOT%\CircuitDesign\nav10_mxp_3_3_final_expio.sch schematics
copy %NAVX_SOURCE_ROOT%\CircuitDesign\nav10_mxp_3_3_final_expio.brd schematics
copy %NAVX_SOURCE_ROOT%\CircuitDesign\nav10_mxp_3_3_bom_digikey_format.csv schematics
if not exist docs mkdir docs
if not exist enclosure mkdir enclosure
if not exist processing mkdir processing
if not exist roborio mkdir roborio
if not exist images mkdir images
pushd docs
if not exist datasheets mkdir datasheets
if not exist reference mkdir reference
popd
copy %NAVX_SOURCE_ROOT%\Docs\Reference\*.* docs\reference
copy %NAVX_SOURCE_ROOT%\Datasheets\*.* docs\datasheets\
copy %NAVX_SOURCE_ROOT%\Docs\Images\*.* schematics
if not exist stm32 mkdir stm32
pushd stm32
xcopy %NAVX_SOURCE_ROOT%\Firmware ./E /V /Y
rmdir /S /Q Debug
mkdir Debug
copy %NAVX_SOURCE_ROOT%\Firmware\Debug\*.bat Debug\
rmdir /S /Q .settings
rmdir /S /Q Projects
rmdir /S /Q MPU9250\core
rmdir /S /Q MPU9250\mpl
pushd MPU9250
mkdir core
mkdir mpl
popd
popd
popd