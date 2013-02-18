#
# CMakeLists.txt: This file is part of the OverTile project.
#
# OverTile: Research compiler for overlapped tiling on GPU architectures
#
# Copyright (C) 2012, Ohio State University
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

set(_sources
  CLContext.cpp
)

set(_headers
  ${HEADER_DIR}/overtile/cl/CLCommon.h
  ${HEADER_DIR}/overtile/cl/CLContext.h
)


add_library(overtile-cl STATIC ${_sources} ${_headers})

install(TARGETS overtile-cl ARCHIVE DESTINATION lib)
