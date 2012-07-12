

namespace overtile {

class ElementType;
class Grid;

/**
 * Abstraction of a field containing data over a Grid.
 */
class Field {
public:
  Field(Grid *G, ElementType *Ty);
  ~Field();


  //==-- Accessors --========================================================= //

  Grid *getGrid() { return TheGrid; }
  const Grid *getGrid() const { return TheGrid; }

  const ElementType *getElementType() const { return ElemTy; }

private:

  Grid        *TheGrid;
  ElementType *ElemTy;
};

}
