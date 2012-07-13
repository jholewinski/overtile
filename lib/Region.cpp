
#include "overtile/Region.h"


namespace overtile {

Region::Region(unsigned Dim) {
  // Default to a unit region
  for (unsigned i = 0; i < Dim; ++i) {
    Bounds.push_back(std::make_pair<unsigned, int>(0, 1));
  }
}

Region::~Region() {
}

void Region::dump(std::ostream &OS) {
  OS << "<";
  for (unsigned i = 0, e = Bounds.size(); i < e; ++i) {
    BoundsPair &P = Bounds[i];
    OS << "[" << P.first << ", " << P.second << "]";
    if (i < e-1) OS << ", ";
  }
  OS << ">";
}

}
