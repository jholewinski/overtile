
#include "overtile/Core/BackEnd.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Grid.h"
#include "llvm/Support/raw_ostream.h"
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

  for (unsigned i = 0, e = G->getNumDimensions(); i != e; ++i) {
    
    BlockSize[i] = 8;
    Elements[i]  = 1;
  }
  
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

  if (getVerbose()) {
    for (unsigned i = 0, e = TheGrid->getNumDimensions(); i != e; ++i) {
      llvm::errs() << "Block Size (" << i << "): " << BlockSize[i] << "\n";
    }
    for (unsigned i = 0, e = TheGrid->getNumDimensions(); i != e; ++i) {
      llvm::errs() << "Tile Size (" << i << "): " << Elements[i] << "\n";
    }
  }
  
  generateTiling();
}

void BackEnd::generateTiling() {
  if (Verbose) {
    llvm::errs() << "Initial Regions:\n";
    const std::list<Field*> &Fields = TheGrid->getFieldList();

    for (std::list<Field*>::const_iterator I = Fields.begin(),
           E = Fields.end(); I != E; ++I) {
      const Field *F = Regions.find(*I)->first;
      Region &R = Regions.find(*I)->second;
      llvm::errs() << "Field `" << F->getName() << "': ";
      R.dump(llvm::errs());
      llvm::errs() << "\n";
    }
  }

  const std::list<Function*> &Functions = TheGrid->getFunctionList();
  
  // Generate list of fields in update order
  std::list<Field*> UpdateOrder;
  
  for (std::list<Function*>::const_iterator I = Functions.begin(),
         E = Functions.end(); I != E; ++I) {

    UpdateOrder.push_back((*I)->getOutput());
  }

  if (Verbose) {
    llvm::errs() << "Field update order: <";
    for (std::list<Field*>::iterator I = UpdateOrder.begin(),
           B = I, E = UpdateOrder.end(); I != E; ++I) {

      if (I != B) llvm::errs() << ", ";
      llvm::errs() << (*I)->getName();
    }
    llvm::errs() << ">\n";
  }

  
  // Iterate for T time steps
  for (unsigned i = 0; i < TimeTileSize; ++i) {
    unsigned T = TimeTileSize - i - 1;
    if (Verbose) llvm::errs() << "Iterating time step " << T << "\n";

    // Get list of stencil point functions
    for (std::list<Function*>::const_reverse_iterator I = Functions.rbegin(),
           E = Functions.rend(); I != E; ++I) {
      const Function *F = *I;
      const Field *Out = F->getOutput();
      Region &OutRegion = Regions.find(Out)->second;
      std::set<Field*> Input = F->getInputFields();

      if (Verbose) {
        llvm::errs() << "Looking at output field " << Out->getName() << "\n";
      }
      
      // For each input field, make sure we are producing enough elements
      for (std::set<Field*>::iterator FI = Input.begin(), FE  = Input.end();
             FI                                              != FE; ++FI) {
        Region &FRegion                                       = Regions.find(*FI)->second;
        Region  OriginalOut(OutRegion);
        F->adjustRegion(*FI, FRegion, OriginalOut, UpdateOrder, (i == TimeTileSize-1));
      }
    }
  }

  if (Verbose) {
    llvm::errs() << "Final Regions:\n";
    const std::list<Field*> &Fields = TheGrid->getFieldList();

    for (std::list<Field*>::const_iterator I = Fields.begin(),
           E = Fields.end(); I != E; ++I) {
      const Field *F = Regions.find(*I)->first;
      Region &R = Regions.find(*I)->second;
      llvm::errs() << "Field `" << F->getName() << "': ";
      R.dump(llvm::errs());
      llvm::errs() << "\n";
    }
  }
}

}
