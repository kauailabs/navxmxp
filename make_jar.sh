#!/bin/bash
pushd java/navx
mkdir jar
pushd src
javac com/kauailabs/navx/*.java
jar cf ../jar/navx.jar com/kauailabs/navx/*.class
popd
popd
pushd roborio/java/navx_frc
mkdir jar
pushd src
javac -classpath ".:../../../../java/navx/jar/navx.jar:/usr/local/frc/third-party/java/wpilibj.jar:/usr/local/frc/third-party/java/wpiutil.jar:../../../java/navx/jar/navx.jar:/usr/local/frc/third-party/java/vmxHalMau.jar:" com/kauailabs/navx/frc/*.java 
mkdir com/kauailabs/navx
cp ../../../../java/navx/src/com/kauailabs/navx/*.class com/kauailabs/navx/
jar cf ../jar/navx_frc.jar com/kauailabs/navx/*.class com/kauailabs/navx/frc/*.class
sudo cp ../jar/navx_frc.jar /usr/local/frc/third-party/java
popd
popd