
#include <vector>
#include <set>
#include <string>

namespace overtile {

class Field;

/**
 * Base class for all expression types.
 */
class Expression {
public:
  Expression();
  virtual ~Expression();

  virtual void getFields(std::set<Field*> &Fields) const = 0;
};


/**
 * A binary operator.
 */
class BinaryOp : public Expression {
public:
  /**
   * Type of binary operator for BinaryOp expressions.
   */
  enum Operator {
    ADD,
    SUB,
    MUL,
    DIV
  };

  BinaryOp(Operator O, Expression *L, Expression *R);
  virtual ~BinaryOp();

  virtual void getFields(std::set<Field*> &Fields) const;

  //==-- Accessors --========================================================= //
  Operator getOperator() const { return Op; }

  Expression *getLHS() { return LHS; }
  const Expression *getLHS() const { return LHS; }

  Expression *getRHS() { return RHS; }
  const Expression *getRHS() const { return RHS; }
  
private:
  Operator    Op;
  Expression *LHS;
  Expression *RHS;
};

class IntConstant;

/**
 * A field reference expression.
 */
class FieldRef : public Expression {
public:
  FieldRef(Field *F, const std::vector<IntConstant*>& Off);
  FieldRef(Field *F, int NumOffsets, IntConstant* Off[]);
  virtual ~FieldRef();

  virtual void getFields(std::set<Field*> &Fields) const;

  Field *getField() { return TheField; }
  const Field *getField() const { return TheField; }

  const std::vector<IntConstant*> &getOffsets() const { return Offsets; }

public:

  Field                     *TheField;
  std::vector<IntConstant*>  Offsets;
};


/**
 * A constant value.
 */
class ConstantExpr : public Expression {
public:
  ConstantExpr();
  virtual ~ConstantExpr();

  virtual void getFields(std::set<Field*> &Fields) const {}
  
  virtual std::string getStringValue() const = 0;
};


class FP32Constant : public ConstantExpr {
public:
  FP32Constant(float V);
  virtual ~FP32Constant();

  virtual std::string getStringValue() const {
    return StringValue;
  }

  float getValue() const { return Value; }

private:
  
  float       Value;
  std::string StringValue;
  
};


class IntConstant : public ConstantExpr {
public:
  IntConstant(int V);
  virtual ~IntConstant();

  virtual std::string getStringValue() const {
    return StringValue;
  }

  int getValue() const { return Value; }

private:
  
  int         Value;
  std::string StringValue;
  
};
  

}
