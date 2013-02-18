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

add_llvm_library(OTCore
  BackEnd.cpp
  CudaBackEnd.cpp
  Error.cpp
  Expressions.cpp
  Field.cpp
  Function.cpp
  Grid.cpp
  Region.cpp
  Types.cpp
)

install(TARGETS OTCore ARCHIVE DESTINATION lib)
