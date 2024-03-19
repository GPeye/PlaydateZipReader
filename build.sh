#!/bin/bash
rm -rf ./build/
mkdir build
cd build
#cmake ..
cmake -DCMAKE_TOOLCHAIN_FILE=$PLAYDATE_SDK_PATH/C_API/buildsupport/arm.cmake ..
make