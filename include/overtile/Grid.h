
#include <list>

namespace overtile {

class Field;

/**
 * Abstraction of a space in Z^n
 */
class Grid {
public:
  Grid(unsigned Dim);
  ~Grid();

  /// attachField - Attaches a Field object to the grid.
  void attachField(Field *F);

  //==-- Accessors --========================================================= //
  unsigned getNumDimensions() const { return Dimensions; }
  
private:

  typedef std::list<Field*> FieldList;

  
  /// Number of dimensions in the space
  unsigned  Dimensions;
  /// Fields attached to this grid.
  FieldList Fields;
};

}
