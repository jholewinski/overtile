
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Grid.h"
#include "overtile/Core/CudaBackEnd.h"
#include "overtile/Core/Types.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <fstream>

using namespace llvm;
using namespace overtile;


static cl::opt<unsigned>
TimeTileSize("t", cl::desc("Specify time tile size"),
             cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
BlockSizeX("x", cl::desc("Specify block size (X)"),
           cl::value_desc("N"), cl::init(16));

static cl::opt<unsigned>
BlockSizeY("y", cl::desc("Specify block size (Y)"),
           cl::value_desc("N"), cl::init(4));

static cl::opt<unsigned>
BlockSizeZ("z", cl::desc("Specify block size (Z)"),
           cl::value_desc("N"), cl::init(4));


static cl::opt<unsigned>
ElementsX("ex", cl::desc("Specify elements per thread (X)"),
          cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
ElementsY("ey", cl::desc("Specify elements per thread (Y)"),
          cl::value_desc("N"), cl::init(1));

static cl::opt<unsigned>
ElementsZ("ez", cl::desc("Specify elements per thread (Z)"),
          cl::value_desc("N"), cl::init(1));



int main(int argc, char** argv) {

  cl::ParseCommandLineOptions(argc, argv, "Rician3D example");
    
  ElementType *Ty = new FP32Type();
  Grid        *GD = new Grid(3);

  Field *U = new Field(GD, Ty, "U");
  Field *G = new Field(GD, Ty, "G");
  Field *F = new Field(GD, Ty, "F");
  Field *R = new Field(GD, Ty, "R");

  FP32Constant *ConstOne = new FP32Constant(1.0);
  FP32Constant *Epsilon  = new FP32Constant(1.0e-20);
  FP32Constant *Sigma    = new FP32Constant(1.0000001);
  FP32Constant *Lambda   = new FP32Constant(1.0000001);
  Expression   *Sigma2   = new BinaryOp(BinaryOp::MUL, Sigma, Sigma);
  Expression   *Gamma    = new BinaryOp(BinaryOp::DIV, Lambda, Sigma2);
  FP32Constant *C1       = new FP32Constant(2.38944);
  FP32Constant *C2       = new FP32Constant(0.950037);
  FP32Constant *C3       = new FP32Constant(4.65314);
  FP32Constant *C4       = new FP32Constant(2.57541);
  FP32Constant *C5       = new FP32Constant(1.48937);
  FP32Constant *DT       = new FP32Constant(5.0);


  
  IntConstant *Offsets[3];

  Offsets[0]          = new IntConstant(0);
  Offsets[1]          = new IntConstant(0);
  Offsets[2]          = new IntConstant(0);
  Expression *U_0_0_0 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(1);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(0);
  Expression *U_p1_0_0 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(-1);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(0);
  Expression *U_m1_0_0 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(1);
  Offsets[2]           = new IntConstant(0);
  Expression *U_0_p1_0 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(-1);
  Offsets[2]           = new IntConstant(0);
  Expression *U_0_m1_0 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(1);
  Expression *U_0_0_p1 = new FieldRef(U, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(-1);
  Expression *U_0_0_m1 = new FieldRef(U, 3, Offsets);



  Offsets[0]          = new IntConstant(0);
  Offsets[1]          = new IntConstant(0);
  Offsets[2]          = new IntConstant(0);
  Expression *G_0_0_0 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(1);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(0);
  Expression *G_p1_0_0 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(-1);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(0);
  Expression *G_m1_0_0 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(1);
  Offsets[2]           = new IntConstant(0);
  Expression *G_0_p1_0 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(-1);
  Offsets[2]           = new IntConstant(0);
  Expression *G_0_m1_0 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(1);
  Expression *G_0_0_p1 = new FieldRef(G, 3, Offsets);

  Offsets[0]           = new IntConstant(0);
  Offsets[1]           = new IntConstant(0);
  Offsets[2]           = new IntConstant(-1);
  Expression *G_0_0_m1 = new FieldRef(G, 3, Offsets);


  Offsets[0]          = new IntConstant(0);
  Offsets[1]          = new IntConstant(0);
  Offsets[2]          = new IntConstant(0);
  Expression *R_0_0_0 = new FieldRef(R, 3, Offsets);


  
  Expression *T1 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_0_p1_0);
  T1             = new BinaryOp(BinaryOp::MUL, T1, T1);

  Expression *T2 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_0_m1_0);
  T2             = new BinaryOp(BinaryOp::MUL, T2, T2);

  Expression *T3 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_0_0_p1);
  T3             = new BinaryOp(BinaryOp::MUL, T3, T3);

  Expression *T4 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_0_0_m1);
  T4             = new BinaryOp(BinaryOp::MUL, T4, T4);

  Expression *T5 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_p1_0_0);
  T5             = new BinaryOp(BinaryOp::MUL, T5, T5);

  Expression *T6 = new BinaryOp(BinaryOp::SUB, U_0_0_0, U_m1_0_0);
  T6             = new BinaryOp(BinaryOp::MUL, T6, T6);

  Expression *T7  = new BinaryOp(BinaryOp::ADD, Epsilon, T1);
  Expression *T8  = new BinaryOp(BinaryOp::ADD, T7, T2);
  Expression *T9  = new BinaryOp(BinaryOp::ADD, T8, T3);
  Expression *T10 = new BinaryOp(BinaryOp::ADD, T9, T4);
  Expression *T11 = new BinaryOp(BinaryOp::ADD, T10, T5);
  Expression *T12 = new BinaryOp(BinaryOp::ADD, T11, T6);

  Expression *T13 = new BinaryOp(BinaryOp::DIV, ConstOne, T12);
  
  Function *Func = new Function(G, T13);
  Func->setLowerBound(0, 1);
  Func->setLowerBound(1, 1);
  Func->setLowerBound(2, 1);
  Func->setUpperBound(0, 1);
  Func->setUpperBound(1, 1);
  Func->setUpperBound(2, 1);

  GD->appendFunction(Func);


  Offsets[0]          = new IntConstant(0);
  Offsets[1]          = new IntConstant(0);
  Offsets[2]          = new IntConstant(0);
  Expression *F_0_0_0 = new FieldRef(F, 3, Offsets);


  T1 = new BinaryOp(BinaryOp::MUL, U_0_0_0, F_0_0_0);
  T2 = new BinaryOp(BinaryOp::DIV, T1, Sigma2);
  
  T3 = new BinaryOp(BinaryOp::ADD, C2, T2);
  T4 = new BinaryOp(BinaryOp::MUL, T2, T3);
  T5 = new BinaryOp(BinaryOp::ADD, C1, T4);
  T6 = new BinaryOp(BinaryOp::MUL, T2, T5);

  T7  = new BinaryOp(BinaryOp::ADD, C5, T2);
  T8  = new BinaryOp(BinaryOp::MUL, T2, T7);
  T9  = new BinaryOp(BinaryOp::ADD, C4, T8);
  T10 = new BinaryOp(BinaryOp::MUL, T2, T9);
  T11 = new BinaryOp(BinaryOp::ADD, C3, T10);
  
  T12 = new BinaryOp(BinaryOp::DIV, T6, T11);


  Func = new Function(R, T12);
  Func->setLowerBound(0, 1);
  Func->setLowerBound(1, 1);
  Func->setLowerBound(2, 1);
  Func->setUpperBound(0, 1);
  Func->setUpperBound(1, 1);
  Func->setUpperBound(2, 1);

  GD->appendFunction(Func);



  T1  = new BinaryOp(BinaryOp::MUL, U_0_p1_0, G_0_p1_0);
  T2  = new BinaryOp(BinaryOp::MUL, U_0_m1_0, G_0_m1_0);
  T3  = new BinaryOp(BinaryOp::MUL, U_0_0_p1, G_0_0_p1);
  T4  = new BinaryOp(BinaryOp::MUL, U_0_0_m1, G_0_0_m1);
  T5  = new BinaryOp(BinaryOp::MUL, U_p1_0_0, G_p1_0_0);
  T6  = new BinaryOp(BinaryOp::MUL, U_m1_0_0, G_m1_0_0);
  T7  = new BinaryOp(BinaryOp::MUL, Gamma, F_0_0_0);
  T8  = new BinaryOp(BinaryOp::MUL, T7, R_0_0_0);
  T9  = new BinaryOp(BinaryOp::MUL, DT, T8);
  T10 = new BinaryOp(BinaryOp::ADD, U_0_0_0, T9);

  T1 = new BinaryOp(BinaryOp::MUL, DT, G_0_p1_0);
  T2 = new BinaryOp(BinaryOp::ADD, ConstOne, T1);
  T3 = new BinaryOp(BinaryOp::ADD, T2, G_0_m1_0);
  T4 = new BinaryOp(BinaryOp::ADD, T3, G_0_0_p1);
  T5 = new BinaryOp(BinaryOp::ADD, T4, G_0_0_m1);
  T6 = new BinaryOp(BinaryOp::ADD, T5, G_p1_0_0);
  T7 = new BinaryOp(BinaryOp::ADD, T6, G_m1_0_0);
  T8 = new BinaryOp(BinaryOp::ADD, T7, Gamma);

  T1 = new BinaryOp(BinaryOp::DIV, T10, T8);


  Func = new Function(U, T1);
  Func->setLowerBound(0, 1);
  Func->setLowerBound(1, 1);
  Func->setLowerBound(2, 1);
  Func->setUpperBound(0, 1);
  Func->setUpperBound(1, 1);
  Func->setUpperBound(2, 1);

  GD->appendFunction(Func);

  

  CudaBackEnd BE(GD);
  BE.setVerbose(true);
  BE.setTimeTileSize(TimeTileSize);
  BE.setBlockSize(0, BlockSizeX);
  BE.setBlockSize(1, BlockSizeY);
  BE.setBlockSize(2, BlockSizeZ);
  BE.setElements(0, ElementsX);
  BE.setElements(1, ElementsY);
  BE.setElements(2, ElementsZ);

  BE.run();
  
  BE.codegen(llvm::outs());
    
  return 0;
}
