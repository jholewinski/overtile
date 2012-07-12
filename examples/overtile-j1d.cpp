
#include "overtile/Field.h"
#include "overtile/Grid.h"
#include "overtile/Types.h"

using namespace overtile;

int main(int argc, char** argv) {

  ElementType *Ty = new FP32Type();
  Grid        *G  = new Grid(1);
  Field       *F  = new Field(G, Ty);

  return 0;
}
