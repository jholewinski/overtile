
#include "overtile/Function.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

Function::Function(Field *Out, Expression *Ex)
  : OutField(Out), Expr(Ex) {
  assert(Out != NULL && "Out cannot be NULL");
  assert(Ex != NULL && "Ex cannot be NULL");
}

Function::~Function() {
}

}
