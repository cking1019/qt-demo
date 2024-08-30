@echo off
cmake -B "build/release" -DCMAKE_BUILD_TYPE="Release" -G "MinGW Makefiles" -Wno-dev
@REM 会生成debug版本的makefile配置文件
@REM cmake -B "build/debug" -DCMAKE_BUILD_TYPE="Debug" -G "MinGW Makefiles" -Wno-dev
