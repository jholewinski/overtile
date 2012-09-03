
#ifndef OVERTILE_CORE_FIELD_H
#define OVERTILE_CORE_FIELD_H

#include <string>

namespace overtile {

class ElementType;
class Grid;

/**
 * Abstraction of a field containing data over a Grid.
 */
class Field {
public:
  Field(Grid *G, ElementType *Ty, const std::string &N);
  ~Field();


  //==-- Accessors --========================================================= //

  Grid *getGrid() { return TheGrid; }
  const Grid *getGrid() const { return TheGrid; }

  const ElementType *getElementType() const { return ElemTy; }

  const std::string &getName() const { return Name; }
  
private:

  Grid        *TheGrid;
  ElementType *ElemTy;
  std::string  Name;
};

}

#endif
