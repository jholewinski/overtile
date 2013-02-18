/*
 * Region.cpp: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, University of California Los Angeles
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: Region.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/Core/Region.h"
#include "llvm/Support/raw_ostream.h"

namespace overtile {

Region::Region(unsigned Dim) {
  // Default to a unit region
  for (unsigned i = 0; i < Dim; ++i) {
    Bounds.push_back(std::make_pair<unsigned, int>(0, 1));
  }
}

Region::~Region() {
}

void Region::dump(llvm::raw_ostream &OS) const {
  OS << "<";
  for (unsigned i = 0, e = Bounds.size(); i < e; ++i) {
    const BoundsPair &P = Bounds[i];
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
