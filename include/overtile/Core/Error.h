

#ifndef OVERTILE_CORE_ERROR_H
#define OVERTILE_CORE_ERROR_H

#include "llvm/Support/system_error.h"

namespace overtile {

// Error handling plumbing
const llvm::error_category &overtile_category();

struct overtile_error {
  enum _ {
    success = 0,
    invalid_yaml,
  };
  _ V_;

  overtile_error(_ v) : V_(v) {}
  explicit overtile_error(int v) : V_(_(v)) {}
  operator int() const { return V_; }
};

inline llvm::error_code make_error_code(overtile_error e) {
  return llvm::error_code(static_cast<int>(e), overtile_category());
}
}

namespace llvm {
template <> struct is_error_code_enum<overtile::overtile_error> : true_type {};
template <>
struct is_error_code_enum<overtile::overtile_error::_> : true_type {};
}

#endif
