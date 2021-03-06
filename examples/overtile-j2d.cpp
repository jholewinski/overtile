
#include "overtile/Expressions.h"
#include "overtile/Field.h"
#include "overtile/Function.h"
#include "overtile/Grid.h"
#include "overtile/OpenCLBackEnd.h"
#include "overtile/Types.h"
#include <iostream>
#include <fstream>

using namespace overtile;

int main(int argc, char** argv) {

  ElementType *Ty = new FP32Type();
  Grid        *G  = new Grid(2);
  Field       *F  = new Field(G, Ty, "A");

  int Offsets[]     = { -1, 0 };
  Expression *FM1C  = new FieldRef(F, 2, Offsets);
  Offsets[0]        = 0;
  Offsets[1]        = 0;
  Expression *FCtr  = new FieldRef(F, 2, Offsets);
  Offsets[0]        = 1;
  Offsets[1]        = 0;
  Expression *FP1C  = new FieldRef(F, 2, Offsets);
  Offsets[0]        = 0;
  Offsets[1]        = -1;
  Expression *FCM1  = new FieldRef(F, 2, Offsets);
  Offsets[0]        = 0;
  Offsets[1]        = 1;
  Expression *FCP1  = new FieldRef(F, 2, Offsets);


  Expression *T1 = new BinaryOp(BinaryOp::ADD, FM1C, FCtr);
  Expression *T2 = new BinaryOp(BinaryOp::ADD, T1, FP1C);
  Expression *T3 = new BinaryOp(BinaryOp::ADD, T2, FCM1);
  Expression *T4 = new BinaryOp(BinaryOp::ADD, T3, FCP1);
  Expression *T5 = new BinaryOp(BinaryOp::MUL, T4, new FP32Constant(0.2f));
  
  Function *Func = new Function(F, T5);
  Func->setLowerBound(0, 1);
  Func->setLowerBound(1, 1);
  Func->setUpperBound(0, 1);
  Func->setUpperBound(1, 1);

  G->appendFunction(Func);


  OpenCLBackEnd OCL(G);
  OCL.setVerbose(true);
  OCL.setTimeTileSize(4);
  OCL.run();
  
  std::cout << "Generating overtile-j2d-host.cpp\n";

  {
    std::ofstream Str("overtile-j2d-host.cpp");
    OCL.codegenHost(Str);
  }

  std::cout << "Generating overtile-j2d-device.cl\n";

  {
    std::ofstream Str("overtile-j2d-device.cl");
    OCL.codegenDevice(Str);
  }
    
  return 0;
}
