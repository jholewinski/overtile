
#ifndef OVERTILE_CORE_REGION_H
#define OVERTILE_CORE_REGION_H

#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <cassert>

namespace overtile {

/**
 * Representation of a rectangular region in Z^n space.
 */
class Region {
public:
  Region(unsigned D);
  ~Region();

  void dump(llvm::raw_ostream &OS);

  unsigned getNumDimensions() const { return Bounds.size(); }

  std::pair<int, unsigned> getBound(unsigned Dim) const {
    assert(Dim < Bounds.size() && "Dim is out of bounds");
    return Bounds[Dim];
  }

  void reset(unsigned Dim, std::pair<int, unsigned> &Bound) {
    assert(Dim < Bounds.size() && "Dim is out of bounds");
    Bounds[Dim] = Bound;
  }

  static Region makeUnion(const Region &A, const Region &B);
  
private:

  typedef std::pair<int, unsigned> BoundsPair;
  typedef std::vector<BoundsPair>  BoundsList;

  BoundsList Bounds;
};

}

#endif
