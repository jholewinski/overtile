
#include "overtile/Core/BackEnd.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Grid.h"
#include <set>
#include <cassert>
#include <cstdlib>
#include <iostream>

namespace overtile {

BackEnd::BackEnd(Grid *G)
  : TheGrid(G), TimeTileSize(1) {
  assert(G != NULL && "G cannot be NULL");

  BlockSize = new unsigned[G->getNumDimensions()];
  Elements  = new unsigned[G->getNumDimensions()];
  
  const std::list<Field*> &Fields     = G->getFieldList();
  unsigned                 Dimensions = G->getNumDimensions();

  // Create initial region for each field.
  for (std::list<Field*>::const_iterator I = Fields.begin(),
         E = Fields.end(); I != E; ++I) {
    Regions.insert(std::make_pair<Field*, Region>(*I, Region(Dimensions)));
  }
}

BackEnd::~BackEnd() {
  delete BlockSize;
  delete Elements;
}

void BackEnd::run() {
  generateTiling();
}

void BackEnd::generateTiling() {
  if (Verbose) {
    std::cerr << "Initial Regions:\n";
    const std::list<Field*> &Fields = TheGrid->getFieldList();

    for (std::list<Field*>::const_iterator I = Fields.begin(),
           E = Fields.end(); I != E; ++I) {
      const Field *F = Regions.find(*I)->first;
      Region &R = Regions.find(*I)->second;
      std::cerr << "Field `" << F->getName() << "': ";
      R.dump(std::cerr);
      std::cerr << "\n";
    }
  }

  // Iterate for T-1 time steps
  for (unsigned i = 0; i < TimeTileSize-1; ++i) {
    unsigned T = TimeTileSize - i - 1;
    if (Verbose) std::cerr << "Iterating time step " << T << "\n";

    // Get list of stencil point functions
    const std::list<Function*> &Functions = TheGrid->getFunctionList();
    for (std::list<Function*>::const_reverse_iterator I = Functions.rbegin(),
           E = Functions.rend(); I != E; ++I) {
      const Function *F = *I;
      const Field *Out = F->getOutput();
      Region &OutRegion = Regions.find(Out)->second;
      std::set<Field*> Input = F->getInputFields();

      // For each input field, make sure we are producing enough elements
      for (std::set<Field*>::iterator FI = Input.begin(), FE = Input.end();
             FI != FE; ++FI) {
        Region &FRegion = Regions.find(*FI)->second;
        F->adjustRegion(*FI, FRegion, OutRegion);
      }
    }
  }

  if (Verbose) {
    std::cerr << "Final Regions:\n";
    const std::list<Field*> &Fields = TheGrid->getFieldList();

    for (std::list<Field*>::const_iterator I = Fields.begin(),
           E = Fields.end(); I != E; ++I) {
      const Field *F = Regions.find(*I)->first;
      Region &R = Regions.find(*I)->second;
      std::cerr << "Field `" << F->getName() << "': ";
      R.dump(std::cerr);
      std::cerr << "\n";
    }
  }
}

}
