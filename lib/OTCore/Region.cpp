
#include "overtile/Core/Region.h"


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

Region Region::makeUnion(const Region &A, const Region &B) {
  Region Ret(A.getNumDimensions());
  
  for (unsigned i = 0, e = A.getNumDimensions(); i < e; ++i) {
    std::pair<int, unsigned> NewBounds;
    std::pair<int, unsigned> ABound, BBound;

    ABound = A.getBound(i);
    BBound = B.getBound(i);
    
    NewBounds.first  = ABound.first;
    NewBounds.second = ABound.second;
    
    if (BBound.first < NewBounds.first) {
      int Diff          = NewBounds.first - BBound.first;
      NewBounds.first   = BBound.first;
      NewBounds.second += Diff;
    }

    if (BBound.first + BBound.second > NewBounds.first + NewBounds.second) {
      int Diff       = BBound.first + BBound.second -
        (NewBounds.first + NewBounds.second);
      NewBounds.second += Diff;
    }

    Ret.reset(i, NewBounds);
  }

  return Ret;
}

}
