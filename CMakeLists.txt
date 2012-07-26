
cmake_minimum_required(VERSION 2.8)

project(overtile)


set(OT_LLVM_SOURCE_DIR "" CACHE PATH
    "Path to LLVM source tree (only needed if using un-installed LLVM")
set(OT_LLVM_BINARY_DIR "" CACHE PATH
    "Path to LLVM build/install tree")

if (OT_LLVM_SOURCE_DIR)
  get_filename_component(LLVM_SOURCE_DIR ${OT_LLVM_SOURCE_DIR} ABSOLUTE)
  list(APPEND CMAKE_MODULE_PATH "${LLVM_SOURCE_DIR}/cmake/modules")
endif()

if (NOT OT_LLVM_BINARY_DIR)
  message(FATAL_ERROR 
    "OT_LLVM_BINARY_DIR must point to an LLVM build/install tree")
endif ()

list(APPEND CMAKE_MODULE_PATH "${OT_LLVM_BINARY_DIR}/share/llvm/cmake")
get_filename_component(PATH_TO_LLVM_BUILD ${OT_LLVM_BINARY_DIR} ABSOLUTE)

include(AddLLVM)
include("${OT_LLVM_BINARY_DIR}/share/llvm/cmake/LLVMConfig.cmake")
include(HandleLLVMOptions)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LLVM_MAIN_INCLUDE_DIR "${LLVM_SOURCE_DIR}/include")
set(LLVM_BINARY_DIR ${CMAKE_BINARY_DIR})

include_directories("${OT_LLVM_BINARY_DIR}/include"
                    "${LLVM_MAIN_INCLUDE_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include")
link_directories("${OT_LLVM_BINARY_DIR}/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_subdirectory(lib)
add_subdirectory(tools)
