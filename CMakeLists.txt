
cmake_minimum_required(VERSION 2.8)
project(overtile)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${HEADER_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/support/opencl/inc)

add_subdirectory(lib)
add_subdirectory(examples)

# Create Doxyfile
set(OVERTILE_DOC_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
set(OVERTILE_INCLUDE_DIR ${HEADER_DIR})
configure_file(Doxyfile.in Doxyfile @ONLY)

install(DIRECTORY include/overtile DESTINATION include)
