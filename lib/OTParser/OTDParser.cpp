
#include "overtile/Core/Error.h"
#include "overtile/Parser/OTDParser.h"

using namespace llvm;

namespace overtile {

error_code ParseOTD(MemoryBuffer *Src, OwningPtr<Grid> &G) {
  return make_error_code(overtile_error::success);
}

}
