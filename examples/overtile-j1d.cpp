
#include "overtile/Expressions.h"
#include "overtile/Field.h"
#include "overtile/Function.h"
#include "overtile/Grid.h"
#include "overtile/OpenCLBackEnd.h"
#include "overtile/Types.h"
#include <iostream>

using namespace overtile;

int main(int argc, char** argv) {

  ElementType *Ty = new FP32Type();
  Grid        *G  = new Grid(1);
  Field       *F  = new Field(G, Ty);

  int Offsets[]    = { -1 };
  Expression *FM1  = new FieldRef(F, 1, Offsets);
  Offsets[0]       = 0;
  Expression *FCtr = new FieldRef(F, 1, Offsets);
  Offsets[0]       = 1;
  Expression *FP1  = new FieldRef(F, 1, Offsets);

  Expression *T1 = new BinaryOp(BinaryOp::ADD, FM1, FCtr);
  Expression *T2 = new BinaryOp(BinaryOp::ADD, T1, FP1);

  Function *Func = new Function(F, T2);
  Func->setLowerBound(0, 1);
  Func->setUpperBound(0, 1);

  G->appendFunction(Func);


  OpenCLBackEnd OCL(G);
  std::cout << "OpenCL Device:\n";
  OCL.codegenDevice(std::cout);
  std::cout << "\n\nOpenCL Host:\n";
  OCL.codegenHost(std::cout);
  
  return 0;
}
