
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
  enum ExprKind {
    BinOp,
    FieldRef,
    IntConst,
    FP32Const
  };
  
  Expression(unsigned ClassType);
  virtual ~Expression();

  virtual void getFields(std::set<Field*> &Fields) const = 0;

  unsigned getClassType() const { return ClassType; }
  static inline bool classof(const Expression*) { return true; }
  
private:

  ExprKind ClassType;
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

  static inline bool classof(const BinaryOp*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::BinOp;
  }
  
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

  static inline bool classof(const FieldRef*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::FieldRef;
  }
  
public:

  Field                     *TheField;
  std::vector<IntConstant*>  Offsets;
};


/**
 * A constant value.
 */
class ConstantExpr : public Expression {
public:
  ConstantExpr(unsigned ExKind);
  virtual ~ConstantExpr();

  virtual void getFields(std::set<Field*> &Fields) const {}
  
  virtual std::string getStringValue() const = 0;

  static inline bool classof(const ConstantExpr*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::IntConst ||
           E->getClassType() == Expression::FP32Const;
  }
};


class FP32Constant : public ConstantExpr {
public:
  FP32Constant(float V);
  virtual ~FP32Constant();

  virtual std::string getStringValue() const {
    return StringValue;
  }

  float getValue() const { return Value; }

  static inline bool classof(const FP32Constant*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::FP32Const;
  }
  
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

  static inline bool classof(const IntConstant*) { return true; }
  static inline bool classof(const Expression* E) {
    return E->getClassType() == Expression::IntConst;
  }
  
private:
  
  int         Value;
  std::string StringValue;
  
};
  

}
