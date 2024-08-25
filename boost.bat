@echo off
cmake -B "build/release" -DCMAKE_BUILD_TYPE="Release" -G "MinGW Makefiles" -Wno-dev
