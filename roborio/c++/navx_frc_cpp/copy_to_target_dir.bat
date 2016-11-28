mkdir %2\navx-mxp\cpp\include\
cp %1/include/* %2/navx-mxp/cpp/include/
mkdir %2\wpilib\user\cpp\include
cp %1/include/* %2/wpilib/user/cpp/include
mkdir %2\navx-mxp\cpp\src\
cp %1/src/* %2/navx-mxp/cpp/src/
mkdir %2\navx-mxp\cpp\lib\
cp %1/Debug/* %2/navx-mxp/cpp/lib/
mkdir %2\wpilib\user\cpp\lib
cp %1/Debug/* %2/wpilib/user/cpp/lib
