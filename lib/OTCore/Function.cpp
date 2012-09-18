
#include "overtile/Core/Function.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include "overtile/Core/Region.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace llvm;

namespace overtile {

Function::Function(Field *Out, Expression *Ex)
  : OutField(Out), Expr(Ex) {
  assert(Out != NULL && "Out cannot be NULL");
  assert(Ex != NULL && "Ex cannot be NULL");

  unsigned Dimensions = Out->getGrid()->getNumDimensions();
  
  for (unsigned i = 0; i < Dimensions; ++i) {
    Bounds.push_back(std::make_pair<unsigned, unsigned>(0, 0));
  }
}

Function::~Function() {
}

void Function::setLowerBound(unsigned Dim, unsigned Offset) {
  unsigned Dimensions = OutField->getGrid()->getNumDimensions();
  assert(Dim < Dimensions && "Dim is too high");
  Bounds[Dim].first   = Offset;
}

void Function::setUpperBound(unsigned Dim, unsigned Offset) {
  unsigned Dimensions = OutField->getGrid()->getNumDimensions();
  assert(Dim < Dimensions && "Dim is too high");
  Bounds[Dim].second  = Offset;
}

std::set<Field*> Function::getInputFields() const {
  std::set<Field*> Fields;
  Expr->getFields(Fields);
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
adjustRegion(Field *F, Region &FRegion, const Region &InRegion,
             const std::list<Field*> &UpdateOrder, bool LastTS) const {
  FieldRefVisitor V(F, FRegion, InRegion, UpdateOrder, LastTS, this);
  V.visitExpr(Expr);
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
  V.visitExpr(Expr);  
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
  FlopsVisitor V;
  V.visitExpr(Expr);
  return V.getFlops();
}




}
