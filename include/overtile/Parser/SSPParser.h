
#ifndef OVERTILE_PARSER_SSPPARSER_H
#define OVERTILE_PARSER_SSPPARSER_H

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/system_error.h"
#include <vector>

namespace overtile {

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

private:
  unsigned                      CurPos;
  llvm::MemoryBuffer           *Buf;
  llvm::SourceMgr              &SrcMgr;
  std::vector<llvm::StringRef>  InternedStrings;
};

inline int SSPlex(void *Val, void *Data) {
  return static_cast<SSPParser*>(Data)->getNextToken(Val);
}

}

#endif
