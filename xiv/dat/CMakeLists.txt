file(GLOB DAT_PUBLIC_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/xiv/dat/*")
file(GLOB DAT_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_library(dat ${DAT_PUBLIC_INCLUDE_FILES} ${DAT_SOURCE_FILES})
target_link_libraries(dat zlibstatic utils)