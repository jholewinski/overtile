
#include "overtile/Core/Grid.h"
#include "overtile/Core/Field.h"
#include <algorithm>
#include <cassert>

namespace overtile {

Grid::Grid() : Dimensions(1) {
}

Grid::Grid(unsigned Dim) : Dimensions(Dim) {
}

Grid::~Grid() {
}

void Grid::attachField(Field *F) {
  assert(F != NULL && "F cannot be NULL");
  assert(std::find(Fields.begin(), Fields.end(), F) == Fields.end() &&
         "F is already attached to the grid");
  Fields.push_back(F);
}

void Grid::appendFunction(Function *F) {
  assert(F != NULL && "F cannot be NULL");
  assert(std::find(Functions.begin(), Functions.end(), F) == Functions.end() &&
         "F is already attached to the grid");
  Functions.push_back(F);
}

Field *Grid::getFieldByName(llvm::StringRef Name) {
  for (FieldList::iterator I = Fields.begin(), E = Fields.end(); I != E; ++I) {
    Field *F = *I;
    if (Name.equals(F->getName().c_str())) {
      return F;
    }
  }

  return NULL;
}

bool Grid::doesParameterExist(llvm::StringRef Name) {
  for (ParamList::iterator I = Params.begin(), E = Params.end(); I != E; ++I) {
    ParamDef &P = *I;
    if (P.first.compare(Name.data()) == 0) {
      return true;
    }
  }

  return false;
}

void Grid::addParameter(llvm::StringRef Name, const ElementType *Ty) {
  assert(!doesParameterExist(Name) && "Parameter already exists");
  ParamDef P;
  P.first  = Name;
  P.second = Ty;
  Params.push_back(P);
}

}
