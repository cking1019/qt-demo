project(client)
message("==================================================")
message("正在构建："${PROJECT_NAME})
message("构建目录: "${CMAKE_CURRENT_BINARY_DIR})
message("源码路径："${CMAKE_CURRENT_SOURCE_DIR})
message("==================================================")
# message(ENV{PATH})
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
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/release/conf/)
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/conf/module.ini ${CMAKE_SOURCE_DIR}/release/conf/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/conf/Detect.json ${CMAKE_SOURCE_DIR}/release/conf/
)