#/bin/bash
pushd hal_cpp/Debug
make
popd
pushd hal_cpp
sudo make install
popd
pushd hal_cpp_test/Debug
make
popd