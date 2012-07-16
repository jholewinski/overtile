

#include "overtile/BackEnd.h"

namespace overtile {

class BinaryOp;
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

  virtual void codegenDevice(std::ostream &OS);
  virtual void codegenHost(std::ostream &OS);

private:

  bool InTS0;
  
  static std::string getTypeName(const ElementType *Ty);

  unsigned codegenExpr(Expression *Expr, std::ostream &OS, unsigned &TempIdx);
  unsigned codegenBinaryOp(BinaryOp *Op, std::ostream &OS, unsigned &TempIdx);
  unsigned codegenFieldRef(FieldRef *Ref, std::ostream &OS, unsigned &TempIdx);
};

}
