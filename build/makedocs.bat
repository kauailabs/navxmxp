REM Generate Java Public Class Library Documentation

pushd ..

REM FRC Library

pushd roborio\java\navx_frc\src
javadoc -d ./docs -overview ./overview.htm com.kauailabs.navx.frc
popd

REM FTC Library

pushd android\navx_ftc\src\main\java
javadoc -d ../../../docs -overview ./overview.htm com.kauailabs.navx.ftc
popd

REM Generate C++ Public Class Library Documentation

pushd roborio\c++\doxygen

doxygen doxygen.cfg

popd
popd
