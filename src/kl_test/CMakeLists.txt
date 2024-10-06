project(Nebula.test)

message(STATUS "正在构建：${PROJECT_NAME}")
message(STATUS "构建目录: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "源码路径：${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} srcs)

add_executable(${PROJECT_NAME} ${srcs})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)