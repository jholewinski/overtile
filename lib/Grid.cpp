

#include "overtile/Grid.h"
#include <algorithm>
#include <cassert>

namespace overtile {

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

}
