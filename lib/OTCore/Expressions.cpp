

#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
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

void BinaryOp::replacePlaceHolder(llvm::StringRef Name, Expression *Expr) {
  if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(LHS)) {
    if (PH->getName().compare(Name) == 0) {
      
      delete PH;
      LHS = Expr;
    }
  } else {
    LHS->replacePlaceHolder(Name, Expr);
  }

  if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(RHS)) {
    if (PH->getName().compare(Name) == 0) {
      
      delete PH;
      RHS = Expr;
    }
  } else {
    RHS->replacePlaceHolder(Name, Expr);
  }
}

void BinaryOp::getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const {
  LHS->getPlaceHolders(PH);
  RHS->getPlaceHolders(PH);
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

void FunctionCall::replacePlaceHolder(llvm::StringRef Name, Expression *Expr) {

  for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
    if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(Exprs[i])) {
      
      if (PH->getName().compare(Name) == 0) {
      
        delete PH;
        Exprs[i] = Expr;
      }
    } else {
      Exprs[i]->replacePlaceHolder(Name, Expr);
    }    
  }
}

void FunctionCall::getFields(std::set<Field*> &Fields) const {
  for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
    Exprs[i]->getFields(Fields);
  }
}

void FunctionCall::getPlaceHolders(std::vector<PlaceHolderExpr*> &PH) const {
  for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
    Exprs[i]->getPlaceHolders(PH);
  }
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

  Val += "f";
  
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


PlaceHolderExpr::PlaceHolderExpr(llvm::StringRef Ident)
  : Expression(Expression::PlaceHolder), Name(Ident) {
}

PlaceHolderExpr::~PlaceHolderExpr() {
}

void PlaceHolderExpr::replacePlaceHolder(llvm::StringRef Name, Expression *Expr) {
  report_fatal_error("replacePlaceHolder called in PlaceHolderExpr");
}

}

