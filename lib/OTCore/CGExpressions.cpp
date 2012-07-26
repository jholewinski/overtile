

#include "overtile/CGExpressions.h"
#include "overtile/Field.h"
#include "overtile/Grid.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

CGExpression::CGExpression() {
}

CGExpression::~CGExpression() {
}


CGBinaryOp::CGBinaryOp(Operator O, CGExpression *L, CGExpression *R)
  : Op(O), LHS(L), RHS(R) {
  assert(L != NULL && "L cannot be NULL");
  assert(R != NULL && "R cannot be NULL");
}

CGBinaryOp::~CGBinaryOp() {
}

CGFieldRef::CGFieldRef(Field *F, const std::vector<CGExpression*>& Off)
  : TheField(F), Offsets(Off) {
  assert(F != NULL && "F cannot be NULL");
  assert(Off.size() == F->getGrid()->getNumDimensions() &&
         "Mismatch between number of offsets and grid dimensionality");
}

CGFieldRef::CGFieldRef(Field *F, int NumOffsets, CGExpression* Off[])
  : TheField(F) {
  assert(F != NULL && "F cannot be NULL");
  assert(NumOffsets == F->getGrid()->getNumDimensions() &&
         "Mismatch between number of offsets and grid dimensionality");
  Offsets.reserve(NumOffsets);
  for (unsigned i = 0; i < NumOffsets; ++i) {
    Offsets.push_back(Off[i]);
  }
}

CGFieldRef::~CGFieldRef() {
}

}

