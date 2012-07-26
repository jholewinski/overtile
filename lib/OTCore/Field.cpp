
#include "overtile/Core/Field.h"
#include "overtile/Core/Grid.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

Field::Field(Grid *G, ElementType *Ty, const std::string &N)
  : TheGrid(G), ElemTy(Ty), Name(N) {
  assert(G != NULL && "G cannot be NULL");
  assert(Ty != NULL && "Ty cannot be NULL");
  G->attachField(this);
}

Field::~Field() {
}

}
