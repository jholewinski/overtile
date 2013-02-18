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

add_bison_target(SSP SSPGrammar.y SSPParserGenerated.cpp SSPParserGenerated.h)
add_llvm_library(OTParser
  SSPParser.cpp
  SSPParserGenerated.cpp
)

install(TARGETS OTParser ARCHIVE DESTINATION lib)
