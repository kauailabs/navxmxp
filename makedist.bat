REM Batch file to create the navX MXP distributable package

md dist
pushd dist

mkdir enclosure

cp -r ../enclosure/* ./enclosure
cp ../setup/Output/* .
cp ../readme.txt ./
cp ../wheres_the_source.txt ./
cp ../getting_started.txt ./
cp ../LICENSE.txt ./

REM Delete any files copied to the dist directory which are not appropriate for distribution
del /S /Q .\roborio\c++\navXMXP_CPlusPlus_RobotExample\Debug\*.*
del /S /Q .\roborio\java\navXMXPSimpleRobotExample\build\*.*
rm  -r -f .\.svn

REM Create the zip file

rm ../navx-mxp.zip
"C:\Program Files\7-Zip\7z" a ../navx-mxp.zip *

REM Cleanup

popd
rm -r -f dist

REM Copy build to the ftp server

REM dosyncftp