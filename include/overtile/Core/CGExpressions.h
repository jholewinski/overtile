
#ifndef OVERTILE_CORE_CGEXPRESSIONS_H
#define OVERTILE_CORE_CGEXPRESSIONS_H

#include <vector>

namespace overtile {

class Field;

/**
 * Base class for all expression types.
 */
class CGExpression {
public:
  CGExpression();
  virtual ~CGExpression();
};


/**
 * A binary operator.
 */
class CGBinaryOp : public CGExpression {
public:
  /**
   * Type of binary operator for CGBinaryOp expressions.
   */
  enum Operator {
    ADD,
    SUB,
    MUL,
    DIV
  };

  CGBinaryOp(Operator O, CGExpression *L, CGExpression *R);
  virtual ~CGBinaryOp();

  //==-- Accessors --========================================================= //
  Operator getOperator() const { return Op; }

  CGExpression *getLHS() { return LHS; }
  const CGExpression *getLHS() const { return LHS; }

  CGExpression *getRHS() { return RHS; }
  const CGExpression *getRHS() const { return RHS; }
  
private:
  Operator      Op;
  CGExpression *LHS;
  CGExpression *RHS;
};


/**
 * A field reference expression.
 */
class CGFieldRef : public CGExpression {
public:
  CGFieldRef(Field *F, const std::vector<CGExpression*>& Off);
  CGFieldRef(Field *F, int NumOffsets, CGExpression* Off[]);
  virtual ~CGFieldRef();

public:

  Field                      *TheField;
  std::vector<CGExpression*>  Offsets;
};


}

#endif
