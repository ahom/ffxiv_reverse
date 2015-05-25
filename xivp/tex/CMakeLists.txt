file(GLOB TEX_PUBLIC_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/xiv/tex/*")
file(GLOB TEX_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_library(tex ${TEX_PUBLIC_INCLUDE_FILES} ${TEX_SOURCE_FILES})
target_link_libraries(tex dat)