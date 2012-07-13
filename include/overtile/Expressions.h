
#include <vector>
#include <set>
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


/**
 * A field reference expression.
 */
class FieldRef : public Expression {
public:
  FieldRef(Field *F, const std::vector<int>& Off);
  FieldRef(Field *F, int NumOffsets, const int Off[]);
  virtual ~FieldRef();

  virtual void getFields(std::set<Field*> &Fields) const;

  Field *getField() { return TheField; }
  const Field *getField() const { return TheField; }

  const std::vector<int> &getOffsets() const { return Offsets; }

public:

  Field            *TheField;
  std::vector<int>  Offsets;
};


}
