
#ifndef OVERTILE_CORE_CUDABACKEND_H
#define OVERTILE_CORE_CUDABACKEND_H

#include "overtile/Core/BackEnd.h"
#include <set>
#include <vector>

namespace overtile {

class BinaryOp;
struct BoundExpr;
class ConstantExpr;
class ElementType;
class Expression;
class FieldRef;
class FunctionCall;

/**
 * Back-end code generator for Cuda.
 */
class CudaBackEnd : public BackEnd {
public:
  CudaBackEnd(Grid *G);
  virtual ~CudaBackEnd();

  virtual void codegen(llvm::raw_ostream &OS);

  virtual std::string getCanonicalPrototype();
  virtual std::string getCanonicalInvocation(llvm::StringRef TimeStepExpr,
                                             llvm::StringRef ConvTolExpr);

private:

  virtual void codegenDevice(llvm::raw_ostream &OS);
  virtual void codegenHost(llvm::raw_ostream &OS);

  bool                  InTS0;
  std::set<std::string> WrittenFields;
  std::vector<unsigned> SharedMaxLeft;
  
  static std::string getTypeName(const ElementType *Ty);

  void codegenExpr(Expression *Expr, llvm::raw_ostream &OS);
  void codegenBinaryOp(BinaryOp *Op, llvm::raw_ostream &OS);
  void codegenFieldRef(FieldRef *Ref, llvm::raw_ostream &OS);
  void codegenFunctionCall(FunctionCall *FC, llvm::raw_ostream &OS);
  void codegenConstant(ConstantExpr *Expr, llvm::raw_ostream &OS);

  void codegenLoads(Expression *Expr, llvm::raw_ostream &OS, std::set<std::string> &Idents);
  void codegenFieldRefLoad(FieldRef *Ref, llvm::raw_ostream &OS, std::set<std::string> &Idents);

  std::string getBoundExpr(BoundExpr &Expr, unsigned Dim);
};

}

#endif
