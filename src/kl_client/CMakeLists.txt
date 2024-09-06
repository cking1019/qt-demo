project(Nebula.RelayModuleCK)
message(STATUS "正在构建：${PROJECT_NAME}")
message(STATUS "构建目录: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "源码路径：${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(PRUE)
include_directories(Nebula)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} srcs)
aux_source_directory(PRUE src)
aux_source_directory(Nebula src)

add_executable(${PROJECT_NAME} ${srcs})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/release/conf/)
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/conf/module.ini ${CMAKE_SOURCE_DIR}/release/conf/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/conf/Detect.json ${CMAKE_SOURCE_DIR}/release/conf/
)