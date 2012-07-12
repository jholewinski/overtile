

#include "overtile/Expressions.h"
#include "overtile/Field.h"
#include "overtile/Grid.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

Expression::Expression() {
}

Expression::~Expression() {
}


BinaryOp::BinaryOp(Operator O, Expression *L, Expression *R)
  : Op(O), LHS(L), RHS(R) {
  assert(L != NULL && "L cannot be NULL");
  assert(R != NULL && "R cannot be NULL");
}

BinaryOp::~BinaryOp() {
}

FieldRef::FieldRef(Field *F, const std::vector<int>& Off)
  : TheField(F), Offsets(Off) {
  assert(F != NULL && "F cannot be NULL");
  assert(Off.size() == F->getGrid()->getNumDimensions() &&
         "Mismatch between number of offsets and grid dimensionality");
}

FieldRef::FieldRef(Field *F, int NumOffsets, const int Off[])
  : TheField(F) {
  assert(F != NULL && "F cannot be NULL");
  assert(NumOffsets == F->getGrid()->getNumDimensions() &&
         "Mismatch between number of offsets and grid dimensionality");
  Offsets.reserve(NumOffsets);
  for (unsigned i = 0; i < NumOffsets; ++i) {
    Offsets.push_back(Off[i]);
  }
}

FieldRef::~FieldRef() {
}

}

