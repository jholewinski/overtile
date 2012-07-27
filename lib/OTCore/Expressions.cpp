

#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include <sstream>
#include <cassert>
#include <cstdlib>

namespace overtile {

Expression::Expression(unsigned ExKind)
  : ClassType((ExprKind)ExKind) {
}

Expression::~Expression() {
}


BinaryOp::BinaryOp(Operator O, Expression *L, Expression *R)
  : Expression(Expression::BinOp), Op(O), LHS(L), RHS(R) {
  assert(L != NULL && "L cannot be NULL");
  assert(R != NULL && "R cannot be NULL");
}

BinaryOp::~BinaryOp() {
}

void BinaryOp::getFields(std::set<Field*> &Fields) const {
  LHS->getFields(Fields);
  RHS->getFields(Fields);
}

FieldRef::FieldRef(Field *F, const std::vector<IntConstant*>& Off)
  : Expression(Expression::FieldRef), TheField(F), Offsets(Off) {
  assert(F != NULL && "F cannot be NULL");
  assert(Off.size() == F->getGrid()->getNumDimensions() &&
         "Mismatch between number of offsets and grid dimensionality");
}

FieldRef::FieldRef(Field *F, int NumOffsets, IntConstant* Off[])
  : Expression(Expression::FieldRef), TheField(F) {
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

void FieldRef::getFields(std::set<Field*> &Fields) const {
  Fields.insert(TheField);
}


ConstantExpr::ConstantExpr(unsigned ExKind)
  : Expression(ExKind) {
}

ConstantExpr::~ConstantExpr() {
}


FP32Constant::FP32Constant(float V)
  : ConstantExpr(Expression::FP32Const), Value(V) {
  std::stringstream Str;
  Str << V;
  StringValue = Str.str();
}

FP32Constant::~FP32Constant() {
}


IntConstant::IntConstant(int V)
  : ConstantExpr(Expression::IntConst), Value(V) {
  std::stringstream Str;
  Str << V;
  StringValue = Str.str();
}

IntConstant::~IntConstant() {
}


}

