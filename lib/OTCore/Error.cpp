
#include "overtile/Core/Error.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace overtile {
class _overtile_category : public _do_message {
public:
  virtual const char *name() const {
    return "overtile";
  }

  virtual std::string message(int ev) const {
    switch (ev) {
    case overtile_error::success:
      return "Success";
    case overtile_error::ssp_parse_error:
      return "SSP Parse Error";
    default:
      llvm_unreachable("Invalid overtile_error enumerator");
    }
  }

  virtual llvm::error_condition default_error_condition(int ev) const {
    if (ev == overtile_error::success)
      return errc::success;
    else
      return errc::invalid_argument;
  }
};

const llvm::error_category &overtile_category() {
  static _overtile_category o;
  return o;
}

}
