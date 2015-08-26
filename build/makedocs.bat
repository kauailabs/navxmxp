REM Generate Java Public Class Library Documentation

pushd ..
pushd roborio\java\navx_frc\src
javadoc -d ./docs com.kauailabs.navx.frc
popd

REM Generate C++ Public Class Library Documentation

pushd roborio\c++\doxygen

doxygen doxygen.cfg

popd
popd
