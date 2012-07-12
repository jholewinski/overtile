
#include "overtile/Function.h"
#include "overtile/Field.h"
#include "overtile/Grid.h"
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

}
