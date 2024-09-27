cmake_minimum_required(VERSION 3.10)
# set (CMAKE_C_COMPILER "D:/dev/lang/mingw64/bin/gcc.exe")
# set (CMAKE_CXX_COMPILER "D:/dev/lang/mingw64/bin/g++.exe")

project(Nebula.North)

message(STATUS "正在构建: ${PROJECT_NAME}")
message(STATUS "CMake命令: ${CMAKE_COMMAND}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 添加预处理，防止在release模式下，输出的日志信息不显示文件名，行号等信息
add_definitions(-DQT_MESSAGELOGCONTEXT)

set(QT_VERSION, "5.15.1")
# Linux下环境配置
if (UNIX)
    set(QT_PLATFORM gcc_64 CACHE INTERNAL "Qt5 platform")
    set(Qt5_SDK_DIR "$ENV{HOME}/Qt${QT_VERSION}/${QT_VERSION}/${QT_PLATFORM}" CACHE INTERNAL "Qt5 sdk dir")
    set(Qt5_DIR "/usr/bin/qt5.15.1/lib/cmake/Qt5" CACHE INTERNAL "Qt5 cmake dir")
    message(STATUS "qt5_dir: ${Qt5_DIR}")
    message(STATUS "build in Linux")
# Window下环境配置
elseif (WIN32)
    set(Qt5_SDK_DIR "D:/dev/Qt5.15.1" CACHE INTERNAL "Qt5 sdk dir")
    set(Qt5_DIR "${Qt5_SDK_DIR}/lib/cmake/Qt5" CACHE INTERNAL "Qt5 cmake dir")
    message(STATUS "build with MinGW Qt ${Qt5_SDK_DIR}")
    message(STATUS "build in Windows")
endif ()

# 寻找动态库
find_package(Qt5 COMPONENTS Core Network Gui Widgets Sql REQUIRED)

add_subdirectory(src/kl_client)
add_subdirectory(src/kl_test)
add_subdirectory(src/kl_ui)
add_subdirectory(src/kl_server)

# add_custom_target(${PROJECT_NAME} ALL 
#                     DEPENDS 
#                     Nebula.RelayModuleCK Nebula.test Nebula.ui Nebula.server
#                     COMMENT "cmake is building target")
