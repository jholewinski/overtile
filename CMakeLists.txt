
cmake_minimum_required(VERSION 2.8)
project(overtile)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${HEADER_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/support/opencl/inc)

add_subdirectory(lib)
add_subdirectory(examples)

install(DIRECTORY include/overtile DESTINATION include)
