cmake_minimum_required(VERSION 2.8)
project(xiv)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils "${CMAKE_CURRENT_BINARY_DIR}/utils")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dat/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dat "${CMAKE_CURRENT_BINARY_DIR}/dat")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/exd/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/exd "${CMAKE_CURRENT_BINARY_DIR}/exd")