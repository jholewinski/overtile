

#include "overtile/Core/BackEnd.h"

namespace overtile {

class BinaryOp;
class ConstantExpr;
class ElementType;
class Expression;
class FieldRef;

/**
 * Back-end code generator for OpenCL.
 */
class OpenCLBackEnd : public BackEnd {
public:
  OpenCLBackEnd(Grid *G);
  virtual ~OpenCLBackEnd();

  virtual void codegenDevice(llvm::raw_ostream &OS);
  virtual void codegenHost(llvm::raw_ostream &OS);

private:

  bool InTS0;
  
  static std::string getTypeName(const ElementType *Ty);

  unsigned codegenExpr(Expression *Expr, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenBinaryOp(BinaryOp *Op, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenFieldRef(FieldRef *Ref, llvm::raw_ostream &OS, unsigned &TempIdx);
  unsigned codegenConstant(ConstantExpr *Expr, llvm::raw_ostream &OS, unsigned &TempIdx);
};

}
