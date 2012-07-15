
#include "overtile/Function.h"
#include "overtile/Expressions.h"
#include "overtile/Field.h"
#include "overtile/Grid.h"
#include "overtile/Region.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

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
  FieldRefVisitor(Field *F, Region &FReg, const Region &InReg)
    : TheField(F), FRegion (FReg), InRegion(InReg) {}

  void visitExpr(Expression *Expr) {
    if (BinaryOp *Op = dynamic_cast<BinaryOp*>(Expr)) {
      visitBinaryOp(Op);
    } else if (FieldRef *Ref = dynamic_cast<FieldRef*>(Expr)) {
      visitFieldRef(Ref);
    } else {
      assert(0 && "Unhandled expression type");
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

    unsigned                NumDims = InRegion.getNumDimensions();
    Region                  NewR(NumDims);
    const std::vector<int> &Offsets = Ref->getOffsets();

    for (unsigned i = 0; i < NumDims; ++i) {
      std::pair<int, unsigned> InBound = InRegion.getBound(i);
      std::pair<int, unsigned> FBound  = FRegion.getBound(i);
      int                      Off     = Offsets[i];
      
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
  Field        *TheField;
  Region       &FRegion;
  const Region &InRegion;
};

void Function::
adjustRegion(Field *F, Region &FRegion, const Region &InRegion) const {
  FieldRefVisitor V(F, FRegion, InRegion);
  V.visitExpr(Expr);
}

}
