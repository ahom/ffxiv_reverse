file(GLOB EXD_PUBLIC_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/xiv/exd/*")
file(GLOB EXD_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_library(exd ${EXD_PUBLIC_INCLUDE_FILES} ${EXD_SOURCE_FILES})
target_link_libraries(exd dat)