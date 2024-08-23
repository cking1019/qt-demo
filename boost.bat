@echo off
cmake -B "build/release" -DCMAKE_BUILD_TYPE="Release" -G "MinGW Makefiles" -Wno-dev
cmake -B "build/debug" -DCMAKE_BUILD_TYPE="Debug" -G "MinGW Makefiles" -Wno-dev
@REM cmake -B "build/debug" -DCMAKE_BUILD_TYPE="Debug" -GNinja .
@REM cmake -B "build/release" -DCMAKE_BUILD_TYPE="Release" -GNinja .
