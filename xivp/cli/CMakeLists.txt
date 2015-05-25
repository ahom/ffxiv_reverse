file(GLOB CLI_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_executable(cli ${CLI_SOURCE_FILES})
target_link_libraries(cli exd tex mdl)