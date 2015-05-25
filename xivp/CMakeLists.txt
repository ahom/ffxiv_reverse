cmake_minimum_required(VERSION 2.8)
project(xivp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../zlib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../zlib "${CMAKE_CURRENT_BINARY_DIR}/zlib")
include_directories(${CMAKE_CURRENT_BINARY_DIR}/zlib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../boost)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../boost/stage/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xiv/utils/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xiv/dat/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xiv/exd/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xiv "${CMAKE_CURRENT_BINARY_DIR}/xiv")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tex/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tex "${CMAKE_CURRENT_BINARY_DIR}/tex")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mdl/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mdl "${CMAKE_CURRENT_BINARY_DIR}/mdl")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cli "${CMAKE_CURRENT_BINARY_DIR}/cli")
