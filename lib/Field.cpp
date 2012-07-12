
#include "overtile/Field.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

Field::Field(Grid *G, ElementType *Ty)
  : TheGrid(G), ElemTy(Ty) {
  assert(G != NULL && "G cannot be NULL");
  assert(Ty != NULL && "Ty cannot be NULL");
}

Field::~Field() {
}

}
