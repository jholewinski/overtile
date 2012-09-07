

#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include "llvm/ADT/APFloat.h"
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace llvm;

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

FunctionCall::FunctionCall(StringRef                       FuncName,
                           const std::vector<Expression*>& Params)
  : Expression(Expression::FunctionCall), Name(FuncName), Exprs(Params) {
  assert(Name.size() > 0 && "Name cannot be empty");
}

FunctionCall::~FunctionCall() {
}


ConstantExpr::ConstantExpr(unsigned ExKind)
  : Expression(ExKind) {
}

ConstantExpr::~ConstantExpr() {
}


FP32Constant::FP32Constant(float V)
  : ConstantExpr(Expression::FP32Const), Value(V) {
  APFloat APV(V);

  SmallVector<char, 32> Str;
  APV.toString(Str);

  std::string Val = std::string(&Str[0]);
  
  if (Val.find('.') == std::string::npos) {
    Val             += ".0";
  }
  
  StringValue = std::string(Val);
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

