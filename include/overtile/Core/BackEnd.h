
#include "overtile/Core/Region.h"
#include "llvm/Support/raw_ostream.h"
#include <list>
#include <map>
#include <ostream>

namespace overtile {

class CGExpression;
class Field;
class Grid;

/**
 * Base class for backend code generators.
 */
class BackEnd {
public:
  BackEnd(Grid *G);
  virtual ~BackEnd();

  /// run - Performs initial target-independent code generation.
  void run();
  
  /// codegenDevice - Generate the final code for the device.
  virtual void codegenDevice(llvm::raw_ostream &OS) = 0;

  /// codegenHost - Generate the final code for the host.
  virtual void codegenHost(llvm::raw_ostream &OS) = 0;

  //==-- Accessors --=========================================================//
  
  unsigned getTimeTileSize() const { return TimeTileSize; }
  void setTimeTileSize(unsigned T) { TimeTileSize = T; }

  Grid *getGrid() { return TheGrid; }
  const Grid *getGrid() const { return TheGrid; }
  
  const std::list<CGExpression*>&
  getCGExpressionList() const { return CGExprs; }

  bool getVerbose() const { return Verbose; }
  void setVerbose(bool V) { Verbose = V; }

  const std::map<const Field*, Region> &getRegionMap() const { return Regions; }
  
private:

  void generateTiling();

  typedef std::list<CGExpression*> CGExpressionList;
  typedef std::map<const Field*, Region> RegionMap;
  
  Grid             *TheGrid;
  unsigned          TimeTileSize;
  CGExpressionList  CGExprs;
  RegionMap         Regions;
  bool              Verbose;
};

}
