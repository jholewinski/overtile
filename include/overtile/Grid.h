
#include <list>

namespace overtile {

class Field;
class Function;

/**
 * Abstraction of a space in Z^n
 */
class Grid {
public:
  Grid(unsigned Dim);
  ~Grid();

  /// attachField - Attaches a Field object to the grid.
  void attachField(Field *F);

  /// appendFunction - Appends the Function \p F to the list of stencil point
  /// functions that act on this grid.
  void appendFunction(Function *F);
  
  //==-- Accessors --========================================================= //
  unsigned getNumDimensions() const { return Dimensions; }
  
private:

  typedef std::list<Field*>    FieldList;
  typedef std::list<Function*> FunctionList;
  
  /// Number of dimensions in the space
  unsigned     Dimensions;
  /// Fields attached to this grid.
  FieldList    Fields;
  /// Functions assigned to this grid.
  FunctionList Functions;
};

}
