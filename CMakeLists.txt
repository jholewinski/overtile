
cmake_minimum_required(VERSION 2.8)
project(overtile)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${HEADER_DIR})

add_subdirectory(lib)

install(DIRECTORY include/overtile DESTINATION include)
