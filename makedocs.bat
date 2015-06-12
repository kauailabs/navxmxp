REM Generate Java Public Class Library Documentation

pushd roborio\java\navXMXPSimpleRobotExample\src
javadoc -d ./docs com.kauailabs.nav6 com.kauailabs.nav6.frc com.kauailabs.navx_mxp com.kauailabs.navx_mxp.frc
popd

REM Generate C++ Public Class Library Documentation

pushd roborio\c++\doxygen

doxygen doxygen.cfg

popd

