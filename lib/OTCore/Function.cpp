/*
 * Function.cpp: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, Ohio State University
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: Function.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/Core/Function.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include "overtile/Core/Region.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace llvm;

namespace overtile {

Function::Function(Field *Out)
  : OutField(Out) {
  assert(Out != NULL && "Out cannot be NULL");

  unsigned Dimensions = Out->getGrid()->getNumDimensions();
}

Function::~Function() {
}

std::set<Field*> Function::getInputFields() const {
  std::set<Field*> Fields;

  for (std::list<BoundedFunction>::const_iterator I = Functions.begin(), E = Functions.end(); I != E; ++I) {
    I->Expr->getFields(Fields);
  }

  return Fields;
}

/// FieldRefVisitor - Helper class for adjustRegion.
class FieldRefVisitor {
public:
  FieldRefVisitor(Field *F, Region &FReg, const Region &InReg,
                  const std::list<Field*> &UO, bool Last, const Function *Fn)
    : TheField(F), FRegion (FReg), InRegion(InReg), UpdateOrder(UO), LastTS(Last), Func(Fn) {}

  void visitExpr(Expression *Expr) {
    if (BinaryOp *Op = dyn_cast<BinaryOp>(Expr)) {
      visitBinaryOp(Op);
    } else if (FieldRef *Ref = dyn_cast<FieldRef>(Expr)) {
      visitFieldRef(Ref);
    } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(Expr)) {
      // Do nothing
    } else if (FunctionCall *FC = dyn_cast<FunctionCall>(Expr)) {
      visitFunctionCall(FC);
    } else if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(Expr)) {
      // Do nothing
    } else {
      report_fatal_error("Unhandled expression type");
    }
  }

  void visitFunctionCall(FunctionCall *FC) {
    
    const std::vector<Expression*> &Exprs = FC->getParameters();
    
    for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
      visitExpr(Exprs[i]);
    }
  }

  void visitBinaryOp(BinaryOp *Op) {
    visitExpr(Op->getLHS());
    visitExpr(Op->getRHS());
  }

  void visitFieldRef(FieldRef *Ref) {
    // The real magic happens here
    Field *F = Ref->getField();
    if (F != TheField) {
      // This is a reference to a field we are not processing.
      return;
    }

    // Only adjust if this is not the first update (in time order)
    if (LastTS) {
      // We're in the last time-step, check if there is a write to
      // this field before us.
      std::list<Field*>::const_iterator Curr = std::find(UpdateOrder.begin(),
                                                         UpdateOrder.end(),
                                                         Func->getOutput());
      
      assert(Curr != UpdateOrder.end() && "Field not in update list");

      bool Found = false;
        
      for (std::list<Field*>::const_iterator I = UpdateOrder.begin();
           I     != Curr; ++I) {

        const Field *O  = *I;

        if (O == TheField) {
          
          Found = true;
          break;
        }
      }

      if (!Found) {
        // We are in the last time-step and there is no previous update,
        // so do not count this field.
        return;
      }
    }
    
    unsigned                         NumDims = InRegion.getNumDimensions();
    Region                           NewR(NumDims);
    const std::vector<IntConstant*> &Offsets = Ref->getOffsets();

#if 0
    llvm::errs() << "Visiting " << Ref->getField()->getName() << "[";
    for (unsigned i = 0, e = Offsets.size(); i != e; ++i) {
      if (i != 0) llvm::errs() << "][";
      llvm::errs() << Offsets[i]->getValue();
    }
    llvm::errs() << "]\n";
    
    llvm::errs() << "FRegion In: ";
    FRegion.dump(llvm::errs());
    llvm::errs() << "\n";

    llvm::errs() << "InRegion In: ";
    InRegion.dump(llvm::errs());
    llvm::errs() << "\n";
#endif
    
    for (unsigned i = 0; i < NumDims; ++i) {
      std::pair<int, unsigned> InBound = InRegion.getBound(i);
      std::pair<int, unsigned> FBound  = FRegion.getBound(i);
      int                      Off     = Offsets[i]->getValue();

      if (InBound.first + Off < FBound.first) {
        // We need more elements on the lower bound
        int Diff       = FBound.first - (InBound.first + Off);
        FBound.first  -= Diff;
        FBound.second += Diff;
      }

      if (InBound.first + InBound.second + Off > FBound.first + FBound.second) {
        // We need more elements on the upper bound
        int Diff       = InBound.first + InBound.second + Off -
          (FBound.first + FBound.second);
        FBound.second += Diff;
      }

#if 0
      llvm::errs() << "FRegion Out: ";
      FRegion.dump(llvm::errs());
      llvm::errs() << "\n";
#endif
    
      FRegion.reset(i, FBound);
    }
  }

private:
  Field                   *TheField;
  Region                  &FRegion;
  const Region            &InRegion;
  const std::list<Field*> &UpdateOrder;
  bool                     LastTS;
  const Function          *Func;
};

void Function::
adjustRegion(Field                   *F, Region &FRegion, const Region &InRegion,
             const std::list<Field*> &UpdateOrder, bool LastTS) const {
  FieldRefVisitor V(F, FRegion, InRegion, UpdateOrder, LastTS, this);

  for (std::list<BoundedFunction>::const_iterator I = Functions.begin(), E = Functions.end(); I != E; ++I) {
    V.visitExpr(I->Expr);
  }
}



/// FieldRefVisitor - Helper class for adjustRegion.
class MaxOffsetVisitor {
public:
  MaxOffsetVisitor(const Field *F, unsigned D, unsigned &LeftMax,
                   unsigned    &RightMax)
    : TheField(F), Dim(D), Left(LeftMax), Right(RightMax) {}

  void visitExpr(Expression *Expr) {
    if (BinaryOp *Op = dyn_cast<BinaryOp>(Expr)) {
      visitBinaryOp(Op);
    } else if (FieldRef *Ref = dyn_cast<FieldRef>(Expr)) {
      visitFieldRef(Ref);
    } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(Expr)) {
      // Do nothing
    } else if (FunctionCall *FC = dyn_cast<FunctionCall>(Expr)) {
      visitFunctionCall(FC);
    } else if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(Expr)) {
      // Do nothing
    } else {
      report_fatal_error("Unhandled expression type");
    }
  }

  void visitFunctionCall(FunctionCall *FC) {
    
    const std::vector<Expression*> &Exprs = FC->getParameters();
    
    for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
      visitExpr(Exprs[i]);
    }
  }

  void visitBinaryOp(BinaryOp *Op) {
    visitExpr(Op->getLHS());
    visitExpr(Op->getRHS());
  }

  void visitFieldRef(FieldRef *Ref) {
    // The real magic happens here
    Field *F = Ref->getField();
    if (F != TheField) {
      // This is a reference to a field we are not processing.
      return;
    }

    const std::vector<IntConstant*> &Offsets = Ref->getOffsets();
    assert(Dim < Offsets.size() && "Not enough offsets");

    int Off = Offsets[Dim]->getValue();
    
    if (Off < 0) {
      unsigned Abs = -Off;
      if (Abs > Left)
        Left       = Abs;
    } else if (Off > 0) {
      unsigned Abs = Off;
      if (Abs > Right)
        Right      = Abs;
    }
  }

private:
  const Field *TheField;
  unsigned     Dim;
  unsigned    &Left;
  unsigned    &Right;
};


void Function::getMaxOffsets(const Field *F, unsigned Dim, unsigned &LeftMax,
                             unsigned &RightMax) const {
  LeftMax  = 0;
  RightMax = 0;
  
  MaxOffsetVisitor V(F, Dim, LeftMax, RightMax);

  for (std::list<BoundedFunction>::const_iterator I = Functions.begin(), E = Functions.end(); I != E; ++I) {
    V.visitExpr(I->Expr);
  }  
}



/// FieldRefVisitor - Helper class for adjustRegion.
class FlopsVisitor {
public:
  FlopsVisitor()
    : Flops(0.0) {}

  void visitExpr(Expression *Expr) {
    if (BinaryOp *Op = dyn_cast<BinaryOp>(Expr)) {
      visitBinaryOp(Op);
    } else if (FieldRef *Ref = dyn_cast<FieldRef>(Expr)) {
      visitFieldRef(Ref);
    } else if (ConstantExpr *C = dyn_cast<ConstantExpr>(Expr)) {
      // Do nothing
    } else if (FunctionCall *FC = dyn_cast<FunctionCall>(Expr)) {
      visitFunctionCall(FC);  
    } else if (PlaceHolderExpr *PH = dyn_cast<PlaceHolderExpr>(Expr)) {
      // Do nothing
    } else {
      report_fatal_error("Unhandled expression type");
    }
  }

  void visitFunctionCall(FunctionCall *FC) {

    // For function calls, we count one op plus the parameters
    const std::vector<Expression*> &Exprs = FC->getParameters();

    Flops = Flops + 1.0;
        
    for (unsigned i = 0, e = Exprs.size(); i != e; ++i) {
      visitExpr(Exprs[i]);
    }
  }
  
  void visitBinaryOp(BinaryOp *Op) {
    visitExpr(Op->getLHS());
    visitExpr(Op->getRHS());
    Flops = Flops + 1.0;
  }

  void visitFieldRef(FieldRef *Ref) {
  }

  double getFlops() const { return Flops; }
  
private:
  double Flops;
};


double Function::countFlops() const {
  double Flops = 0.0;

  for (std::list<BoundedFunction>::const_iterator I = Functions.begin(), E = Functions.end(); I != E; ++I) {
    FlopsVisitor V;
    V.visitExpr(I->Expr);
    Flops = std::max(Flops, V.getFlops());
  }

  return Flops;
}




}
