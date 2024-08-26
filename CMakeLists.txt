cmake_minimum_required(VERSION 3.10)
project(Qt-Demo)

message(STATUS "正在构建: "${PROJECT_NAME})
message(STATUS "CMake命令: "${CMAKE_COMMAND})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ENV{QT_VERSION_NUM} "5.15.1")
message(STATUS "QT_VERSION_NUM is: $ENV{QT_VERSION_NUM}")
# Linux下配置Qt非apt安装的Qt环境，不然找不到Qt模块
if (DEFINED ENV{QT_VERSION_NUM})
    set(QT_VERSION $ENV{QT_VERSION_NUM} CACHE INTERNAL "Qt5 version")
    message(STATUS "当前Qt版本号为: $ENV{QT_VERSION_NUM}")
else ()
    set(QT_VERSION $ENV{QT_VERSION_NUM} CACHE INTERNAL "Qt5 version")
    message(STATUS "当前Qt版本号为: $ENV{QT_VERSION_NUM}")
endif ()


# Linux下环境配置
if (UNIX)
    set(QT_PLATFORM gcc_64 CACHE INTERNAL "Qt5 platform")
    set(Qt5_SDK_DIR "$ENV{HOME}/Qt${QT_VERSION}/${QT_VERSION}/${QT_PLATFORM}" CACHE INTERNAL "Qt5 sdk dir")
    set(Qt5_DIR "/usr/bin/qt5.15.1/lib/cmake/Qt5" CACHE INTERNAL "Qt5 cmake dir")
    message(STATUS "qt5_dir: ${Qt5_DIR}")
    message(STATUS "build in Linux")
# Window下环境配置
elseif (WIN32)
    if (MSVC)
        if (CMAKE_CL_64)
            set(QT_PLATFORM msvc2019_64 CACHE INTERNAL "Qt5 platform")
        else ()
            set(QT_PLATFORM msvc2019 CACHE INTERNAL "Qt5 platform")
        endif ()
        set(Qt5_SDK_DIR "C:/Qt/${QT_VERSION}/${QT_PLATFORM}" CACHE INTERNAL "Qt5 sdk dir")
        message(STATUS "build with msvc Qt ${Qt5_SDK_DIR}")
        set(Qt5_DIR "${Qt5_SDK_DIR}/lib/cmake/Qt5" CACHE INTERNAL "Qt5 cmake dir")
    else ()
        set(Qt5_SDK_DIR "D:/dev/Qt5.15.1" CACHE INTERNAL "Qt5 sdk dir")
        message(STATUS "build with MinGW Qt ${Qt5_SDK_DIR}")
        set(Qt5_DIR "${Qt5_SDK_DIR}/lib/cmake/Qt5" CACHE INTERNAL "Qt5 cmake dir")
    endif()
    message(STATUS "build in Windows")
endif ()

find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_subdirectory(src/client)
add_subdirectory(src/server)

add_custom_target(${PROJECT_NAME} ALL 
                    DEPENDS client server 
                    COMMENT "cmake is building target")
