project(Nebula.ui)
message(STATUS "正在构建：${PROJECT_NAME}")
message(STATUS "构建目录: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "源码路径：${CMAKE_CURRENT_SOURCE_DIR}")

# 关闭控制台显示
if(WIN32)
    # set(CMAKE_WIN32_EXECUTABLE true)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} srcs)

add_executable(${PROJECT_NAME} ${srcs})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    Qt5::Gui
    Qt5::Widgets
    Qt::Sql
)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)