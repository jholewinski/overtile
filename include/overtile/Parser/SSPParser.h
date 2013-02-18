/*
 * SSPParser.h: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, Ohio State University
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: SSPParser.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_PARSER_SSPPARSER_H
#define OVERTILE_PARSER_SSPPARSER_H

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/system_error.h"
#include <vector>
#include <cassert>

namespace overtile {

class Grid;

class SSPParser {
public:
  /// SPPParser - Constructs a new parser.  This method takes ownership of
  /// the memory buffer \p Buffer.
  SSPParser(llvm::MemoryBuffer *Buffer, llvm::SourceMgr &SM);
  ~SSPParser();

  /// parseBuffer - Parses the stencil specification.
  llvm::error_code parseBuffer();

  /// getNextToken - Returns the next token in the input stream (INTERNAL USE ONLY)
  int getNextToken(void *Val);

  /// getGrid - Returns the Grid parsed from the specification.
  Grid *getGrid() {
    assert(G && "Grid is NULL");
    return G;
  }


  // INTERNAL PARSER METHODS
  void setGrid(Grid *GD) {
    assert(G == NULL && "Grid already set");
    G = GD;
  }

  void printError(llvm::StringRef Msg, bool PrintLoc=true);
  
private:
  unsigned                       CurPos;
  llvm::MemoryBuffer            *Buf;
  llvm::SourceMgr               &SrcMgr;
  std::vector<llvm::StringRef*>  InternedStrings;
  Grid                          *G;                                               
};

inline int SSPlex(void *Val, void *Data) {
  return static_cast<SSPParser*>(Data)->getNextToken(Val);
}

}

#endif
