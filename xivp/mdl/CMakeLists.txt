file(GLOB MDL_PUBLIC_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/xiv/mdl/*")
file(GLOB MDL_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_library(mdl ${MDL_PUBLIC_INCLUDE_FILES} ${MDL_SOURCE_FILES})
target_link_libraries(mdl tex)