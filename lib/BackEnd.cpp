
#include "overtile/BackEnd.h"
#include <cassert>
#include <cstdlib>

namespace overtile {

BackEnd::BackEnd(Grid *G)
  : TheGrid(G) {
  assert(G != NULL && "G cannot be NULL");
}

BackEnd::~BackEnd() {
}

}
