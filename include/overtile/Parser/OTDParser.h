
#ifndef OVERTILE_PARSER_OTDPARSER
#define OVERTILE_PARSER_OTDPARSER

#include "overtile/Core/Grid.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/system_error.h"

namespace overtile {

llvm::error_code ParseOTD(llvm::MemoryBuffer *Src, llvm::OwningPtr<Grid> &G);

}

#endif
