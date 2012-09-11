
#ifndef OVERTILE_CORE_BACKEND_H
#define OVERTILE_CORE_BACKEND_H

#include "overtile/Core/Grid.h"
#include "overtile/Core/Region.h"
#include "llvm/Support/raw_ostream.h"
#include <list>
#include <map>
#include <ostream>

namespace overtile {

class CGExpression;
class Field;

/**
 * Base class for backend code generators.
 */
class BackEnd {
public:
  BackEnd(Grid *G);
  virtual ~BackEnd();

  /// run - Performs initial target-independent code generation.
  void run();

  /// codegen - Generate code and write to stream \p OS.
  virtual void codegen(llvm::raw_ostream &OS) = 0;

  virtual std::string getCanonicalPrototype() { return ""; }
  virtual std::string getCanonicalInvocation() { return ""; }
  
  //==-- Accessors --========================================================= //
  
  unsigned getTimeTileSize() const { return TimeTileSize; }
  void setTimeTileSize(unsigned T) { TimeTileSize = T; }

  unsigned getBlockSize(unsigned Dim) const {
    if (Dim < TheGrid->getNumDimensions()) {
      return BlockSize[Dim];
    } else {
      return 1;
    }
  }

  void setBlockSize(unsigned Dim, unsigned X) {
    if (Dim < TheGrid->getNumDimensions()) {
      BlockSize[Dim] = X;
    }
  }

  unsigned getElements(unsigned Dim) const {
    if (Dim < TheGrid->getNumDimensions()) {
      return Elements[Dim];
    } else {
      return 0;
    }
  }

  void setElements(unsigned Dim, unsigned X) {
    if (Dim < TheGrid->getNumDimensions()) {
      Elements[Dim] = X;
    }
  }


  
  Grid *getGrid() { return TheGrid; }
  const Grid *getGrid() const { return TheGrid; }
  
  const std::list<CGExpression*>&
  getCGExpressionList() const { return CGExprs; }

  bool getVerbose() const { return Verbose; }
  void setVerbose(bool V) { Verbose = V; }

  const std::map<const Field*, Region> &getRegionMap() const { return Regions; }
  
private:

  void generateTiling();

  typedef std::list<CGExpression*>       CGExpressionList;
  typedef std::map<const Field*, Region> RegionMap;
  
  Grid             *TheGrid;
  unsigned          TimeTileSize;
  unsigned         *BlockSize;
  unsigned         *Elements;
  CGExpressionList  CGExprs;
  RegionMap         Regions;
  bool              Verbose;
};

}

#endif
