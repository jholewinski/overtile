#
# CMakeLists.txt: This file is part of the OverTile project.
#
# OverTile: Research compiler for overlapped tiling on GPU architectures
#
# Copyright (C) 2012, University of California Los Angeles
#
# This program can be redistributed and/or modified under the terms
# of the license specified in the LICENSE.txt file at the root of the
# project.
#
# Contact: P Sadayappan <saday@cse.ohio-state.edu>
#

#
# @file: CMakeLists.txt
# @author: Justin Holewinski <justin.holewinski@gmail.com>
#

cmake_minimum_required(VERSION 2.8)

project(overtile)


set(OT_LLVM_SOURCE_DIR "" CACHE PATH
    "Path to LLVM source tree (only needed if using un-installed LLVM")
set(OT_LLVM_BINARY_DIR "" CACHE PATH
    "Path to LLVM build/install tree")


option(USE_INTERNAL_LLVM "Use internal git submodule of LLVM" ON)



if (USE_INTERNAL_LLVM)

  add_subdirectory(external/llvm)


  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/llvm/cmake/modules")

  include(AddLLVM)
  include("${CMAKE_CURRENT_BINARY_DIR}/external/llvm/share/llvm/cmake/LLVMConfig.cmake")
  include(HandleLLVMOptions)

  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/llvm/include")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}/external/llvm/include")

else()

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

  set(LLVM_MAIN_INCLUDE_DIR "${LLVM_SOURCE_DIR}/include")
  set(LLVM_BINARY_DIR ${CMAKE_BINARY_DIR})

  include_directories("${OT_LLVM_BINARY_DIR}/include"
                      "${LLVM_MAIN_INCLUDE_DIR}")
  link_directories("${OT_LLVM_BINARY_DIR}/lib")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


### Bison
find_program(BISON_PROGRAM NAMES bison)
if (BISON_PROGRAM)
  message(STATUS "Found bison: ${BISON_PROGRAM}")
else()
  message(FATAL_ERROR "Bison not found")
endif()

macro(add_bison_target PREFIX GRAMMAR SOURCE HEADER)
  add_custom_command(OUTPUT  ${SOURCE}
                     OUTPUT  ${HEADER}
                     DEPENDS ${GRAMMAR}
                     COMMAND ${BISON_PROGRAM} -p ${PREFIX} -o "${SOURCE}" --defines="${HEADER}" "${CMAKE_CURRENT_SOURCE_DIR}/${GRAMMAR}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()



add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(examples)
add_subdirectory(tests)
