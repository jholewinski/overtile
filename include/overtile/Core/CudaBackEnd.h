
#ifndef OVERTILE_CORE_CUDABACKEND_H
#define OVERTILE_CORE_CUDABACKEND_H

#include "overtile/Core/BackEnd.h"
#include <set>

namespace overtile {

class BinaryOp;
class ConstantExpr;
class ElementType;
class Expression;
class FieldRef;

/**
 * Back-end code generator for Cuda.
 */
class CudaBackEnd : public BackEnd {
public:
  CudaBackEnd(Grid *G);
  virtual ~CudaBackEnd();

  virtual void codegen(llvm::raw_ostream &OS);

private:

  virtual void codegenDevice(llvm::raw_ostream &OS);
  virtual void codegenHost(llvm::raw_ostream &OS);

  bool                  InTS0;
  std::set<std::string> WrittenFields;
  
  static std::string getTypeName(const ElementType *Ty);

  unsigned codegenExpr(Expression *Expr, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenBinaryOp(BinaryOp *Op, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenFieldRef(FieldRef *Ref, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenConstant(ConstantExpr *Expr, llvm::raw_ostream &OS, unsigned &TempIdx);
};

}

#endif
